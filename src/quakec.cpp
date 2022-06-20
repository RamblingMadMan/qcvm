#define QCVM_IMPLEMENTATION

#include "qcvm/common.h"
#include "qcvm/lex.h"
#include "qcvm/quakec.h"

bool qcParseQuakeCDef(
	QC_Ast *ast,
	const QC_Token *it, const QC_Token *const beg, const QC_Token *const end
){
	if(it == end){

	}
	return false;
}

extern "C" {

bool qcParseQuakeC(QC_Ast *ast, QC_Uint32 standard, const QC_Token *toks, QC_Uint64 numToks){
	if(!ast){
		qcLogError("NULL ast argument passed");
		return false;
	}

	const auto beg = toks;
	const auto end = toks + numToks;
	const auto it = beg;



	return false;
}

};
