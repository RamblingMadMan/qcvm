#ifndef QCVM_TYPES_HPP
#define QCVM_TYPES_HPP

#include "types.h"

#include "common.hpp"

#include <concepts>

namespace qcvm{
	namespace concepts{
		template<typename T>
		concept character = std::same_as<T, char> || std::same_as<T, char8_t>;
	}

	namespace detail{
		using namespace concepts;

		template<QCVM_DerivedFrom<QC_Type> T> struct TypeKindT;
		template<> struct TypeKindT<QC_Type_Void>: IdT<QC_TYPE_VOID>{};
		template<> struct TypeKindT<QC_Type_Bool>: IdT<QC_TYPE_BOOL>{};
		template<> struct TypeKindT<QC_Type_Int>: IdT<QC_TYPE_INT>{};
		template<> struct TypeKindT<QC_Type_Float>: IdT<QC_TYPE_FLOAT>{};
		template<> struct TypeKindT<QC_Type_String>: IdT<QC_TYPE_STRING>{};
		template<> struct TypeKindT<QC_Type_Ptr>: IdT<QC_TYPE_PTR>{};
		template<> struct TypeKindT<QC_Type_Array>: IdT<QC_TYPE_ARRAY>{};
		template<> struct TypeKindT<QC_Type_Vector>: IdT<QC_TYPE_VECTOR>{};
		template<> struct TypeKindT<QC_Type_Function>: IdT<QC_TYPE_FUNCTION>{};

		template<QCVM_DerivedFrom<QC_Type> T> struct IsCompoundTypeT: std::false_type{};
		template<> struct IsCompoundTypeT<QC_Type_Ptr>: std::true_type{};
		template<> struct IsCompoundTypeT<QC_Type_Array>: std::true_type{};
		template<> struct IsCompoundTypeT<QC_Type_Vector>: std::true_type{};
		template<> struct IsCompoundTypeT<QC_Type_Function>: std::true_type{};

		template<typename T> struct TypeQCVMRep;
		template<> struct TypeQCVMRep<void>:		Tag<QC_Type_Void>{};
		template<> struct TypeQCVMRep<bool>:		Tag<QC_Type_Bool>{};
		template<> struct TypeQCVMRep<QC_StrView>:	Tag<QC_Type_String>{};
		template<> struct TypeQCVMRep<Vector>:		Tag<QC_Type_Vector>{};
		template<> struct TypeQCVMRep<Vec4>:		Tag<QC_Type_Vector>{};

		template<character Char, std::size_t N> struct TypeQCVMRep<const Char(&)[N]>: Tag<QC_Type_String>{};

		template<typename T> struct TypeQCVMRep<T*>: Tag<QC_Type_Ptr>{};

		template<typename T, std::size_t N> struct TypeQCVMRep<T[N]>: Tag<QC_Type_Array>{};

		template<typename Ret, typename ... Args> struct TypeQCVMRep<Ret(Args...)>:		Tag<QC_Type_Function>{};
		template<typename Ret, typename ... Args> struct TypeQCVMRep<Ret(*)(Args...)>:	Tag<QC_Type_Function>{};

		template<std::integral Int> struct TypeQCVMRep<Int>: Tag<QC_Type_Int>{};

		template<std::floating_point Float> struct TypeQCVMRep<Float>: Tag<QC_Type_Float>{};
	}

	template<QCVM_DerivedFrom<QC_Type> T>
	inline constexpr bool isCompoundType(){ return detail::IsCompoundTypeT<T>::value; }

	class Type{
		public:
			Type() noexcept: m_type(nullptr){}

			template<QCVM_DerivedFrom<QC_Type> T>
			Type(const T *val) noexcept: m_type(super(val)){}

			operator bool() const noexcept{ return m_type; }
			operator const QC_Type*() const noexcept{ return m_type; }

			const QC_Type *cptr() const noexcept{ return m_type; }

			Uint64 numBits() const noexcept{ return m_type->numBits; }

