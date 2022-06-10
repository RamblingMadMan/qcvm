#ifndef QCVM_VM_H
#define QCVM_VM_H 1

/**
 * @defgroup VM Virtual Machine
 * @{
 */

#include "bytecode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QC_VM QC_VM;

QC_VM *qcCreateVM();
bool qcDestroyVM(QC_VM *vm);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_VM_H
