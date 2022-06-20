#ifndef QCVM_NET_H
#define QCVM_NET_H 1

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

QCVM_PREDEF(QC_Net);
QCVM_PREDEF(QC_Socket);

QCVM_API QC_Net *qcCreateNetA(const QC_Allocator *allocator);

static inline QC_Net *qcCreateNet(){
	return qcCreateNetA(QC_DEFAULT_ALLOC);
}

QCVM_API bool qcDestroyNet(QC_Net *net);

QC_Socket *qcNetOpenSocket(QC_Net *net, QC_StrView addr, QC_Uint16 port);
bool qcNetCloseSocket(QC_Net *net, QC_Socket *sock);

QC_Uintptr qcNetWrite(QC_Socket *sock, QC_Uintptr len, const void *data);
QC_Uintptr qcNetRead(QC_Socket *sock, QC_Uintptr maxLen, void *buf);

#ifdef __cplusplus
}
#endif

#endif // !QCVM_NET_H
