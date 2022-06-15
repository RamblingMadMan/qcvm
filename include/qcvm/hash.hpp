#ifndef QCVM_HASH_HPP
#define QCVM_HASH_HPP 1

#include "common.hpp"

#include <string_view>

namespace qcvm{
	class Fnv1aHash{
		public:
			static constexpr Uint32 offsetBasis32 = 0x811c9dc5;
			static constexpr Uint64 offsetBasis64 = 0xcbf29ce484222325;

			static constexpr Uint32 prime32 = 0x01000193;
			static constexpr Uint64 prime64 = 0x100000001b3;

			static constexpr Uint32 hash32(const std::string_view str) noexcept{
				Uint32 hash = offsetBasis32;

				for(const auto c : str){
					hash ^= (Uint8)c;
					hash *= prime32;
				}

				return hash;
			}

			static constexpr Uint64 hash64(const std::string_view str) noexcept{
				Uint64 hash = offsetBasis64;

				for(const auto c : str){
					hash ^= (Uint8)c;
					hash *= prime64;
				}

				return hash;
			}

			static constexpr auto hash(const std::string_view str) noexcept{
				#ifdef QCVM_HASH_64
				return hash64(str);
				#else
				return hash32(str);
				#endif
			}
	};

	namespace hash_literals{
		constexpr Uint32 operator""_hash32(const char *str, std::size_t len) noexcept{
			return Fnv1aHash::hash32(std::string_view(str, len));
		}

		constexpr Uint64 operator""_hash64(const char *str, std::size_t len) noexcept{
			return Fnv1aHash::hash64(std::string_view(str, len));
		}

		constexpr Uint32 operator""_hash(const char *str, std::size_t len) noexcept{
			return Fnv1aHash::hash(std::string_view(str, len));
		}
	}

	template<typename T>
	class DefaultHash;

	template<>
	class DefaultHash<std::string_view>: public Fnv1aHash{};

	template<typename T>
	constexpr auto hash(const T &val) noexcept(noexcept(DefaultHash<T>::hash(val))){
		return DefaultHash<T>::hash(val);
	}
}

#endif // !QCVM_HASH_HPP
