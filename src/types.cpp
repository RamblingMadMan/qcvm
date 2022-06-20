#define QCVM_IMPLEMENTATION

#include "qcvm/types.h"
#include "qcvm/hash.hpp"

#include "plf_list.h"

#include "parallel_hashmap/phmap.h"

#include <climits>

static const QC_Type_Meta qcvm_typeMeta = { .QCVM_SUPER_MEMBER = { .numBits = sizeof(void*) * CHAR_BIT } };
static const QC_Type_Void qcvm_typeVoid = { .QCVM_SUPER_MEMBER = { .numBits = 0 } };
static const QC_Type_Bool qcvm_typeBool = { .QCVM_SUPER_MEMBER = { .numBits = 1 } };

static const QC_Type_String qcvm_typeStrs[QC_STRING_ENCODING_COUNT] = {
#define QCVM_STRTYPE(encoding_) { .QCVM_SUPER_MEMBER = { .numBits = sizeof(void*) * CHAR_BIT }, .encoding = (encoding_) }
	QCVM_STRTYPE(QC_STRING_UTF8),
	QCVM_STRTYPE(QC_STRING_UTF16),
	QCVM_STRTYPE(QC_STRING_UTF32),
#undef QCVM_STRTYPE
};

namespace detail{
	template<QC_IntFormat format, std::size_t ... Is>
	inline const QC_Type_Int *static_int_types(std::index_sequence<Is...>) noexcept{
		static QC_Type_Int ts[] = { { .QCVM_SUPER_MEMBER = { .numBits = Is + 1 }, .format = format }... };
		return ts;
	}
}

static const QC_Type_Float qcvm_typeFloat32 = { .QCVM_SUPER_MEMBER = { .numBits = 32 }, .format = QC_FLOAT_IEEE754 };
static const QC_Type_Float qcvm_typeFloat64 = { .QCVM_SUPER_MEMBER = { .numBits = 64 }, .format = QC_FLOAT_IEEE754 };

extern "C" {

struct QC_TypeSet{
	const QC_Allocator *allocator;

	plf::list<std::vector<const QC_Type*>> typeLists;
	phmap::node_hash_map<const QC_Type*, QC_Type_Ptr> ptrs;
	phmap::node_hash_map<std::pair<const QC_Type*, QC_Uint64>, QC_Type_Array> arrays;
	phmap::node_hash_map<std::pair<const QC_Type*, QC_Uint64>, QC_Type_Vector> vectors;
	phmap::node_hash_set<QC_Type_Function> functions;
};

QC_TypeSet *qcCreateTypeSetA(const QC_Allocator *allocator){
	const auto mem = qcAllocA(allocator, sizeof(QC_TypeSet), alignof(QC_TypeSet));
	if(!mem){
		qcLogError("failed to allocate memory for QC_TypeSet");
		return nullptr;
	}

	auto p = new(mem) QC_TypeSet;

	p->allocator = allocator;

	return p;
}

bool qcDestroyTypeSet(QC_TypeSet *ts){
	if(!ts){
		return false;
	}

	const auto allocator = ts->allocator;

	std::destroy_at(ts);

	if(!qcFreeA(allocator, ts)){
		qcLogError("failed to free QC_TypeSet memory at 0x%p, WARNING! OBJECT DESTROYED!", ts);
		return false;
	}

	return true;
}

const QC_Type *qcTypeCommon(QC_TypeSet *ts, QC_Uint32 numTys, const QC_Type *const *tys){
	if(!numTys || !tys) return nullptr;

	const QC_Type *ret = tys[0];

	for(QC_Uint32 i = 0; i < numTys; i++){

	}

	return ret;
}

const QC_Type_Meta *qcTypeMeta(){ return &qcvm_typeMeta; }
const QC_Type_Void *qcTypeVoid(){ return &qcvm_typeVoid; }
const QC_Type_Bool *qcTypeBool(){ return &qcvm_typeBool; }

const QC_Type_Int *qcTypeInt(QC_Uint32 format, QC_Uint32 numBits){
	if(numBits == 0 || numBits > 64){
		qcLogError("invalid number of bits for integer type: %u (must be in range [1,64])", numBits);
		return nullptr;
	}

	switch(format){
#define QCVM_CASE(val) case (val): return &detail::static_int_types<(val)>(std::make_index_sequence<64>())[numBits-1]
		QCVM_CASE(QC_INT_TWOS_COMPLIMENT);
		QCVM_CASE(QC_INT_UNSIGNED);
#undef QCVM_CASE
		default:{
			qcLogError("unrecognized integer type format 0x%ux", format);
			return nullptr;
		}
	}
}

const QC_Type_Float *qcTypeFloat(QC_Uint32 format, QC_Uint32 numBits){
	if(format != QC_FLOAT_IEEE754){
		qcLogError("only IEEE-754 float format is currently implemented");
		return nullptr;
	}

	switch(numBits){
		case 32: return &qcvm_typeFloat32;
		case 64: return &qcvm_typeFloat64;

		default:{
			qcLogError("invalid number of bits for float type: %u (must be in range 32 or 64)", numBits);
			return nullptr;
		}
	}
}

const QC_Type_String *qcTypeString(QC_Uint32 encoding){
	if(encoding >= QC_STRING_ENCODING_COUNT){
		return nullptr;
	}

	return &qcvm_typeStrs[encoding];
}

const QC_Type_Ptr *qcTypePtr(QC_TypeSet *ts, const QC_Type *pointed){
	const auto emplaceRes = ts->ptrs.try_emplace(pointed);
	if(emplaceRes.second){
		const auto ptr = &emplaceRes.first->second;
		ptr->QCVM_SUPER_MEMBER = { .numBits = sizeof(void*) * CHAR_BIT };
		ptr->pointed = pointed;
	}

	return &emplaceRes.first->second;
}

const QC_Type_Array *qcTypeArray(QC_TypeSet *ts, const QC_Type *elementType, QC_Uint64 numElements){
	const auto emplaceRes = ts->arrays.try_emplace(std::make_pair(elementType, numElements));
	if(emplaceRes.second){
		const auto arr = &emplaceRes.first->second;
		arr->QCVM_SUPER_MEMBER = { .numBits = elementType->numBits * numElements };
		arr->elementType = elementType;
		arr->numElements = numElements;
	}

	return &emplaceRes.first->second;
}

const QC_Type_Vector *qcTypeVector(QC_TypeSet *ts, const QC_Type *elementType, QC_Uint64 numElements){
	const auto emplaceRes = ts->vectors.try_emplace(std::make_pair(elementType, numElements));
	if(emplaceRes.second){
		const auto arr = &emplaceRes.first->second;
		arr->QCVM_SUPER_MEMBER = { .numBits = elementType->numBits * numElements };
		arr->elementType = elementType;
		arr->numElements = numElements;
	}

	return &emplaceRes.first->second;
}

const QC_Type_Function *qcTypeFunction(
	QC_TypeSet *ts,
	const QC_Type *resultTy,
	QC_Uint64 numParams, const QC_Type *const *paramTys,
	QC_Uint32 flags
){
	if(!ts){
		qcLogError("NULL ts argument passed");
		return nullptr;
	}
	else if(!resultTy){
		qcLogError("NULL resultTy argument passed");
		return nullptr;
	}
	else if(numParams && !paramTys){
		qcLogError("NULL paramTys argument passed");
		return nullptr;
	}

	qcLogError("function types unimplemented");
	return nullptr;
}

}
