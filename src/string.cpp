#include "qcvm/string.h"

#include "parallel_hashmap/btree.h"

#include <mutex>

extern "C" {

struct QC_StringBuffer{
	const QC_Allocator *allocator;
	mutable std::mutex mut;
	phmap::btree_map<QC_Uint32, size_t> taken;
	std::vector<char> buf;
};

QC_StringBuffer *qcCreateStringBufferA(const QC_Allocator *allocator){
	const auto mem = qcAllocA(allocator, sizeof(QC_StringBuffer), alignof(QC_StringBuffer));
	if(!mem){
		qcLogError("failed to allocate memory for QC_StringBuffer");
		return nullptr;
	}

	auto p = new(mem) QC_StringBuffer;

	p->allocator = allocator;
	p->buf.resize(1024, '\0');
	p->taken.emplace(0, 1);

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

QC_String qcStringBufferEmplace(QC_StringBuffer *buf, const char *str, size_t len){
	QCVM_ASSERT(buf);
	QCVM_ASSERT(str && len);

	const auto reqStr = std::string_view(str, len);

	std::scoped_lock lock(buf->mut);

	const auto reqRes = std::search(
		buf->buf.begin(), buf->buf.end(),
		reqStr.begin(), reqStr.end()
	);

	if(reqRes != buf->buf.end()){
		const QC_Uint32 index = uintptr_t(reqRes.base()) - uintptr_t(buf->buf.begin().base());
		const auto emplaceRes = buf->taken.try_emplace(index, len);
		if(emplaceRes.second || emplaceRes.first->second == len){
			return index;
		}
	}

	const auto lastTaken = buf->taken.rbegin().base();
	const auto lastIdx = lastTaken->first;
	const auto lastLen = lastTaken->second;

	const auto nextIdx = lastIdx + lastLen;
	if(nextIdx >= buf->buf.size()){
		const auto newSize = std::bit_ceil(nextIdx);
		buf->buf.resize(newSize);
	}

	std::memcpy(buf->buf.data() + nextIdx, str, len);
	buf->taken.emplace(nextIdx, len);

	return nextIdx;
}

bool qcStringBufferErase(QC_StringBuffer *buf, QC_String s){
	return buf->taken.erase(s);
}

#define QCVM_MAX_VIEWS 128

bool qcStringView(const QC_StringBuffer *buf, size_t numStrs, const QC_String *ss, QC_StringViewFn viewFn, void *user){
	if(!buf){
		qcLogError("NULL buf argument passed");
		return false;
	}
	else if(!viewFn){
		qcLogError("NULL viewFn argument passed");
		return false;
	}
	else if(numStrs > QCVM_MAX_VIEWS){
		qcLogError("too many strings requested: %zu (max %d)", numStrs, QCVM_MAX_VIEWS);
		return false;
	}

	std::scoped_lock lock(buf->mut);

	const char *strs[QCVM_MAX_VIEWS] = { nullptr };
	size_t lens[QCVM_MAX_VIEWS] = { 0 };

	for(size_t i = 0; i < numStrs; i++){
		const auto s = ss[i];
		const auto res = buf->taken.find(s);
		if(res == buf->taken.end()){
			qcLogWarn("could not find QC_String 0x%ux in buffer", s);
			strs[i] = "\0";
			lens[i] = 0;
		}
		else{
			strs[i] = buf->buf.data() + res->first;
			lens[i] = res->second;
		}
	}

	viewFn(user, strs, lens);
	return true;
}

}
