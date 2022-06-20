#ifndef QCVM_TYPES_H
#define QCVM_TYPES_H 1

/**
 * @defgroup TypesC Type information
 * @{
 */

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

QCVM_PREDEF(QC_TypeSet);

QCVM_API QC_TypeSet *qcCreateTypeSetA(const QC_Allocator *allocator);

static inline QC_TypeSet *qcCreateTypeSet(){ return qcCreateTypeSetA(QC_DEFAULT_ALLOC); }

QCVM_API bool qcDestroyTypeSet(QC_TypeSet *ts);

/**
 * @defgroup TypesFundamentalC Fundamental types
 * @{
 */

enum QC_IntFormat{
	QC_INT_UNSIGNED,
	QC_INT_TWOS_COMPLIMENT,
	QC_INT_GMP,

	QC_INT_FORMAT_COUNT
};

enum QC_FloatFormat{
	QC_FLOAT_IEEE754,
	QC_FLOAT_MPFR,

	QC_FLOAT_FORMAT_COUNT
};

enum QC_StringEncoding{
	QC_STRING_UTF8,
	QC_STRING_UTF16,
	QC_STRING_UTF32,

	QC_STRING_ENCODING_COUNT
};

enum QC_TypeKind{
	QC_TYPE_META,
	QC_TYPE_VOID,
	QC_TYPE_BOOL,
	QC_TYPE_INT,
	QC_TYPE_FLOAT,
	QC_TYPE_STRING,

	QC_TYPE_PTR,
	QC_TYPE_ARRAY,
	QC_TYPE_VECTOR,
	QC_TYPE_FUNCTION,

	QC_TYPE_KIND_COUNT
};

static inline bool qcIsCompoundTypeKind(QC_Uint32 kind){
	switch(kind){
		case QC_TYPE_PTR:
		case QC_TYPE_ARRAY:
		case QC_TYPE_VECTOR:
		case QC_TYPE_FUNCTION:
			return true;

		default: return false;
	}
}

QCVM_PREDEF(QC_Type);
QCVM_PREDEF(QC_Type_Meta);
QCVM_PREDEF(QC_Type_Void);
QCVM_PREDEF(QC_Type_Bool);
QCVM_PREDEF(QC_Type_Int);
QCVM_PREDEF(QC_Type_Float);
QCVM_PREDEF(QC_Type_String);

QCVM_API const QC_Type_Meta *qcTypeMeta();
QCVM_API const QC_Type_Void *qcTypeVoid();
QCVM_API const QC_Type_Bool *qcTypeBool();
QCVM_API const QC_Type_Int *qcTypeInt(QC_Uint32 format, QC_Uint32 numBits);
QCVM_API const QC_Type_Float *qcTypeFloat(QC_Uint32 format, QC_Uint32 numBits);
QCVM_API const QC_Type_String *qcTypeString(QC_Uint32 format);

static inline bool qcTypeIntIsSigned(const QC_Type_Int *int_);
static inline bool qcTypeFloatIsIEEE754(const QC_Type_Float *float_);

/**
 * @}
 */

QCVM_API const QC_Type *qcTypeCommon(QC_TypeSet *ts, QC_Uint32 numTys, const QC_Type *const *tys);

/**
 * @defgroup TypesCompound Compound types
 * @{
 */

QCVM_PREDEF(QC_Type_Ptr);
QCVM_PREDEF(QC_Type_Array);
QCVM_PREDEF(QC_Type_Vector);
QCVM_PREDEF(QC_Type_Function);

QCVM_API const QC_Type_Ptr *qcTypePtr(QC_TypeSet *ts, const QC_Type *pointed);
QCVM_API const QC_Type_Array *qcTypeArray(QC_TypeSet *ts, const QC_Type *elementType, QC_Uint64 numElements);
QCVM_API const QC_Type_Vector *qcTypeVector(QC_TypeSet *ts, const QC_Type *elementType, QC_Uint64 numElements);

enum QC_FunctionFlags{
	QC_FUNCTION_PURE = 0x1,
	QC_FUNCTION_INLINE = 0x2,
	QC_FUNCTION_STATIC = 0x4,
};

QCVM_API const QC_Type_Function *qcTypeFunction(
	QC_TypeSet *ts,
	const QC_Type *resultTy,
	QC_Uint64 numParams, const QC_Type *const *paramTys,
	QC_Uint32 flags
);

/**
 * @}
 */

struct QC_Type{
	QC_Uint32 kind;
	QC_Uint64 numBits;
};

struct QC_Type_Meta{ QCVM_DERIVED(QC_Type); };
struct QC_Type_Void{ QCVM_DERIVED(QC_Type); };
struct QC_Type_Bool{ QCVM_DERIVED(QC_Type); };

struct QC_Type_Int{
	QCVM_DERIVED(QC_Type);
	QC_Uint32 format;
};

struct QC_Type_Float{
	QCVM_DERIVED(QC_Type);
	QC_Uint32 format;
};

struct QC_Type_String{
	QCVM_DERIVED(QC_Type);
	QC_Uint32 encoding;
};

struct QC_Type_Ptr{
	QCVM_DERIVED(QC_Type);
	const QC_Type *pointed;
};

struct QC_Type_Array{
	QCVM_DERIVED(QC_Type);
	const QC_Type *elementType;
	QC_Uintptr numElements;
};

struct QC_Type_Vector{
	QCVM_DERIVED(QC_Type);
	const QC_Type *elementType;
	QC_Uintptr numElements;
};

struct QC_Type_Function{
	QCVM_DERIVED(QC_Type);
	const QC_Type *resultType;
	QC_Uintptr numParams;
	const QC_Type *const *paramTypes;
	QC_Uintptr flags;
};

bool qcTypeIntIsSigned(const QC_Type_Int *int_){ return int_->format != QC_INT_UNSIGNED; }

bool qcTypeFloatIsIEEE754(const QC_Type_Float *float_){ return float_->format == QC_FLOAT_IEEE754; }

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif // !QCVM_TYPES_H
