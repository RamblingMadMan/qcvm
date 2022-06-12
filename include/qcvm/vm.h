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

typedef enum QC_VM_FnType{
	QC_VM_FN_BYTECODE = 0x0,
	QC_VM_FN_NATIVE = 0x1,
	QC_VM_FN_BUILTIN = 0x2,
} QC_VM_FnType;

typedef struct QC_VM QC_VM;

typedef struct QC_VM_Fn QC_VM_Fn;
struct QC_VM_Fn{
	QC_VM_FnType type;
};

typedef struct QC_VM_Fn_Native QC_VM_Fn_Native;
struct QC_VM_Fn_Native{
	QCVM_DERIVED(QC_VM_Fn)
	QC_Type retType;
	QC_Uint32 nParams;
	QC_Type paramTypes[8];
	QC_Value(*ptr)(void **args);
};

bool qcMakeNativeFn(
	QC_Type retType,
	QC_Uint32 nParams, const QC_Type *paramTypes,
	QC_Value(*ptr)(void**),
	QC_VM_Fn_Native *ret
);

typedef struct QC_VM_Fn_Bytecode QC_VM_Fn_Bytecode;
struct QC_VM_Fn_Bytecode{
	QCVM_DERIVED(QC_VM_Fn)
	const QC_ByteCode *bc;
	const QC_Function *fn;
};

typedef struct QC_VM_Fn_Builtin QC_VM_Fn_Builtin;
struct QC_VM_Fn_Builtin{
	QCVM_DERIVED(QC_VM_Fn_Native)
	QC_Uint32 index;
};

typedef struct QC_VM_Var{
	QC_Uint32 type;
	QC_Value value;
} QC_VM_Var;

QC_VM *qcCreateVM();
bool qcDestroyVM(QC_VM *vm);

bool qcVMSetBuiltin(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting);
bool qcVMGetBuiltin(const QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native *ret);

const QC_VM_Fn *qcVMFindFn(const QC_VM *vm, const char *name, size_t nameLen);

QC_VM_Var qcVMGetGlobal(const QC_VM *vm, const char *name, size_t nameLen);

void qcVMSetGlobal(QC_VM *vm, const char *name, size_t nameLen, QC_VM_Var value);

bool qcVMLoadByteCode(QC_VM *vm, const QC_ByteCode *bc, bool overrideFns);

bool qcVMExec(const QC_VM *vm, const QC_VM_Fn *fn, QC_Uint32 nArgs, QC_Value *args, QC_Value *ret);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_VM_H
