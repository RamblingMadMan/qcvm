#ifndef QCVM_COMMON_HPP
#define QCVM_COMMON_HPP 1

#include "common.h"

#include <memory>
#include <algorithm>
#include <utility>

namespace qcvm{
	template<typename T>
	struct Tag{ using type = T; };

	using Int8 = QC_Int8;
	using Uint8 = QC_Uint8;
	using Int16 = QC_Int16;
	using Uint16 = QC_Uint16;
	using Int32 = QC_Int32;
	using Uint32 = QC_Uint32;
	using Int64 = QC_Int64;
	using Uint64 = QC_Uint64;

	using Intptr = QC_Intptr;
	using Uintptr = QC_Uintptr;

	using Float32 = QC_Float32;
	using Float64 = QC_Float64;

	using Vector = QC_Vector;
	using Vec4 = QC_Vec4;

	template<typename T>
	concept QCVM_Derived = requires(T v){ v.QCVM_SUPER_MEMBER; };

	template<QCVM_Derived T>
	inline constexpr auto *super(T *ptr) noexcept{ return &ptr->QCVM_SUPER_MEMBER; }

	template<typename T, typename From>
	concept QCVM_DerivedFrom = std::is_same_v<decltype(super(std::declval<T*>())), From*>;

	namespace detail{
		template<auto Val> struct ValueT{ static constexpr auto value = Val; };

		template<typename T> struct IsQCVM_DerivedT: std::false_type{};
		template<QCVM_Derived Derived> struct IsQCVM_DerivedT<Derived>: std::true_type{};
	}

	template<typename T>
	constexpr bool isQCVM_Derived(){ return detail::IsQCVM_DerivedT<T>::value; }

	struct StrView{
		constexpr StrView(std::string_view str_): str{ str_.data(), str_.size() }{}
		constexpr StrView(QC_StrView str_): str(str_){}

		template<std::size_t N>
		constexpr StrView(const char(&lit)[N]): str{ lit, N-1 }{}

		constexpr operator QC_StrView() const noexcept{ return str; }
		constexpr operator std::string_view() const noexcept{ return std::string_view(str.ptr, str.len); }

		QC_StrView str;
	};

	template<Uintptr N>
	struct ConstStr{
		const char chars[N + 1];

		template<std::size_t M> requires (M == (N + 1))
		constexpr ConstStr(const char (&lit)[M]) noexcept
			: ConstStr(lit, std::make_index_sequence<N>(), detail::ValueT<Uintptr(0)>()){}

		template<std::size_t M> requires (M == (N + 1))
		constexpr ConstStr(const char *cstr, detail::ValueT<M>)
			: ConstStr(cstr, std::make_index_sequence<N>(), detail::ValueT<Uintptr(0)>()){}

		constexpr operator std::string_view() const noexcept{
			return std::string_view(chars, N);
		}

		constexpr std::size_t size() const noexcept{ return N; }
		constexpr const char *data() const noexcept{ return chars; }
		constexpr const char *c_str() const noexcept{ return chars; }

		template<Uintptr From, Uintptr Len = N>
		constexpr auto substr() const noexcept{
			constexpr Uintptr actualFrom = std::min(From, N);
			constexpr Uintptr actualLen = std::min(N - actualFrom, Len);

			if constexpr(actualLen == 0){
				return ConstStr<0>("");
			}

			return ConstStr<actualLen>(chars, std::make_index_sequence<actualLen>(), detail::ValueT<actualFrom>());
		}

		private:
			template<Uintptr Off, std::size_t ... Is>
			constexpr ConstStr(const char *lit, std::index_sequence<Is...>, detail::ValueT<Off>) noexcept
				: chars{ lit[Off + Is]..., '\0' }{}
	};

	template<std::size_t N>
	ConstStr(const char(&)[N]) -> ConstStr<N-1>;

	namespace detail{
		template<std::size_t ... Is, std::size_t ... Js>
		constexpr auto constStrAddImpl(
			ConstStr<sizeof...(Is)> a, std::index_sequence<Is...>,
			ConstStr<sizeof...(Js)> b, std::index_sequence<Js...>
		){
			constexpr char chars[] = { a.chars[Is]..., b.chars[Js]..., '\0' };
			return ConstStr(chars);
		}
	}

