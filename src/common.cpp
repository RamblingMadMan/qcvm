#define QCVM_IMPLEMENTATION

#include "qcvm/common.h"

#include "qcvm/hash.hpp"

#ifdef __linux__
#include <unistd.h>
static inline QC_Uintptr qcvm_getPageSize(){ return QC_Uintptr(sysconf(_SC_PAGESIZE)); }
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static inline QC_Uintptr qcvm_getPageSize(){ return 4096; } // TODO
#else
static inline QC_Uintptr qcvm_getPageSize(){ return 4096; }
#endif

#include <cstring>
#include <mutex>

static void QCVM_PRINTF_LIKE(3, 0) qcvm_defaultLogHandler(void*, QC_LogLevel level, const char *fmtStr, va_list args){
	char msgBuf[1024] = {'\0'};
	vsnprintf(msgBuf, 1023, fmtStr, args);

	std::FILE *outFile = stdout;
	const char *label = "INFO";

	switch(level){
		case QC_LOG_INFO: break;

		case QC_LOG_WARN:{
			outFile = stderr;
			label = "WARN";
			break;
		}

		case QC_LOG_ERROR:{
			outFile = stderr;
			label = "ERROR";
			break;
		}

		case QC_LOG_FATAL:{
			outFile = stderr;
			label = "FATAL";
			break;
		}

		default:{
			label = "UNKNOWN";
			break;
		}
	}

	fprintf(outFile, "[%s] %s\n", label, msgBuf);
}

namespace {
	std::mutex qcvm_logMut;
	QC_LogHandler qcvm_logHandler = qcvm_defaultLogHandler;
	void *qcvm_logUser = nullptr;
}

extern "C" {

int qcStrCmp(QC_StrView a, QC_StrView b){
	if(a.len > b.len){
		const auto d = std::strncmp(a.ptr, b.ptr, b.len);
		return d == 0 ? a.ptr[b.len] : d;
	}
	else if(a.len < b.len){
		const auto d = std::strncmp(a.ptr, b.ptr, a.len);
		return d == 0 ? -int(b.ptr[b.len]) : d;
	}
	else{
		return std::strncmp(a.ptr, b.ptr, a.len);
	}
}

QC_Uint32 qcStrHash32(QC_StrView str){ return qcvm::hash32(str); }
QC_Uint64 qcStrHash64(QC_StrView str){ return qcvm::hash64(str); }

static const QC_Allocator qcvm_defaultAllocator = {
	.alloc = [](void*, size_t size, size_t alignment){ return std::aligned_alloc(alignment, size); },
	.free = [](void*, void *mem){ std::free(mem); return (bool)mem; },
	.user = nullptr
};

const QC_Allocator *const QC_DEFAULT_ALLOC = &qcvm_defaultAllocator;

QC_Uintptr qcPageSize(){ return qcvm_getPageSize(); }

QC_LogHandler qcGetLogHandler(void **userRet){
	std::scoped_lock lock(qcvm_logMut);
	if (userRet) *userRet = qcvm_logUser;
	return qcvm_logHandler;
}

void qcSetLogHandler(QC_LogHandler handler, void *user){
	std::scoped_lock lock(qcvm_logMut);
	qcvm_logHandler = handler;
	qcvm_logUser = user;
}

void qcLogV(QC_LogLevel level, const char *fmtStr, va_list args){
	std::scoped_lock lock(qcvm_logMut);
	qcvm_logHandler(qcvm_logUser, level, fmtStr, args);
}

}
