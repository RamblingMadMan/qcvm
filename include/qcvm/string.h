#ifndef QCVM_STRING_H
#define QCVM_STRING_H 1

#include "common.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QC_StringBuffer QC_StringBuffer;

QCVM_API QC_StringBuffer *qcCreateStringBufferA(const QC_Allocator *allocator);

static inline QC_StringBuffer *qcCreateStringBuffer(){
	return qcCreateStringBufferA(QC_DEFAULT_ALLOC);
}

QCVM_API bool qcDestroyStringBuffer(QC_StringBuffer *buf);

QCVM_API QC_String qcStringBufferEmplace(QC_StringBuffer *buf, QC_StrView str);

QCVM_API bool qcStringBufferErase(QC_StringBuffer *buf, QC_String s);

typedef void(*QC_StringViewFn)(void *user, const QC_StrView *strs);

QCVM_API bool qcStringView(const QC_StringBuffer *buf, size_t numStrs, const QC_String *ss, QC_StringViewFn viewFn, void *user);

#ifdef __cplusplus
}
#endif

#endif // !QCVM_STRING_H