	template<std::size_t N, std::size_t M>
	constexpr ConstStr<N + M> operator+(ConstStr<N> a, ConstStr<M> b){
		return detail::constStrAddImpl(
			a, std::make_index_sequence<N>(),
			b, std::make_index_sequence<M>()
		);
	}

	template<std::size_t N, std::size_t M>
	constexpr ConstStr<N + (M - 1)> operator+(ConstStr<N> a, const char(&b)[M]){
		return a + ConstStr(b);
	}

	template<std::size_t N, std::size_t M>
	constexpr ConstStr<(N - 1) + M> operator+(const char(&a)[N], ConstStr<M> b){
		return ConstStr(a) + b;
	}

	namespace cstr_literals{
		template<ConstStr Str>
		consteval auto operator ""_cstr(){ return Str; }
	}

	using namespace cstr_literals;

	namespace detail{
		template<auto Val, typename T = decltype(Val)>
		struct IdT{ static constexpr T value = Val; };

		template<ConstStr Str>
		struct ConstStrGetter{ static constexpr auto get() noexcept{ return Str; } };
	}

	template<typename T> struct DefaultTypeString;

#define QCVM_DEFAULT_TYPE_STR_U(ty, name) template<> struct DefaultTypeString<ty>: detail::ConstStrGetter<#name##_cstr>{}
#define QCVM_DEFAULT_TYPE_STR(ty) QCVM_DEFAULT_TYPE_STR_U(ty, ty)

	QCVM_DEFAULT_TYPE_STR(void);
	QCVM_DEFAULT_TYPE_STR(char);
	QCVM_DEFAULT_TYPE_STR(Uint8);
	QCVM_DEFAULT_TYPE_STR(Int8);
	QCVM_DEFAULT_TYPE_STR(Uint16);
	QCVM_DEFAULT_TYPE_STR(Int16);
	QCVM_DEFAULT_TYPE_STR(Uint32);
	QCVM_DEFAULT_TYPE_STR(Int32);
	QCVM_DEFAULT_TYPE_STR(Uint64);
	QCVM_DEFAULT_TYPE_STR(Int64);

	QCVM_DEFAULT_TYPE_STR_U(QC_StrView, StrView);

	QCVM_DEFAULT_TYPE_STR(Float32);
	QCVM_DEFAULT_TYPE_STR(Float64);

	QCVM_DEFAULT_TYPE_STR(Vector);
	QCVM_DEFAULT_TYPE_STR(Vec4);

#undef QCVM_DEFAULT_TYPE_STR

	template<typename T>
	constexpr auto typeStr() noexcept{ return DefaultTypeString<T>::get(); }

	enum class LogLevel: Uint32{
		info = QC_LOG_INFO,
		warn = QC_LOG_WARN,
		error = QC_LOG_ERROR,
		fatal = QC_LOG_FATAL
	};

	namespace detail{
		template<typename T, typename ... Args>
		inline T *createInline(const QC_Allocator *allocator, Args &&... args){
			const auto mem = qcAllocA(allocator, sizeof(T), alignof(T));
			if(!mem){
				qcLogError("failed to allocate memory for %s", typeStr<T>().c_str());
				return nullptr;
			}

			return new(mem) T{ std::forward<Args>(args)... };
		}

		template<typename T>
		inline bool destroyInline(T *ptr) noexcept{
			if(!ptr) return false;

			const QC_Allocator *allocator = ptr->allocator;

			std::destroy_at(ptr);

			if(!qcFreeA(allocator, ptr)){
				qcLogError("failed to free %s memory at 0x%p, WARNING! OBJECT DESTROYED!", typeStr<T>().c_str());
				return false;
			}

			return true;
		}
	}
}

inline bool operator==(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) == 0; }
inline bool operator!=(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) != 0; }
inline bool operator<(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) < 0; }
inline bool operator>(QC_StrView a, QC_StrView b) noexcept{ return qcStrCmp(a, b) > 0; }

#endif // !QCVM_COMMON_HPP
