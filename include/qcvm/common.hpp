#ifndef QCVM_COMMON_HPP
#define QCVM_COMMON_HPP 1

#include "common.h"

namespace qcvm{
	using Int8 = QC_Int8;
	using Uint8 = QC_Uint8;
	using Int16 = QC_Int16;
	using Uint16 = QC_Uint16;
	using Int32 = QC_Int32;
	using Uint32 = QC_Uint32;
	using Int64 = QC_Int64;
	using Uint64 = QC_Uint64;
	using Float32 = QC_Float32;
	using Float64 = QC_Float64;
	using Vector = QC_Vector;
	using Vec4 = QC_Vec4;

	namespace detail{
		template<auto Val> struct IdT{ static constexpr auto value = Val; };
		template<typename T> struct ToQC_Type;
		template<> struct ToQC_Type<Int32>:		IdT<QC_TYPE_INT32>{};
		template<> struct ToQC_Type<Uint32>:	IdT<QC_TYPE_UINT32>{};
		template<> struct ToQC_Type<Int64>:		IdT<QC_TYPE_INT64>{};
		template<> struct ToQC_Type<Uint64>:	IdT<QC_TYPE_UINT64>{};
		template<> struct ToQC_Type<Float32>:	IdT<QC_TYPE_FLOAT32>{};
		template<> struct ToQC_Type<Float64>:	IdT<QC_TYPE_FLOAT64>{};
		template<> struct ToQC_Type<Vector>:	IdT<QC_TYPE_VECTOR>{};
	}

	template<typename T>
	constexpr Uint32 toQC_Type(){ return detail::ToQC_Type<T>::value; }

	enum class LogLevel: Uint32{
		info = QC_LOG_INFO,
		warn = QC_LOG_WARN,
		error = QC_LOG_ERROR,
		fatal = QC_LOG_FATAL
	};
}

inline bool operator==(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) == 0; }
inline bool operator!=(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) != 0; }
inline bool operator<(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) < 0; }
inline bool operator>(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) > 0; }

#endif // !QCVM_COMMON_HPP
