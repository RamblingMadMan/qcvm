#include "qcvm/vm.h"

#include <cstdlib>

#define qcFatal(fmtStr, ...) \
	(qcLogFatal((fmtStr) __VA_OPT__(,) __VA_ARGS__), std::exit(EXIT_FAILURE), 0)

QC_Value qcvm_printFloatAndDouble(QC_VM*, void **args){
	const auto valPtr = reinterpret_cast<const QC_Float*>(args[0]);
	qcLogInfo("printFloat: %f", *valPtr);
	return { .f32 = *valPtr * 2.f };
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;

	const auto vm = qcCreateVM();
	if(!vm){
		qcFatal("failed to create VM");
	}

	const QC_Type nativeFnArgTypes[] = {
		QC_TYPE_FLOAT
	};

	QC_VM_Fn_Native nativePrintFloat;
	if(!qcMakeNativeFn(
		QC_TYPE_FLOAT,
		1, nativeFnArgTypes,
		qcvm_printFloatAndDouble,
		&nativePrintFloat
	)){
		qcDestroyVM(vm);
		qcFatal("failed to create native function");
	}

	if(!qcVMSetBuiltin(vm, 0, nativePrintFloat, true)){
		qcDestroyVM(vm);
		qcFatal("failed to set builtin");
	}

	QC_VM_Fn_Native nativeReturned;
	if(!qcVMGetBuiltin(vm, 0, &nativeReturned)){
		qcDestroyVM(vm);
		qcFatal("failed to get previously set builtin");
	}

	QC_Value execArgs[] = {
		QC_Value{ .f32 = 12.34f }
	};

	QC_Value execRet;
	if(!qcVMExec(vm, QCVM_SUPER(&nativeReturned), 1, execArgs, &execRet)){
		qcDestroyVM(vm);
		qcFatal("failed to execute builtin");
	}

	if(execRet.f32 != execArgs[0].f32 * 2.f){
		qcDestroyVM(vm);
		qcFatal("wrong value returned from builtin: %f (expected %f)", execRet.f32, execArgs[0].f32 * 2.f);
	}

	if(!qcDestroyVM(vm)){
		qcFatal("error destroying VM");
	}

	return 0;
}