			template<QCVM_DerivedFrom<QC_Type> T>
			const T *as(){
				if(m_type->kind == Uint32(detail::TypeKindT<T>::value)){
					return reinterpret_cast<const T*>(m_type);
				}
				else{
					return nullptr;
				}
			}

		private:
			const QC_Type *m_type;
	};

	namespace detail{
		template<auto Fn, auto ... Memoized> struct StaticCallT{
			template<typename ... Args>
			static auto get(Args &&... args){
				static auto ret = Fn(std::forward<Args>(args)..., Memoized...);
				return ret;
			}
		};

		template<typename T>
		struct ToType;

		template<typename T> struct ToTypeT;
		template<> struct ToTypeT<void>:		StaticCallT<qcTypeVoid>{};
		template<> struct ToTypeT<bool>:		StaticCallT<qcTypeBool>{};

		template<std::signed_integral Int>
		struct ToTypeT<Int>: StaticCallT<qcTypeInt, QC_INT_TWOS_COMPLIMENT, sizeof(Int) * CHAR_BIT>{};

		template<std::unsigned_integral Uint>
		struct ToTypeT<Uint>: StaticCallT<qcTypeInt, QC_INT_UNSIGNED, sizeof(Uint) * CHAR_BIT>{};

		template<std::floating_point Float>
		struct ToTypeT<Float>: StaticCallT<qcTypeFloat, QC_FLOAT_IEEE754, sizeof(Float) * CHAR_BIT>{};

		template<> struct ToTypeT<QC_StrView>:	StaticCallT<qcTypeString, QC_STRING_UTF8>{};

		template<character Char, std::size_t N>
		struct ToTypeT<const Char(&)[N]>: StaticCallT<qcTypeString, QC_STRING_UTF8>{};

		template<typename T>
		struct ToTypeT<T*>{
			static auto get(QC_TypeSet *ts){
				const auto inner = ToType<T>::get(ts);
				return qcTypePtr(ts, Type(inner));
			}
		};

		template<typename T, std::size_t N>
		struct ToType<T[N]>{
			static auto get(QC_TypeSet *ts){
				static const auto inner = ToType<T>::get(ts);
				return qcTypeArray(ts, Type(inner), N);
			}
		};

		template<typename Ret, typename ... Args>
		struct ToTypeT<Ret(Args...)>{
			static auto get(QC_TypeSet *ts){
				const QC_Type *retTy = Type(ToType<Ret>::get(ts));
				const QC_Type *argTys[] = { Type(ToType<Args>::get(ts))... };
				static const std::size_t numArgs = sizeof...(Args);
				return qcTypeFunction(ts, retTy, numArgs, argTys, 0);
			}
		};

		template<typename Ret, typename ... Args>
		struct ToTypeT<Ret(*)(Args...)>: ToTypeT<Ret(Args...)>{};

		template<>
		struct ToTypeT<QC_Vector>{
			static auto get(QC_TypeSet *ts){
				return qcTypeVector(ts, Type(ToTypeT<Float32>::get()), 3);
			}
		};

		template<>
		struct ToTypeT<QC_Vec4>{
			static auto get(QC_TypeSet *ts){
				return qcTypeVector(ts, Type(ToTypeT<Float32>::get()), 4);
			}
		};

		template<typename T>
		struct ToType{
			static const auto get(QC_TypeSet *ts = nullptr){
				if constexpr(isCompoundType<typename detail::TypeQCVMRep<T>::type>()){
					return ToTypeT<T>::get(ts);
				}
				else{
					return ToTypeT<T>::get();
				}
			}
		};
	}

	template<typename T>
	inline Type toType(){
		static_assert(!detail::IsCompoundTypeT<T>::value);
		return detail::ToType<T>::get();
	}

	template<typename T>
	inline Type toType(QC_TypeSet *ts){
		return detail::ToType<T>::get(ts);
	}
}

#endif //QCVM_TYPES_HPP
