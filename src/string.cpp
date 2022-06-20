#include <functional>
#define QCVM_IMPLEMENTATION
#include "qcvm/common.h"
#include "qcvm/string.h"

#include "parallel_hashmap/btree.h"

#include "plf_list.h"

#include <mutex>

extern "C" {

#define QCVM_STRING_BUF_IDX_SHIFT (QC_Uint32(24))
#define QCVM_STRING_BUF_IDX_MASK (~QC_Uint32(0) << QCVM_STRING_BUF_IDX_SHIFT)
#define QCVM_STRING_OFFSET_MASK (~QCVM_STRING_BUF_IDX_MASK)

struct QC_StringBuffer{
	const QC_Allocator *allocator;
	QC_Uintptr pageSize;
	mutable std::mutex mut;
	phmap::btree_map<QC_Uint32, size_t> taken;
	plf::list<std::vector<char>> bufs;
};

static inline QC_Uint32 qcvm_findInStrBuf(const std::vector<char> &buf, QC_StrView str){
	const auto reqRes = std::search(
		buf.begin(), buf.end(),
		std::boyer_moore_searcher(str.ptr, str.ptr + str.len)
	);

	if(reqRes != buf.end()){
		const QC_Uint32 index = uintptr_t(reqRes.base()) - uintptr_t(buf.begin().base());
		return index;
	}

	return UINT32_MAX;
}

static inline QC_Uint32 qcvm_tryTakeStr(
	const std::vector<char> &buf,
	phmap::btree_map<QC_Uint32, size_t> &taken,
	QC_Uint8 bufIdx,
	QC_Uint32 offset,
	size_t strLen
){
	if(offset == UINT32_MAX || (offset + strLen) >= buf.size()) return offset;

	const QC_Uint32 newIdx = (QC_Uint32(bufIdx) << QCVM_STRING_BUF_IDX_SHIFT) | offset;

	const auto takenRes = taken.try_emplace(newIdx, strLen);
	if(takenRes.second || takenRes.first->second == strLen){
		return newIdx;
	}

	return UINT32_MAX;
}

static inline QC_Uint32 qcvm_tryCreateStr(
	std::vector<char> &buf,
	phmap::btree_map<QC_Uint32, size_t> &taken,
	QC_Uint8 bufIdx,
	QC_StrView str
){
	if(str.len >= buf.size()-1) return UINT32_MAX;

	phmap::btree_map<QC_Uint32, size_t>::iterator lastTaken = taken.end();

	for(auto it = taken.rbegin(); it != taken.rend(); ++it){
		const auto takenBufIdx = ((it->first) & QCVM_STRING_BUF_IDX_MASK) >> QCVM_STRING_BUF_IDX_SHIFT;
		if(takenBufIdx != bufIdx){
			continue;
		}

		lastTaken = it.base();
		break;
	}

	const QC_Uint32 lastTakenIdx = lastTaken == taken.end()
		? 0
		: lastTaken->first & ~QCVM_STRING_BUF_IDX_MASK;

	const QC_Uintptr lastTakenLen = lastTakenIdx == 0
		? 1
		: lastTaken->second;

	const QC_Uintptr nextIdx = lastTakenIdx + lastTakenLen;
	if((nextIdx + str.len) >= buf.size()){
		return UINT32_MAX;
	}

	const QC_Uint32 newTaken = (QC_Uint32(bufIdx) << QCVM_STRING_BUF_IDX_SHIFT) | nextIdx;

	std::memcpy(buf.data() + nextIdx, str.ptr, str.len);
	taken.emplace(newTaken, str.len);
	return newTaken;
}

static inline QC_Uint32 qcvm_emplaceStrBuf(QC_StringBuffer *strs, QC_StrView str){
	QC_Uint32 newIdx = UINT32_MAX;
	QC_Uint8 bufIdx = 0;

	for(auto &&buf : strs->bufs){
		newIdx = qcvm_tryTakeStr(buf, strs->taken, bufIdx, qcvm_findInStrBuf(buf, str), str.len);
		if(newIdx == UINT32_MAX){
			newIdx = qcvm_tryCreateStr(buf, strs->taken, bufIdx, str);
			if(newIdx != UINT32_MAX){
				break;
			}
		}
		else{
			break;
		}

		++bufIdx;
	}

	if(newIdx != UINT32_MAX){
		return newIdx;
	}

	const auto newBufIdx = strs->bufs.size();
	if(newBufIdx > std::numeric_limits<QC_Uint8>::max()){
		qcLogError("no space left in string buffer");
		return UINT32_MAX;
	}

	auto &newBuf = strs->bufs.emplace_back();
	newBuf.resize(strs->pageSize, '\0');

	const QC_Uint32 nullTaken = QC_Uint32(newBufIdx) << QCVM_STRING_BUF_IDX_SHIFT;
	strs->taken.emplace(nullTaken, 1);

	const QC_Uint32 newTaken = nullTaken | 0x1;
	std::memcpy(newBuf.data() + 1, str.ptr, str.len);
	strs->taken.emplace(newTaken, str.len);

	return newTaken;
}

QC_StringBuffer *qcCreateStringBufferA(const QC_Allocator *allocator){
	const auto mem = qcAllocA(allocator, sizeof(QC_StringBuffer), alignof(QC_StringBuffer));
	if(!mem){
		qcLogError("failed to allocate memory for QC_StringBuffer");
		return nullptr;
	}

	auto p = new(mem) QC_StringBuffer;

	p->allocator = allocator;
	p->pageSize = qcPageSize();

	auto &newBuf = p->bufs.emplace_back();
	newBuf.resize(p->pageSize, '\0');
	p->taken.emplace(0x0, 1);

	return p;
}

bool qcDestroyStringBuffer(QC_StringBuffer *buf){
	if(!buf){
		return false;
	}

	const auto allocator = buf->allocator;

	std::destroy_at(buf);

	if(!qcFreeA(allocator, buf)){
		qcLogError("failed to free memory at 0x%p, WARNING! OBJECT DESTROYED!", buf);
		return false;
	}

	return true;
}

QC_String qcStringBufferEmplace(QC_StringBuffer *strs, QC_StrView str){
	if(!strs){
		qcLogError("NULL strs argument passed to qcStringBufferEmplace");
		return UINT32_MAX;
	}
	else if(!str.ptr || !str.len){
		qcLogError("empty str argument passed to qcStringBufferEmplace");
		return UINT32_MAX;
	}
	else if(str.len >= (strs->pageSize - 1)){
		qcLogError("str argument too large (max length %zu) passed to qcStringBufferEmplace", str.len);
		return UINT32_MAX;
	}

	std::scoped_lock lock(strs->mut);

	return qcvm_emplaceStrBuf(strs, str);
}

bool qcStringBufferErase(QC_StringBuffer *strs, QC_String s, bool clearBytes){
	if(!strs){
		qcLogError("NULL strs argument passed");
		return false;
	}

	const auto bufIdx = (s & QCVM_STRING_BUF_IDX_MASK) >> QCVM_STRING_BUF_IDX_SHIFT;
	const auto offset = s & QCVM_STRING_OFFSET_MASK;

	std::lock_guard lock(strs->mut);

	if(bufIdx >= strs->bufs.size() || offset >= strs->pageSize){
		qcLogError("invalid QC_String passed");
		return false;
	}

	const auto takenRes = strs->taken.find(s);
	if(takenRes == strs->taken.end()){
		return false;
	}

	const auto takenLen = takenRes->second;

	strs->taken.erase(takenRes);

	if(clearBytes){
		// Erase the string so that information isn't leaked
		// this may be removed or made an option for the function and/or buffer
		auto bufCount = bufIdx;
		auto bufIt = strs->bufs.begin();

		while(bufCount){
			++bufIt;
			--bufCount;
		}

		std::memset(bufIt->data() + offset, 0, takenLen);
	}

	return true;
}

#define QCVM_MAX_VIEWS 128

QC_StrView qcString(const QC_StringBuffer *strs, QC_String s){
	if(!strs){
		qcLogError("NULL strs argument passed");
		return QC_EMPTY_STRVIEW;
	}

	std::scoped_lock lock(strs->mut);

	const auto takenRes = strs->taken.find(s);
	if(takenRes == strs->taken.end()){
		qcLogError("QC_String argument 's' is not a taken string");
		return QC_EMPTY_STRVIEW;
	}

	const QC_Uint32 strBufIdx = (s & QCVM_STRING_BUF_IDX_MASK) >> QCVM_STRING_BUF_IDX_SHIFT;
	const QC_Uint32 strOffset = (s & QCVM_STRING_OFFSET_MASK);

	if(strBufIdx >= strs->bufs.size() || strOffset >= strs->pageSize){
		qcLogError("invalid QC_String argument s passed");
		return QC_EMPTY_STRVIEW;
	}

	auto bufCount = strBufIdx;
	auto bufIt = strs->bufs.begin();

	while(bufCount){
		++bufIt;
		--bufCount;
	}

	return QC_StrView{ bufIt->data() + strOffset, takenRes->second };
}

}
