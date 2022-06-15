#ifndef QCVM_VM_H
#define QCVM_VM_H 1

/**
 * @defgroup VM Virtual Machine
 * @{
 */

#include "bytecode.h"
#include "builtins.h"

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
	QC_String nameIdx;
};

typedef QC_Value(*QC_BuiltinFn)(QC_VM *vm, void *user, void **args);

typedef struct QC_VM_Fn_Native QC_VM_Fn_Native;
struct QC_VM_Fn_Native{
	QCVM_DERIVED(QC_VM_Fn)
	QC_Uint32 retType;
	QC_Uint32 nParams;
	QC_Uint32 paramTypes[8];
	QC_BuiltinFn ptr;
	void *user;
};

QCVM_API bool qcMakeNativeFn(
	QC_Uint32 retType,
	QC_Uint32 nParams, const QC_Uint32 *paramTypes,
	QC_BuiltinFn ptr,
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

typedef struct QC_VM_Value{
	QC_Uint32 type;
	QC_Value value;
} QC_VM_Value;

typedef enum QC_VM_CreateFlags{
	QC_VM_CREATE_DEFAULT_BUILTINS = 1u,
} QC_VM_CreateFlags;

QCVM_API QC_VM *qcCreateVMA(const QC_Allocator *allocator, QC_Uint32 flags);

static inline QC_VM *qcCreateVM(QC_Uint32 flags){
	return qcCreateVMA(QC_DEFAULT_ALLOC, flags);
}

QCVM_API bool qcDestroyVM(QC_VM *vm);

QCVM_API bool qcVMResetDefaultBuiltins(QC_VM *vm);

QCVM_API bool qcVMSetBuiltin(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting);
QCVM_API bool qcVMGetBuiltin(const QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native *ret);

QCVM_API const QC_DefaultBuiltins *qcVMDefaultBuiltins(const QC_VM *vm);

QCVM_API const QC_VM_Fn *qcVMFindFn(const QC_VM *vm, const char *name, size_t nameLen);

QCVM_API bool qcVMGetGlobal(const QC_VM *vm, const char *name, size_t nameLen, QC_VM_Value *ret);
QCVM_API bool qcVMSetGlobal(QC_VM *vm, const char *name, size_t nameLen, QC_VM_Value value);

typedef enum QC_VM_LoadFlags{
	QC_VM_LOAD_OVERRIDE_FNS = 0x1u,
	QC_VM_LOAD_OVERRIDE_GLOBALS = 0x1u << 1u,
} QC_VM_LoadFlags;

QCVM_API bool qcVMLoadByteCode(QC_VM *vm, const QC_ByteCode *bc, QC_Uint32 loadFlags);

QCVM_API bool qcVMExec(QC_VM *vm, const QC_VM_Fn *fn, QC_Uint32 nArgs, QC_Value *args, QC_Value *ret);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_VM_H
