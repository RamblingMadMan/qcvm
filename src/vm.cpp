#include "qcvm/common.h"
#include "qcvm/vm.h"

#include <cstdlib>
#include <vector>

#include "parallel_hashmap/phmap_fwd_decl.h"
#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/btree.h"

#include "plf_colony.h"

template<
	class Key, class Value,
	class Hash  = phmap::priv::hash_default_hash<Key>,
	class Eq    = phmap::priv::hash_default_eq<Key>,
	class Alloc = phmap::priv::Allocator<phmap::priv::Pair<const Key, Value>>
>
using NodeHashMap = phmap::node_hash_map<Key, Value, Hash, Eq, Alloc>;

template<
	class Key, class Value,
	class Hash  = phmap::priv::hash_default_hash<Key>,
	class Eq    = phmap::priv::hash_default_eq<Key>,
	class Alloc = phmap::priv::Allocator<phmap::priv::Pair<const Key, Value>>
>
using FlatHashMap = phmap::flat_hash_map<Key, Value, Hash, Eq, Alloc>;

template<
    class Key, class Value,
	class Compare = phmap::Less<Key>,
	class Alloc   = phmap::Allocator<phmap::priv::Pair<const Key, Value>>
>
using FlatMap = phmap::btree_map<Key, Value, Compare, Alloc>;

