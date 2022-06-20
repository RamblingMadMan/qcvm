#ifndef QCVM_BYTECODE_HPP
#define QCVM_BYTECODE_HPP 1

#include "bytecode.h"

#include "common.hpp"

#include <optional>

namespace qcvm{
	class ByteCodeFunction{
		public:
			ByteCodeFunction(QC_ByteCodeFunction val)
				: m_fn(val){}

			operator QC_ByteCodeFunction&() noexcept{ return m_fn; }
			operator const QC_ByteCodeFunction&() const noexcept{ return m_fn; }

			QC_ByteCodeFunction *cptr() noexcept{ return &m_fn; }
			const QC_ByteCodeFunction *cptr() const noexcept{ return &m_fn; }

			Int32 entryPoint() const noexcept{ return m_fn.entryPoint; }
			Int32 localIdx() const noexcept{ return m_fn.localIdx; }
			Uint32 numLocals() const noexcept{ return m_fn.numLocals; }
			Uint32 profile() const noexcept{ return m_fn.profile; }
			Uint32 nameIdx() const noexcept{ return m_fn.nameIdx; }
			Uint32 fileIdx() const noexcept{ return m_fn.fileIdx; }
			Uint32 numArgs() const noexcept{ return m_fn.numArgs; }
			const int8_t *argSizes() const noexcept{ return m_fn.argSizes; }

		private:
			QC_ByteCodeFunction m_fn;
	};

	namespace detail{
		template<typename T> struct ToByteCodeType;
		template<> struct ToByteCodeType<Int32>:	IdT<QC_BYTECODE_TYPE_INT32>{};
		template<> struct ToByteCodeType<Uint32>:	IdT<QC_BYTECODE_TYPE_UINT32>{};
		template<> struct ToByteCodeType<Int64>:	IdT<QC_BYTECODE_TYPE_INT64>{};
		template<> struct ToByteCodeType<Uint64>:	IdT<QC_BYTECODE_TYPE_UINT64>{};
		template<> struct ToByteCodeType<Float32>:	IdT<QC_BYTECODE_TYPE_FLOAT32>{};
		template<> struct ToByteCodeType<Float64>:	IdT<QC_BYTECODE_TYPE_FLOAT64>{};
		template<> struct ToByteCodeType<Vector>:	IdT<QC_BYTECODE_TYPE_VECTOR>{};
	}

	template<typename T>
	constexpr Uint32 toByteCodeType(){ return detail::ToByteCodeType<T>::value; }

	class ByteCode{
		public:
			ByteCode(const char *bytes, std::size_t len, const QC_Allocator *allocator = QC_DEFAULT_ALLOC)
				: m_bc(qcCreateByteCodeA(allocator, bytes, len)){}

			~ByteCode(){ qcDestroyByteCode(m_bc); }

			Uintptr numFunctions() const noexcept{ return m_bc ? qcByteCodeNumFunctions(m_bc) : 0; }

			std::optional<ByteCodeFunction> function(Uintptr idx) const{
				const auto n = numFunctions();
				if(idx >= n) return std::nullopt;
				else return *(qcByteCodeFunctions(m_bc) + idx);
			}

		private:
			QC_ByteCode *m_bc;
	};
}

#endif // !QCVM_BYTECODE_HPP
