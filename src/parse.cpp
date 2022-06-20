#define QCVM_IMPLEMENTATION

#include "qcvm/parse.hpp"

#include <memory>

extern "C" {

struct QC_Parser{
	const QC_Allocator *allocator;
	QC_ParseOuterFn outerFn;
	void *user;
};

QC_Parser *qcCreateParserA(const QC_Allocator *allocator, QC_ParseOuterFn outerFn, void *user){
	const auto mem = qcAllocA(allocator, sizeof(QC_Parser), alignof(QC_Parser));
	if(!mem){
		qcLogError("failed to allocate memory for QC_Parser");
		return nullptr;
	}

	const auto p = new(mem) QC_Parser;

	p->allocator = allocator;
	p->outerFn = outerFn;
	p->user = user;

	return p;
}

bool qcDestroyParser(QC_Parser *parser){
	if(!parser){
		return false;
	}

	const auto allocator = parser->allocator;

	std::destroy_at(parser);

	if(!qcFreeA(allocator, parser)){
		qcLogError("failed to free QC_Parser memory at 0x%p, WARNING! OBJECT DESTROYED!");
		return false;
	}

	return true;
}

const QC_AstNode *qcParse(
    QC_Parser *parser,
    QC_Ast *ast,
    QC_Uintptr numToks, const QC_Token *toks,
    QC_Uintptr *nToksRet, const QC_Token **remRet
){
	if(!parser){ return nullptr; }


}

}