extern "C" {

bool qcMakeNativeFn(
	QC_Type retType,
	QC_Uint32 nParams, const QC_Type *paramTypes,
	QC_Value(*ptr)(void**),
	QC_VM_Fn_Native *ret
){
	if(!ret){
		qcLogError("NULL ret argument passed");
		return false;
	}
	else if(!ptr){
		qcLogError("NULL ptr argument passed");
		return false;
	}
	else if(nParams > 8){
		qcLogError("too many parameters: %u (max 8)", nParams);
		return false;
	}

	for(QC_Uint32 i = 0; i < nParams; i++){
		const auto paramType = paramTypes[i];
		if(paramType > QC_TYPE_COUNT){
			qcLogError("invalid type code 0x%ux for parameter %u", paramType, i);
			return false;
		}
	}

	ret->QCVM_SUPER_MEMBER = QC_VM_Fn{ .type = QC_VM_FN_NATIVE };
	ret->retType = retType;
	ret->nParams = nParams;
	std::memcpy(ret->paramTypes, paramTypes, sizeof(QC_Type) * nParams);
	ret->ptr = ptr;
	return true;
}

union QC_VM_FnStorage{
	QC_VM_Fn base;
	QC_VM_Fn_Bytecode bytecode;
	QC_VM_Fn_Native native;
	QC_VM_Fn_Builtin builtin;
};

struct QC_VM_Entity{
	FlatHashMap<
	    std::string, QC_Value,
		phmap::priv::hash_default_hash<std::string_view>,
		phmap::priv::hash_default_eq<std::string_view>
	> fields;
};

struct QC_VM{
	std::vector<const QC_ByteCode*> loadedBc;

	FlatMap<QC_Uint32, QC_VM_Fn_Builtin> builtins;

	FlatHashMap<
		std::string, QC_VM_Value,
		phmap::priv::hash_default_hash<std::string_view>,
		phmap::priv::hash_default_eq<std::string_view>
	> globals;

	NodeHashMap<
		std::string, QC_VM_FnStorage,
		phmap::priv::hash_default_hash<std::string_view>,
		phmap::priv::hash_default_eq<std::string_view>
	> fns;

	plf::colony<QC_VM_Entity> ents;
};

QC_VM *qcCreateVM(){
	const auto mem = std::aligned_alloc(alignof(QC_VM), sizeof(QC_VM));
	if(!mem){
		qcLogError("failed to allocate memory for QC_VM");
		return nullptr;
	}

	const auto p = new(mem) QC_VM;

	return p;
}

bool qcDestroyVM(QC_VM *vm){
	if(!vm) return false;
	std::destroy_at(vm);
	std::free(vm);
	return true;
}

bool qcVMSetBuiltin(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting){
	if(!vm){
		qcLogError("NULL vm argument passed for builtin %u", index);
		return false;
	}
	else if(!fn.ptr){
		qcLogError("NULL function passed for builtin %u", index);
		return false;
	}
	else if(fn.nParams > 8){
		qcLogError("invalid number of function parameters %u (max 8) for builtin %u", fn.nParams, index);
		return false;
	}
	else if(fn.retType >= QC_TYPE_COUNT){
		qcLogError("invalid return type code 0x%ux for builtin %u", fn.retType, index);
		return false;
	}

	for(QC_Uint32 i = 0; i < fn.nParams; i++){
		const auto paramType = fn.paramTypes[i];
		if(paramType >= QC_TYPE_COUNT){
			qcLogError("invalid parameter type code 0x%ux for builtin %u", paramType, index);
			return false;
		}
	}

	QCVM_SUPER(&fn)->type = QC_VM_FN_BUILTIN;

	QC_VM_Fn_Builtin newBuiltin = {
		.QCVM_SUPER_MEMBER = fn,
		.index = index
	};

	const auto emplaceRes = vm->builtins.try_emplace(index, newBuiltin);
	if(emplaceRes.second){
		return true;
	}
	else if(overrideExisting){
		emplaceRes.first->second = newBuiltin;
		return true;
	}
	else{
		return false;
	}
}

bool qcVMGetBuiltin(const QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native *ret){
	if(!vm){
		qcLogError("NULL vm passed for builtin %u", index);
		return false;
	}
	else if(!ret){
		qcLogError("NULL ret passed for builtin %u", index);
		return false;
	}

	const auto res = vm->builtins.find(index);
	if(res == vm->builtins.end()){
		return false;
	}

	*ret = *QCVM_SUPER(&res->second);
	return true;
}

const QC_VM_Fn *qcVMFindFn(const QC_VM *vm, const char *name, size_t nameLen){
	if(!vm){
		qcLogError("NULL argument passed");
		return nullptr;
	}
	else if(!name || !nameLen){
		qcLogError("invalid name string");
		return nullptr;
	}

	const auto nameStr = std::string_view(name, nameLen);
	const auto res = vm->fns.find(nameStr);
	if(res == vm->fns.end()){
		return nullptr;
	}

	return &res->second.base;
}

bool qcVMExecNative_unsafe(QC_VM *vm, const QC_VM_Fn_Native *fn, QC_Uint32 nargs, QC_Value *args, QC_Value *ret){
	void *argPtrs[8] = { nullptr };

	for(QC_Uint32 i = 0; i < nargs; i++){
		const auto argType = fn->paramTypes[i];
		const auto argSize = qcTypeSize(argType);
		switch(argSize){
			case 4: argPtrs[i] = &args[i].u32; break;
			case 8: argPtrs[i] = &args[i].u64; break;
			case 12: argPtrs[i] = &args[i].v32; break;

			default:{
				qcLogError("internal error: invalid type size for parameter %u", i);
				return false;
			}
		}
	}

	*ret = fn->ptr(argPtrs);
	return true;
}

bool qcVMExec(QC_VM *vm, const QC_VM_Fn *fn, QC_Uint32 nArgs, QC_Value *args, QC_Value *ret){
	if(!vm){
		qcLogError("NULL vm passed to qcVMExec");
		return false;
	}
	else if(!fn){
		qcLogError("NULL fn passed to qcVMExec");
		return false;
	}
	else if(nArgs > 8){
		qcLogError("too many arguments passed: %u (max 8)", nArgs);
		return false;
	}

	if(!ret){
		static QC_Value unused;
		ret = &unused;
	}

	switch(fn->type){
		case QC_VM_FN_BUILTIN:
		case QC_VM_FN_NATIVE:{
			const auto nativeFn = reinterpret_cast<const QC_VM_Fn_Native*>(fn);
			if(nArgs != nativeFn->nParams){
				qcLogError("wrong number of arguments passed: %u (expected %u)", nArgs, nativeFn->nParams);
				return false;
			}

			return qcVMExecNative_unsafe(vm, nativeFn, nArgs, args, ret);
		}

		default:{
			qcLogError("unimplemented QC_VM_FnType 0x%ux", fn->type);
			return false;
		}
	}
}

bool qcVMLoadByteCode(QC_VM *vm, const QC_ByteCode *bc, QC_Uint32 loadFlags){
	if(!vm || !bc){
		qcLogError("NULL argument passed");
		return false;
	}

	const auto strBuf = qcByteCodeStrings(bc);
	const auto strBufLen = qcByteCodeStringsSize(bc);

	const auto fns = qcByteCodeFunctions(bc);
	const auto nFns = qcByteCodeNumFunctions(bc);

	const auto defs = qcByteCodeDefs(bc);
	const auto nDefs = qcByteCodeNumDefs(bc);

	const auto globals = qcByteCodeGlobals(bc);
	const auto nGlobals = qcByteCodeNumGlobals(bc);

	for(QC_Uint32 i = 0; i < nFns; i++){
		const auto fn = fns + i;
		const auto fnName = std::string_view(strBuf + fn->nameIdx);

		if(fn->entryPoint < 0){
			const auto builtinIndex = QC_Uint32(-fn->entryPoint);
			const auto res = vm->builtins.find(builtinIndex);
			if(res == vm->builtins.end()){
				qcLogError("builtin %u not found for function '%s'", builtinIndex, strBuf + fn->nameIdx);
				return false;
			}

			const auto builtinFn = &res->second;
			const auto nativeFn = QCVM_SUPER(builtinFn);

			if(fn->numArgs != nativeFn->nParams){
				qcLogError(
					"wrong number of parameters for builtin (%u) function '%s': %u (should be %u)",
					builtinIndex, strBuf + fn->nameIdx, fn->numArgs, nativeFn->nParams
				);
				return false;
			}

			for(QC_Uint32 j = 0; j < nativeFn->nParams; j++){
				const auto builtinParamType = nativeFn->paramTypes[j];
				const auto builtinParamSize = qcTypeSize((QC_Type)builtinParamType);
				const auto fnParamSize = fn->argSizes[j];
				if(fnParamSize != builtinParamSize){
					qcLogError(
						"wrong argument size %u for argument %u in builtin %u for function '%s'",
						fnParamSize, j, builtinIndex, strBuf + fn->nameIdx
					);

					return UINT32_MAX;
				}
			}

			const auto emplaceRes = vm->fns.try_emplace(fnName);
			if(emplaceRes.second || (loadFlags & QC_VM_LOAD_OVERRIDE_FNS)){
				const auto vmFn = &emplaceRes.first->second;
				vmFn->builtin = *builtinFn;
			}
		}
		else{
			const auto emplaceRes = vm->fns.try_emplace(fnName);
			if(emplaceRes.second || (loadFlags & QC_VM_LOAD_OVERRIDE_FNS)){
				const auto vmFn = &emplaceRes.first->second;
				vmFn->bytecode = QC_VM_Fn_Bytecode{
					.QCVM_SUPER_MEMBER = QC_VM_Fn{ .type = QC_VM_FN_BYTECODE },
					.bc = bc,
					.fn = fn
				};
			}
		}
	}

	for(QC_Uint32 i = 0; i < nDefs; i++){
		const auto def = defs + i;
		const bool isGlobal = def->type & (1u << 15u);
		const auto defType = def->type & ~(1u << 15u);

		if(!isGlobal){
			continue;
		}

		const auto defName = std::string_view(strBuf + def->nameIdx);

		const auto globalVal = globals + def->globalIdx;

		const auto emplaceRes = vm->globals.try_emplace(defName);
		if(emplaceRes.second || (loadFlags & QC_VM_LOAD_OVERRIDE_GLOBALS)){
			const auto vmGlobal = &emplaceRes.first->second;
			*vmGlobal = QC_VM_Value{ .type = defType, .value = *globalVal };
		}
	}

	vm->loadedBc.emplace_back(bc);
	return true;
}

}
