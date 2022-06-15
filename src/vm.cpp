#define QCVM_IMPLEMENTATION

#include "qcvm/vm.h"
#include "qcvm/string.h"
#include "qcvm/hash.hpp"

#include "parallel_hashmap/phmap_fwd_decl.h"
#include "parallel_hashmap/phmap.h"
#include "parallel_hashmap/btree.h"

#include "plf_colony.h"

#include "fmt/format.h"

#include <vector>
#include <charconv>

using namespace qcvm::hash_literals;

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
	QC_Uint32 retType,
	QC_Uint32 nParams, const QC_Uint32 *paramTypes,
	QC_BuiltinFn ptr,
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
	const QC_Allocator *allocator;
	QC_DefaultBuiltins vmBuiltins;
	QC_StringBuffer *strBuf;

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

static bool qcVMSetBuiltin_unsafe(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting);
static void qcVMResetDefaultBuiltins_unsafe(QC_VM *vm);

QC_VM *qcCreateVMA(const QC_Allocator *allocator, QC_Uint32 flags){
	const auto mem = qcAllocA(allocator, sizeof(QC_VM), alignof(QC_VM));
	if(!mem){
		qcLogError("failed to allocate memory for QC_VM");
		return nullptr;
	}

	const auto p = new(mem) QC_VM;

	p->allocator = allocator;
	p->strBuf = qcCreateStringBufferA(allocator);

	p->vmBuiltins = QC_DefaultBuiltins{
		.normalize = [](QC_VM*, QC_Vector v) -> QC_Vector{
			const auto vec = qcVec4(v.x, v.y, v.z, 0.f);
			const auto norm = qcVec4Normalize(vec);
			return QC_Vector{QC_VEC4_X(norm), QC_VEC4_Y(norm), QC_VEC4_Z(norm)};
		},
		.vlen = [](QC_VM*, QC_Vector v) -> QC_Float{
			const auto vec = qcVec4(v.x, v.y, v.z, 0.f);
			return qcVec4Length(vec);
		},
		.ftos = [](QC_VM *vm, QC_Float v) -> QC_String{
			const auto str = std::to_string(v);
			return qcStringBufferEmplace(vm->strBuf, QC_StrView{ str.c_str(), str.size() });
		},
		.vtos = [](QC_VM *vm, QC_Vector v) -> QC_String{
			const auto builtins = qcVMDefaultBuiltins(vm);
			const QC_String strs[] = {
				builtins->ftos(vm, v.x),
				builtins->ftos(vm, v.y),
				builtins->ftos(vm, v.z)
			};

			struct Data{
				QC_VM *vm;
				QC_String ret;
			};

			Data user = {.vm = vm, .ret = 0};

			qcStringView(
				vm->strBuf,
				3, strs,
				[](void *user, const QC_StrView *strs){
					const auto data = reinterpret_cast<Data *>(user);
					const auto ret = fmt::format(
						"{} {} {}",
						std::string_view(strs[0].ptr, strs[0].len),
						std::string_view(strs[1].ptr, strs[1].len),
						std::string_view(strs[2].ptr, strs[2].len)
					);

					data->ret = qcStringBufferEmplace(data->vm->strBuf, QC_StrView{ ret.c_str(), ret.size() });
				},
				&user
			);

			return user.ret;
		},
		.rint = [](QC_VM*, QC_Float v) -> QC_Float{ return std::round(v); },
		.floor = [](QC_VM*, QC_Float v) -> QC_Float{ return std::floor(v); },
		.ceil = [](QC_VM*, QC_Float v) -> QC_Float{ return std::ceil(v); },
		.fabs = [](QC_VM*, QC_Float v) -> QC_Float{ return std::abs(v); },
		.stof = [](QC_VM *vm, QC_String s) -> QC_Float{
			QC_Float ret = 0;

			qcStringView(
				vm->strBuf,
				1, &s,
				[](void *user, const QC_StrView *strs){
					const auto ret = reinterpret_cast<QC_Float *>(user);
					const auto str = strs[0];
					const auto convRes = std::from_chars(str.ptr, str.ptr + str.len, *ret);
					if(convRes.ec == std::errc::invalid_argument){
						*ret = std::numeric_limits<QC_Float>::quiet_NaN();
					}
				},
				&ret
			);

			return ret;
		}
	};

	if(flags & QC_VM_CREATE_DEFAULT_BUILTINS){
		qcVMResetDefaultBuiltins_unsafe(p);
	}

	return p;
}

bool qcDestroyVM(QC_VM *vm){
	if(!vm) return false;

	qcDestroyStringBuffer(vm->strBuf);

	const auto allocator = vm->allocator;

	std::destroy_at(vm);

	if(!qcFreeA(allocator, vm)){
		qcLogError("failed to free memory at 0x%p, WARNING! OBJECT DESTROYED!", vm);
		return false;
	}

	return true;
}

