#ifndef QCVM_COMMON_H
#define QCVM_COMMON_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

#ifndef NDEBUG
#include <assert.h>
#define QCVM_ASSERT(...) assert(__VA_ARGS__)
#else
#define QCVM_ASSERT(...)
#endif

#define QCVM_PREDEF(structName) typedef struct structName structName
#define QCVM_SUPER_MEMBER _super
#define QCVM_DERIVED(base) base QCVM_SUPER_MEMBER;
#define QCVM_SUPER(ptr) (&(ptr)->QCVM_SUPER_MEMBER)
#define QCVM_SUPER2(ptr) (&(ptr)->QCVM_SUPER_MEMBER.QCVM_SUPER_MEMBER)

#if defined(_WIN32)
	#ifdef __GNUC__
		#ifdef QCVM_IMPLEMENTATION
		#define QCVM_API __attribute__((dllexport))
		#else
		#define QCVM_API __attribute__((dllimport))
		#endif
	#else
		#ifdef QCVM_IMPLEMENTATION
		#define QCVM_API __declspec(dllexport)
		#else
		#define QCVM_API __declspec(dllimport)
		#endif
	#endif
#else
#define QCVM_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t QC_Int8;
typedef uint8_t QC_Uint8;

typedef int16_t QC_Int16;
typedef uint16_t QC_Uint16;

typedef int32_t QC_Int32;
typedef uint32_t QC_Uint32;

typedef int64_t QC_Int64;
typedef uint64_t QC_Uint64;

enum {
	QC_TRUE = 1, QC_FALSE = 0
};

typedef QC_Uint32 QC_String;

#define QC_STRING_MAX UINT32_MAX

typedef QC_Uint32 QC_Enum;
typedef QC_Uint32 QC_Bool;
typedef QC_Uint64 QC_Entity; // TODO: check quakec compatibility with 64-bit types

typedef float QC_Float;
typedef double QC_Double;

#ifndef __STDC_IEC_559__
#error "QCVM expects IEEE 754 floating point numbers"
#endif

static_assert(sizeof(QC_Float) == 4, "QCVM expects 32-bit floats");
static_assert(sizeof(QC_Double) == 8, "QCVM expects 64-bit doubles");

typedef QC_Float QC_Float32;
typedef QC_Double QC_Float64;

typedef struct QC_Vector{
	QC_Float x, y, z;
} QC_Vector;

static_assert(sizeof(QC_Vector) == 12, "misaligned QC_Vector");

typedef struct QC_StrView{
	const char *ptr;
	uintptr_t len;
} QC_StrView;

#define QC_STRVIEW(lit) (QC_StrView{ lit, sizeof(lit)-1 })
#define QC_EMPTY_STRVIEW (QC_StrView{ NULL, 0 })

#ifdef __cplusplus
#define QC_MIN(a, b) ([](auto &&a_, auto &&b_){ return (a_ < b_) ? a_ : b_; }((a), (b)))
#define QC_MAX(a, b) ([](auto &&a_, auto &&b_){ return (a_ > b_) ? a_ : b_; }((a), (b)))
#elif __GNUC__
#define QC_MIN(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); _a < _b ? a_ : b_; })
#define QC_MAX(a, b) ({ __typeof__(a) a_ = (a); __typeof__(b) b_ = (b); _a > _b ? a_ : b_; })
#else
#error "Unsupported compiler"
#endif

QCVM_API int qcStrCmp(QC_StrView a, QC_StrView b);

QCVM_API QC_Uint32 qcStrHash32(QC_StrView str);
QCVM_API QC_Uint64 qcStrHash64(QC_StrView str);

#ifdef QCVM_HASH_64
#define qcStrHash qcStrHash64
#else
#define qcStrHash qcStrHash32
#endif

#ifdef __GNUC__
typedef QC_Float QC_Vec4 __attribute__((vector_size(16)));
#define QC_VEC4_DATA(v) (v)
static inline QC_Vec4 qcVec4(QC_Float x, QC_Float y, QC_Float z, QC_Float w){ return QC_Vec4{ x, y, z, w }; }
static inline QC_Vec4 qcVec4All(QC_Float x){ return QC_Vec4{ x, x, x, x }; }
static inline QC_Vec4 qcVec4Add(QC_Vec4 a, QC_Vec4 b){ return a + b; }
static inline QC_Vec4 qcVec4Sub(QC_Vec4 a, QC_Vec4 b){ return a - b; }
static inline QC_Vec4 qcVec4Mul(QC_Vec4 a, QC_Vec4 b){ return a * b; }
static inline QC_Vec4 qcVec4Div(QC_Vec4 a, QC_Vec4 b){ return a / b; }
#else
typedef union QC_Vec4{
	struct {
		QC_Float x, y, z, w;
	};
	QC_Float xyzw alignas(16)[4];
} QC_Vec4;
#define QC_VEC4_DATA(v) (v.xyzw)
static inline QC_Vec4 qcVec4(QC_Float x, QC_Float y, QC_Float z, QC_Float w){ return QC_Vec4{ .xyzw = { x, y, z, w } }; }
static inline QC_Vec4 qcVec4All(QC_Float x){ return QC_Vec4{ .xyzw = { x, x, x, x } }; }
#define QCVM_VEC4_OP(a, b, op) (QC_Vec4{ .xyzw = { (a.x op b.x), (a.y op b.y), (a.z op b.z), (a.w op b.w) } })
static inline QC_Vec4 qcVec4Add(QC_Vec4 a, QC_Vec4 b){ return QCVM_VEC4_OP(a, b, +); }
static inline QC_Vec4 qcVec4Sub(QC_Vec4 a, QC_Vec4 b){ return QCVM_VEC4_OP(a, b, -); }
static inline QC_Vec4 qcVec4Mul(QC_Vec4 a, QC_Vec4 b){ return QCVM_VEC4_OP(a, b, *); }
static inline QC_Vec4 qcVec4Div(QC_Vec4 a, QC_Vec4 b){ return QCVM_VEC4_OP(a, b, /); }
#undef QCVM_VEC4_OP
#endif

