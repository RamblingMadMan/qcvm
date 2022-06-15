#include "qcvm/vm.h"
#include "qcvm/lex.h"

#include "qcvm/common.hpp"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <cstdlib>

QC_Value qcvm_printFloatAndDouble(QC_VM*, void*, void **args){
	const auto valPtr = reinterpret_cast<const QC_Float*>(args[0]);
	qcLogInfo("printFloat: %f", *valPtr);
	return { .f32 = *valPtr * 2.f };
}

constexpr QC_StrView lexTest0Src = QC_STRVIEW(
R"(my test ids
123 1.2 2.3
void(int x, float, int64) testBuiltinDef = #0;
)"
);

const QC_Token lexTest0Expected[] = {
	// my test ids
	{ QC_TOKEN_ID,			{ 0, 0 },		QC_STRVIEW("my") },
	{ QC_TOKEN_SPACE,		{ 2, 0 },		QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,			{ 3, 0 },		QC_STRVIEW("test") },
	{ QC_TOKEN_SPACE,		{ 7, 0 },		QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,			{ 8, 0 },		QC_STRVIEW("ids") },
	{ QC_TOKEN_NEWLINE,	{ 11, 0 },	QC_STRVIEW("\n") },

	// 123 1.2 2.3
	{ QC_TOKEN_INT,		{ 0, 1 },		QC_STRVIEW("123") },
	{ QC_TOKEN_SPACE,		{ 3, 1 },		QC_STRVIEW(" ") },
	{ QC_TOKEN_FLOAT,		{ 4, 1 },		QC_STRVIEW("1.2") },
	{ QC_TOKEN_SPACE,		{ 7, 1 },		QC_STRVIEW(" ") },
	{ QC_TOKEN_FLOAT,		{ 8, 1 },		QC_STRVIEW("2.3") },
	{ QC_TOKEN_NEWLINE,	{ 11, 1 },	QC_STRVIEW("\n") },

	// void(int x, float, int64) testBuiltinDef = #0;
	{ QC_TOKEN_ID,		{ 0, 2 },		QC_STRVIEW("void") },
	{ QC_TOKEN_BRACKET,	{ 4, 2 },		QC_STRVIEW("(") },
	{ QC_TOKEN_ID,		{ 5, 2 },		QC_STRVIEW("int") },
	{ QC_TOKEN_SPACE,		{ 8, 2 },		QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,		{ 9, 2 },		QC_STRVIEW("x") },
	{ QC_TOKEN_OP,		{ 10, 2 },	QC_STRVIEW(",") },
	{ QC_TOKEN_SPACE,		{ 11, 2 },	QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,		{ 12, 2 },	QC_STRVIEW("float") },
	{ QC_TOKEN_OP,		{ 17, 2 },	QC_STRVIEW(",") },
	{ QC_TOKEN_SPACE,		{ 18, 2 },	QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,		{ 19, 2 },	QC_STRVIEW("int64") },
	{ QC_TOKEN_BRACKET,	{ 24, 2 },	QC_STRVIEW(")") },
	{ QC_TOKEN_SPACE,		{ 25, 2 },	QC_STRVIEW(" ") },
	{ QC_TOKEN_ID,		{ 26, 2 },	QC_STRVIEW("testBuiltinDef") },
	{ QC_TOKEN_SPACE,		{ 40, 2 },	QC_STRVIEW(" ") },
	{ QC_TOKEN_OP,		{ 41, 2 },	QC_STRVIEW("=") },
	{ QC_TOKEN_SPACE,		{ 42, 2 },	QC_STRVIEW(" ") },
	{ QC_TOKEN_OP,		{ 43, 2 },	QC_STRVIEW("#") },
	{ QC_TOKEN_INT,		{ 44, 2 },	QC_STRVIEW("0") },
	{ QC_TOKEN_OP,		{ 45, 2 },	QC_STRVIEW(";") },
	{ QC_TOKEN_NEWLINE,	{ 46, 2 },	QC_STRVIEW("\n") },
};

TEST_CASE( "default VM initialization", "[vm-init]" ){
	QC_VM *vm = qcCreateVM(0);

	REQUIRE(vm);

	SECTION( "initializing virtual machine defaults" ){
		const auto initialized = qcVMResetDefaultBuiltins(vm);

		REQUIRE(initialized);
	}

	const bool destroyed = qcDestroyVM(vm);

	REQUIRE(destroyed);
}

TEST_CASE( "simple lexing", "[lex]" ){
	QC_Token token;
	QC_SourceLocation nextLoc = { 0, 0 };
	QC_StrView rem = lexTest0Src;

	for(std::size_t i = 0; i < std::size(lexTest0Expected); i++){
		const auto &expected = lexTest0Expected[i];

		rem = qcLex(rem, nextLoc, &token, &nextLoc);

		REQUIRE(rem.ptr);
		REQUIRE(token.kind == expected.kind);
		REQUIRE(token.loc.col == expected.loc.col);
		REQUIRE(token.loc.line == expected.loc.line);
		REQUIRE(token.str == expected.str);
	}
}