static inline void qcVMResetDefaultBuiltins_unsafe(QC_VM *vm){
	size_t nBuiltins;
	const auto builtinInfos = qcDefaultBuiltinsInfo(&nBuiltins);

	for(size_t i = 0; i < nBuiltins; i++){
		const auto builtin = builtinInfos + i;
		const std::string_view builtinName = builtin->name;
		QC_VM_Fn_Builtin newBuiltin;
		std::memset(&newBuiltin, 0, sizeof(newBuiltin));

		const auto newNative = QCVM_SUPER(&newBuiltin);
		const auto newFn = QCVM_SUPER(newNative);

		switch(qcvm::hash(builtinName)){

#define QCVM_CASE_1(fn, retTy, argT) \
                case #fn##_hash: \
                    newNative->ptr = [](QC_VM *vm, void*, void **args) -> QC_Value{ \
                        const auto arg = reinterpret_cast<const argT*>(args[0]); \
                        return QC_Value{ .retTy = vm->vmBuiltins.fn(vm, *arg) }; \
                    }; \
					break;

			QCVM_CASE_1(normalize,	v32, QC_Vector)
			QCVM_CASE_1(vlen,		f32, QC_Vector)
			QCVM_CASE_1(ftos,		u32, QC_Float)
			QCVM_CASE_1(vtos,		u32, QC_Vector)
			QCVM_CASE_1(rint,		f32, QC_Float)
			QCVM_CASE_1(floor,		f32, QC_Float)
			QCVM_CASE_1(ceil,		f32, QC_Float)
			QCVM_CASE_1(fabs,		f32, QC_Float)
			QCVM_CASE_1(stof,		f32, QC_String)

#undef QCVM_CASE_1

			default: break;
		}

		if(!newNative->ptr){
			continue;
		}

		newFn->type = QC_VM_FN_BUILTIN;
		newFn->nameIdx = qcStringBufferEmplace(vm->strBuf, QC_StrView{ builtinName.data(), builtinName.length() });

		newNative->retType = builtin->retType;
		newNative->nParams = builtin->nParams;
		std::memcpy(newNative->paramTypes, builtin->paramTypes, builtin->nParams * sizeof(QC_Uint32));

		newBuiltin.index = builtin->index;

		qcVMSetBuiltin_unsafe(vm, builtin->index, *newNative, true);
	}
}

bool qcVMResetDefaultBuiltins(QC_VM *vm){
	if(!vm){
		qcLogError("NULL vm argument passed");
		return false;
	}

	qcVMResetDefaultBuiltins_unsafe(vm);
	return true;
}

static inline bool qcVMSetBuiltin_unsafe(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting){
	QCVM_SUPER(&fn)->type = QC_VM_FN_BUILTIN;

	QC_VM_Fn_Builtin newBuiltin = {
		.QCVM_SUPER_MEMBER = fn,
		.index = index
	};

	const auto setIfNamed = [vm, &fn, &newBuiltin]{
		if(QCVM_SUPER(&fn)->nameIdx == 0){
			return;
		}

		struct Data{
			QC_VM *vm;
			QC_VM_Fn_Builtin *fn;
		};

		auto data = Data{ .vm = vm, .fn = &newBuiltin };

		qcStringView(
			vm->strBuf,
			1, &QCVM_SUPER(&fn)->nameIdx,
			[](void *user, const QC_StrView *strs){
				const auto data = reinterpret_cast<Data*>(user);
				data->vm->fns[std::string_view(strs[0].ptr, strs[0].len)] = QC_VM_FnStorage{ .builtin = *data->fn };
			},
			&data
		);
	};

	const auto emplaceRes = vm->builtins.try_emplace(index, newBuiltin);
	if(emplaceRes.second){
		setIfNamed();
		return true;
	}
	else if(overrideExisting){
		emplaceRes.first->second = newBuiltin;
		setIfNamed();
		return true;
	}
	else{
		return false;
	}
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

	return qcVMSetBuiltin_unsafe(vm, index, fn, overrideExisting);
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

const QC_DefaultBuiltins *qcVMDefaultBuiltins(const QC_VM *vm){
#ifndef NDEBUG
	if(!vm){
		qcLogError("NULL vm argument passed");
		return nullptr;
	}
#endif
	return &vm->vmBuiltins;
}

const QC_VM_Fn *qcVMFindFn(const QC_VM *vm, const char *name, size_t nameLen){
	if(!vm){
		qcLogError("NULL vm argument passed");
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
			case 1: argPtrs[i] = &args[i].u64; break;
			case 3: argPtrs[i] = &args[i].v32; break;

			default:{
				qcLogError("internal error: invalid type size for parameter %u", i);
				return false;
			}
		}
	}

	*ret = fn->ptr(vm, fn->user, argPtrs);
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

inline QC_String qcvmByteCodeStringEmplace(QC_StringBuffer *buf, const QC_ByteCode *bc, QC_String index){
	const auto strs = qcByteCodeStrings(bc);
	const auto str = std::string_view(strs + index);
	return qcStringBufferEmplace(buf, QC_StrView{ str.data(), str.size() });
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
				vmFn->base.nameIdx = qcvmByteCodeStringEmplace(vm->strBuf, bc, fn->nameIdx);
			}
		}
		else{
			const auto emplaceRes = vm->fns.try_emplace(fnName);
			if(emplaceRes.second || (loadFlags & QC_VM_LOAD_OVERRIDE_FNS)){
				const auto vmFn = &emplaceRes.first->second;
				vmFn->bytecode = QC_VM_Fn_Bytecode{
					.QCVM_SUPER_MEMBER = QC_VM_Fn{
						.type = QC_VM_FN_BYTECODE,
						.nameIdx = qcvmByteCodeStringEmplace(vm->strBuf, bc, fn->nameIdx)
					},
					.bc = bc,
					.fn = fn
				};
			}
		}
	}

	for(QC_Uint32 i = 0; i < nDefs; i++){
		const auto def = defs + i;
		const bool isGlobal = def->type & (1u << 15u);

		if(!isGlobal){
			continue;
		}

		const auto defType = def->type & ~(1u << 15u);
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