#define QC_VEC4_X(v) (QC_VEC4_DATA(v)[0])
#define QC_VEC4_Y(v) (QC_VEC4_DATA(v)[1])
#define QC_VEC4_Z(v) (QC_VEC4_DATA(v)[2])
#define QC_VEC4_W(v) (QC_VEC4_DATA(v)[3])

static inline QC_Float qcVec4Length(QC_Vec4 v){
	const QC_Vec4 exps = qcVec4Mul(v, v);
	return sqrtf(QC_VEC4_X(exps) + QC_VEC4_Y(exps) + QC_VEC4_Z(exps) + QC_VEC4_W(exps));
}

static inline QC_Vec4 qcVec4Normalize(QC_Vec4 v){
	const QC_Float mag = qcVec4Length(v);
	return qcVec4Div(v, qcVec4All(mag));
}

typedef union QC_Value{
	QC_Uint8 u8;
	QC_Int8 i8;
	QC_Uint16 u16;
	QC_Int16 i16;
	QC_Uint32 u32;
	QC_Int32 i32;
	QC_Uint64 u64;
	QC_Int64 i64;
	QC_Float f32;
	QC_Double f64;
	QC_Vector v32;
	QC_Vec4 v4f32;
} QC_Value;

/**
 * @brief Bytecode data types
 */
enum QC_Type{
	// Vanilla types
	QC_TYPE_VOID = 0x0,
	QC_TYPE_STRING	= 0x1,
	QC_TYPE_FLOAT32	= 0x2,
	QC_TYPE_FLOAT	= QC_TYPE_FLOAT32,
	QC_TYPE_VECTOR	= 0x3,
	QC_TYPE_ENTITY	= 0x4,
	QC_TYPE_FIELD	= 0x5,
	QC_TYPE_FUNC	= 0x6,

	// extended types
	QC_TYPE_INT32	= 0x7,
	QC_TYPE_UINT32	= 0x8,
	QC_TYPE_INT64	= 0x9,
	QC_TYPE_UINT64	= 0xA,
	QC_TYPE_FLOAT64	= 0xB,
	QC_TYPE_DOUBLE	= QC_TYPE_FLOAT64,

	// qc-only types (qc?)
	QC_TYPE_VARIANT		= 0xC,
	QC_TYPE_STRUCT		= 0xD,
	QC_TYPE_UNION		= 0xE,
	QC_TYPE_ACCESSOR	= 0xF,	// some weird type to provide class-like functions over a basic type.
	QC_TYPE_ENUM		= 0x10,
	QC_TYPE_BOOL		= 0x11,

	QC_TYPE_COUNT
};

/**
 * @brief Get the size (in number of array elements) of a type
 * @param type Type code
 * @returns Size of type referred to by \p type
 */
QCVM_API QC_Uint32 qcTypeSize(QC_Uint32 type);

typedef struct QC_Allocator{
	void*(*alloc)(void *user, size_t size, size_t alignment);
	bool(*free)(void *user, void *mem);
	void *user;
} QC_Allocator;

QCVM_API extern const QC_Allocator *const QC_DEFAULT_ALLOC;

static inline void *qcAllocA(const QC_Allocator *allocator, size_t size, size_t alignment){
	QCVM_ASSERT(allocator);
	return allocator->alloc(allocator->user, size, alignment);
}

static inline bool qcFreeA(const QC_Allocator *allocator, void *mem){
	QCVM_ASSERT(allocator);
	return allocator->free(allocator->user, mem);
}

static inline void *qcAlloc(size_t size, size_t alignment){
	return qcAllocA(QC_DEFAULT_ALLOC, size, alignment);
}

static inline bool qcFree(void *mem){
	return qcFreeA(QC_DEFAULT_ALLOC, mem);
}

#ifdef __GNUC__
#define QCVM_PRINTF_LIKE(fmtStrArg, vaArg) __attribute__ ((format (printf, (fmtStrArg), (vaArg))))
#else
#define QCVM_PRINTF_LIKE(...)
#endif

enum QC_LogLevel{
	QC_LOG_INFO,
	QC_LOG_WARN,
	QC_LOG_ERROR,
	QC_LOG_FATAL,

	QC_LOG_LEVEL_COUNT
};

typedef void(*QC_LogHandler)(void *user, QC_LogLevel logLevel, const char *fmtStr, va_list args) QCVM_PRINTF_LIKE(3, 0);

QCVM_API QC_LogHandler qcGetLogHandler(void **userRet);
QCVM_API void qcSetLogHandler(QC_LogHandler handler, void *user);

QCVM_API void qcLogV(QC_LogLevel level, const char *fmtStr, va_list args) QCVM_PRINTF_LIKE(2, 0);

inline void QCVM_PRINTF_LIKE(2, 3) qcLog(QC_LogLevel level, const char *fmtStr, ...){
	va_list va;
	va_start(va, fmtStr);
	qcLogV(level, fmtStr, va);
	va_end(va);
}

#define qcLogInfo(fmtStr, ...) qcLog(QC_LOG_INFO, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogWarn(fmtStr, ...) qcLog(QC_LOG_WARN, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogError(fmtStr, ...) qcLog(QC_LOG_ERROR, fmtStr __VA_OPT__(,) __VA_ARGS__)
#define qcLogFatal(fmtStr, ...) qcLog(QC_LOG_FATAL, fmtStr __VA_OPT__(,) __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // !QCVM_COMMON_H
