#include "qcvm/amath.h"
#define QCVM_IMPLEMENTATION

#include "qcvm/common.hpp"

#include "qcvm/ast.h"
#include "qcvm/string.h"

#include "plf_colony.h"

#include "parallel_hashmap/phmap.h"

#include <vector>

template<>
struct qcvm::DefaultTypeString<QC_Ast>{
	static constexpr auto get() noexcept{
		using namespace qcvm::cstr_literals;
		return "QC_Ast"_cstr;
	}
};

extern "C" {

struct QC_Ast{
	const QC_Allocator *allocator;
	bool ownsTypes;

	QC_StringBuffer *strBuf;
	QC_TypeSet *ts;

	plf::colony<QC_AstNodeStorage> storedNodes;

	std::vector<QC_AstNode*> nodeDeleteList;
};

QC_Ast *qcCreateAstA(const QC_Allocator *allocator, QC_TypeSet *ts){
	const auto ptr = qcvm::detail::createInline<QC_Ast>(allocator);
	if(!ptr){ return nullptr; }

	ptr->allocator = allocator;
	ptr->ownsTypes = !ts;

	ptr->strBuf = qcCreateStringBufferA(allocator);
	ptr->ts = ts ? ts : qcCreateTypeSetA(allocator);

	return ptr;
}


bool qcDestroyAst(QC_Ast *ast){
	if(!ast){
		return false;
	}

	if(ast->strBuf && !qcDestroyStringBuffer(ast->strBuf)){
		qcLogError("failed to destroy ast string buffer");
		return false;
	}

	ast->strBuf = nullptr;

	if(ast->ownsTypes && ast->ts && !qcDestroyTypeSet(ast->ts)){
		qcLogError("failed to destroy ast type set");
		return false;
	}

	ast->ownsTypes = false;
	ast->ts = nullptr;

	const auto allocator = ast->allocator;

	std::destroy_at(ast);

	if(!qcFreeA(allocator, ast)){
		qcLogError("failed to free QC_Ast memory at 0x%p, WARNING! OBJECT DESTROYED!", ast);
		return false;
	}

	return true;
}

const QC_StringBuffer *qcAstStringBuffer(const QC_Ast *ast){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	return ast->strBuf;
}

const QC_TypeSet *qcAstTypes(const QC_Ast *ast){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	return ast->ts;
}

const QC_AstNode_Binding *qcAstCreateBinding(QC_Ast *ast, const QC_Type *type, QC_StrView name, const QC_AstNode *value){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	QC_AstNodeStorage *newNode = &*ast->storedNodes.emplace();

	newNode->binding = QC_AstNode_Binding{
		.QCVM_SUPER_MEMBER = QC_AstNode{ .kind = QC_AST_NODE_BINDING },
		.type = type,
		.name = name,
		.value = value,
	};

	return &newNode->binding;
}

const QC_AstNode_LiteralType *qcAstCreateLiteralType(QC_Ast *ast, const QC_Type *type){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	QC_AstNodeStorage *newNode = &*ast->storedNodes.emplace();

	newNode->literalType = QC_AstNode_LiteralType{
		.QCVM_SUPER_MEMBER = QC_AstNode_Literal{ .QCVM_SUPER_MEMBER = QC_AstNode{ .kind = QC_AST_NODE_LITERAL_TYPE } },
		.value = type
	};

	return &newNode->literalType;
}

const QC_AstNode_LiteralInt *qcAstCreateLiteralInt(QC_Ast *ast, QC_StrView str, int base){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	QC_AstNodeStorage *newNode = &*ast->storedNodes.emplace();

	newNode->literalInt = QC_AstNode_LiteralInt{
		.QCVM_SUPER_MEMBER = QC_AstNode_Literal{ .QCVM_SUPER_MEMBER = QC_AstNode{ .kind = QC_AST_NODE_LITERAL_INT } },
	};

	const auto ustr = std::string(str.ptr, str.len);
	qcInitAintStr(&newNode->literalInt.value, ustr.c_str(), base);

	return &newNode->literalInt;
}

const QC_AstNode_LiteralFloat *qcAstCreateLiteralFloat(QC_Ast *ast, QC_StrView str, int base){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	QC_AstNodeStorage *newNode = &*ast->storedNodes.emplace();

	newNode->literalFloat = QC_AstNode_LiteralFloat{
		.QCVM_SUPER_MEMBER = QC_AstNode_Literal{ .QCVM_SUPER_MEMBER = QC_AstNode{ .kind = QC_AST_NODE_LITERAL_FLOAT } },
	};

	const auto ustr = std::string(str.ptr, str.len);
	qcInitAfloatStr(&newNode->literalFloat.value, ustr.c_str(), base);

	return &newNode->literalFloat;
}

const QC_AstNode_LiteralVector *qcAstCreateLiteralVector(QC_Ast *ast, QC_Uintptr numElems, const QC_AstNode_Literal *const *elements){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	qcLogError("vector literals unimplemented");
	return nullptr;
}

const QC_AstNode_LiteralStr *qcAstCreateLiteralStr(QC_Ast *ast, QC_StrView str){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return nullptr;
	}

	const auto s = qcStringBufferEmplace(ast->strBuf, str);
	if(s == UINT32_MAX){
		qcLogError("could not emplace string in buffer");
		return nullptr;
	}

	QC_AstNodeStorage *newNode = &*ast->storedNodes.emplace();

	newNode->literalStr = QC_AstNode_LiteralStr{
		.QCVM_SUPER_MEMBER = QC_AstNode_Literal{
			.QCVM_SUPER_MEMBER = QC_AstNode{ .kind = QC_AST_NODE_LITERAL_STR },
			.type = QCVM_SUPER(qcTypeString(QC_STRING_UTF8))
		},
		.value = qcString(ast->strBuf, s)
	};

	return &newNode->literalStr;
}

}
