#define QCVM_IMPLEMENTATION

#include "qcvm/ast.h"
#include "qcvm/string.h"

#include "plf_colony.h"

extern "C" {

struct QC_AstTypeSet{
	const QC_Allocator *allocator;
};

struct QC_Ast{
	const QC_Allocator *allocator;
	bool ownsTypes;

	QC_StringBuffer *strBuf;
	QC_AstTypeSet *types;

	plf::colony<QC_AstNodeStorage> storedNodes;
};

QC_AstTypeSet *qcCreateAstTypeSetA(const QC_Allocator *allocator){
	const auto mem = qcAllocA(allocator, sizeof(QC_AstTypeSet), alignof(QC_AstTypeSet));
	if(!mem){
		qcLogError("failed to allocate memory for QC_AstTypeSet");
		return nullptr;
	}

	const auto p = new(mem) QC_AstTypeSet;

	p->allocator = allocator;

	return p;
}

bool qcDestroyAstTypeSet(QC_AstTypeSet *types){
	if(!types){
		return false;
	}

	const auto allocator = types->allocator;

	std::destroy_at(types);

	if(!qcFreeA(allocator, types)){
		qcLogError("failed to free QC_AstTypeSet memory at 0x%p, WARNING! OBJECT DESTROYED!", types);
		return false;
	}

	return true;
}

QC_Ast *qcCreateAstA(const QC_Allocator *allocator, QC_AstTypeSet *types){
	const auto mem = qcAllocA(allocator, sizeof(QC_Ast), alignof(QC_Ast));
	if(!mem){
		qcLogError("failed to allocate memory for QC_Ast");
		return nullptr;
	}

	const auto p = new(mem) QC_Ast;

	p->allocator = allocator;
	p->ownsTypes = !types;

	p->strBuf = qcCreateStringBufferA(allocator);
	p->types = types ? types : qcCreateAstTypeSetA(allocator);

	return p;
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

	if(ast->ownsTypes && ast->types && !qcDestroyAstTypeSet(ast->types)){
		qcLogError("failed to destroy ast type set");
		return false;
	}

	ast->ownsTypes = false;
	ast->types = nullptr;

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

}
