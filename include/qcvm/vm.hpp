#ifndef QCVM_VM_HPP
#define QCVM_VM_HPP 1

#include "vm.h"
#include "common.hpp"

#include <stdexcept>
#include <atomic>

namespace qcvm{
	namespace detail{
		template<typename T> struct ToQC_Value;

		template<> struct ToQC_Value<Int8>{ static constexpr QC_Value value(Int8 v){ return QC_Value{ .i8 = v }; } };
		template<> struct ToQC_Value<Uint8>{ static constexpr QC_Value value(Uint8 v){ return QC_Value{ .u8 = v }; } };
		template<> struct ToQC_Value<Int16>{ static constexpr QC_Value value(Int16 v){ return QC_Value{ .i16 = v }; } };
		template<> struct ToQC_Value<Uint16>{ static constexpr QC_Value value(Uint16 v){ return QC_Value{ .u16 = v }; } };
		template<> struct ToQC_Value<Int32>{ static constexpr QC_Value value(Int32 v){ return QC_Value{ .i32 = v }; } };
		template<> struct ToQC_Value<Uint32>{ static constexpr QC_Value value(Uint32 v){ return QC_Value{ .u32 = v }; } };
		template<> struct ToQC_Value<Int64>{ static constexpr QC_Value value(Int64 v){ return QC_Value{ .i64 = v }; } };
		template<> struct ToQC_Value<Uint64>{ static constexpr QC_Value value(Uint64 v){ return QC_Value{ .u64 = v }; } };
		template<> struct ToQC_Value<Float32>{ static constexpr QC_Value value(Float32 v){ return QC_Value{ .f32 = v }; } };
		template<> struct ToQC_Value<Float64>{ static constexpr QC_Value value(Float64 v){ return QC_Value{ .f64 = v }; } };
		template<> struct ToQC_Value<Vector>{ static constexpr QC_Value value(Vector v){ return QC_Value{ .v32 = v }; } };
		template<> struct ToQC_Value<Vec4>{ static constexpr QC_Value value(Vec4 v){ return QC_Value{ .v4f32 = v }; } };
	}

	template<> struct DefaultTypeString<QC_VM>: detail::ConstStrGetter<"VM"_cstr>{};

	class VM{
		public:
			VM(Uint32 flags, const QC_Allocator *allocator = QC_DEFAULT_ALLOC)
				: m_vm(qcCreateVMA(allocator, flags))
			{
				if(!m_vm){
					throw std::runtime_error("error in qcCreateVMA");
				}
			}

			VM(VM &&other) noexcept
				: m_vm(other.m_vm.exchange(nullptr)){}

			VM(const VM&) = delete;

			~VM(){
				const auto oldPtr = m_vm.exchange(nullptr);
				qcDestroyVM(oldPtr);
			}

			VM &operator=(VM &&other) noexcept{
				if(this != &other){
					const auto oldPtr = m_vm.exchange(other.m_vm.exchange(nullptr));
					qcDestroyVM(oldPtr);
				}
				return *this;
			}

			VM &operator=(const VM&) = delete;

			void resetDefaultBuiltins() noexcept{
				qcVMResetDefaultBuiltins(m_vm.load(std::memory_order_relaxed));
			}

			/*
			bool qcVMSetBuiltin(QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native fn, bool overrideExisting);
			bool qcVMGetBuiltin(const QC_VM *vm, QC_Uint32 index, QC_VM_Fn_Native *ret);
			 */

			template<typename Ret, typename ... Args>
				requires (detail::ToQC_Value<Ret>::value && (detail::ToQC_Value<Args>::value && ...))
			bool setBuiltin(Uint32 index, Ret(*fptr)(Args...), bool overrideExisting = true){
				const QC_VM_Fn_Native nativeFn = {
					.QCVM_SUPER_MEMBER = QC_VM_Fn{
						.type = QC_VM_FN_BUILTIN,
						.nameIdx = 0,
					},
					.retType = toQC_Type<Ret>(),
					.nParams = sizeof...(Args),
					.paramTypes = { toQC_Type<Args>()... },
					.ptr = [](QC_VM *vm, void *user, void **args) -> QC_Value{
						const auto fptr = *reinterpret_cast<Ret(**)(Args...)>(user);
						return applyArgs(args, fptr, std::index_sequence_for<Args...>());
					},
					.user = fptr
				};

				return qcVMSetBuiltin(
					m_vm, index,
					nativeFn,
					overrideExisting
				);
			}

		private:
			template<typename Ret, typename ... Args, std::size_t ... Is>
			static QC_Value applyArgs(void **args, Ret(*fptr)(Args...), std::index_sequence<Is...>){
				static_assert(sizeof...(Args) == sizeof...(Is));

				if constexpr(std::is_same_v<Ret, void>){
					fptr(*reinterpret_cast<Args*>(args[Is])...);
					return QC_Value{ .v4f32 = qcVec4All(0.f) };
				}
				else{
					return detail::ToQC_Value<Ret>::value(fptr(*reinterpret_cast<Args*>(args[Is])...));
				}
			}

			std::atomic<QC_VM*> m_vm;
	};
}

#endif // !QCVM_VM_HPP
