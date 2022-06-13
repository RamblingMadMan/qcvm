#include "qcvm/builtins.h"

#include <array>

extern "C" {

static const QC_BuiltinInfo qcvm_defaultBuiltinsInfo[] = {
	{ .index = 9,	.name = "normalize",	.retType = QC_TYPE_VECTOR,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 12,  .name = "vlen",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 26,	.name = "ftos",			.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 27,	.name = "vtos",			.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 36,	.name = "rint",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 37,	.name = "floor",		.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 38,	.name = "ceil",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 43,	.name = "fabs",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 81,	.name = "stof",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
};

const QC_BuiltinInfo *qcDefaultBuiltinsInfo(size_t *nRet){
	if(nRet) *nRet = std::size(qcvm_defaultBuiltinsInfo);
	return qcvm_defaultBuiltinsInfo;
}

static const QC_BuiltinInfo qcvm_quakeBuiltinsInfo[] = {
	{ .index = 1,	.name = "makevectors",		.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 2,	.name = "setorigin",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_VECTOR } },
	{ .index = 3,	.name = "setmodel",			.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_STRING } },
	{ .index = 4,	.name = "setsize",			.retType = QC_TYPE_VOID,	.nParams = 3, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_VECTOR, QC_TYPE_VECTOR } },
	{ .index = 6,	.name = "break",			.retType = QC_TYPE_VOID,	.nParams = 0, .paramTypes = {} },
	{ .index = 7,	.name = "random",			.retType = QC_TYPE_FLOAT,	.nParams = 0, .paramTypes = {} },
	{ .index = 8,	.name = "sound",			.retType = QC_TYPE_VOID,	.nParams = 5, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_FLOAT, QC_TYPE_STRING, QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 11,	.name = "objerror",			.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 13,	.name = "vectoyaw",			.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 14,	.name = "spawn",			.retType = QC_TYPE_ENTITY,	.nParams = 0, .paramTypes = {} },
	{ .index = 15,	.name = "remove",			.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 16,	.name = "traceline",		.retType = QC_TYPE_VOID,	.nParams = 4, .paramTypes = { QC_TYPE_VECTOR, QC_TYPE_VECTOR, QC_TYPE_FLOAT, QC_TYPE_ENTITY } },
	{ .index = 17,	.name = "checkclient",		.retType = QC_TYPE_ENTITY,	.nParams = 0, .paramTypes = {} },
	{ .index = 18,	.name = "find",				.retType = QC_TYPE_ENTITY,	.nParams = 3, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_STRING, QC_TYPE_STRING } },
	{ .index = 19,	.name = "precache_sound",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 20,	.name = "precache_model",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 21,	.name = "stuffcmd",			.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_STRING } },
	{ .index = 22,	.name = "findradius",		.retType = QC_TYPE_ENTITY,	.nParams = 2, .paramTypes = { QC_TYPE_VECTOR, QC_TYPE_FLOAT } },
	{ .index = 23,	.name = "bprint",			.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_STRING } },
	{ .index = 24,	.name = "sprint",			.retType = QC_TYPE_VOID,	.nParams = 3, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_FLOAT, QC_TYPE_STRING } },
	{ .index = 25,	.name = "dprint",			.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 28,	.name = "coredump",			.retType = QC_TYPE_VOID,	.nParams = 0, .paramTypes = {} },
	{ .index = 29,	.name = "traceon",			.retType = QC_TYPE_VOID,	.nParams = 0, .paramTypes = {} },
	{ .index = 30,	.name = "traceoff",			.retType = QC_TYPE_VOID,	.nParams = 0, .paramTypes = {} },
	{ .index = 31,	.name = "eprint",			.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 32,	.name = "walkmove",			.retType = QC_TYPE_FLOAT,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 34,	.name = "droptofloor",		.retType = QC_TYPE_FLOAT,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 35,	.name = "lightstyle",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_STRING } },
	{ .index = 40,	.name = "checkbottom",		.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 41,	.name = "pointcontents",	.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 44,	.name = "aim",				.retType = QC_TYPE_VECTOR,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_FLOAT } },
	{ .index = 45,	.name = "cvar",				.retType = QC_TYPE_FLOAT,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 46,	.name = "localcmd",			.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 47,	.name = "nextent",			.retType = QC_TYPE_ENTITY,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 49,	.name = "ChangeYaw",		.retType = QC_TYPE_VOID,	.nParams = 0, .paramTypes = {} },
	{ .index = 51,	.name = "vectoangles",		.retType = QC_TYPE_VECTOR,	.nParams = 1, .paramTypes = { QC_TYPE_VECTOR } },
	{ .index = 52,	.name = "WriteByte",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 53,	.name = "WriteChar",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 54,	.name = "WriteShort",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 55,	.name = "WriteLong",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 56,	.name = "WriteCoord",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 57,	.name = "WriteAngle",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 58,	.name = "WriteString",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_STRING } },
	{ .index = 59,	.name = "WriteEntity",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_FLOAT, QC_TYPE_ENTITY } },
	{ .index = 67,	.name = "movetogoal",		.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_FLOAT } },
	{ .index = 68,	.name = "precache_file",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 69,	.name = "makestatic",		.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 70,	.name = "changelevel",		.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 71,	.name = "cvar_set",			.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_STRING, QC_TYPE_STRING } },
	{ .index = 73,	.name = "centerprint",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_STRING, QC_TYPE_STRING } },
	{ .index = 74,	.name = "ambientsound",		.retType = QC_TYPE_VOID,	.nParams = 3, .paramTypes = { QC_TYPE_STRING, QC_TYPE_FLOAT, QC_TYPE_FLOAT } },
	{ .index = 75,	.name = "precache_model2",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 76,	.name = "precache_sound2",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 77,	.name = "precache_file2",	.retType = QC_TYPE_STRING,	.nParams = 1, .paramTypes = { QC_TYPE_STRING } },
	{ .index = 78,	.name = "setspawnparms",	.retType = QC_TYPE_VOID,	.nParams = 1, .paramTypes = { QC_TYPE_ENTITY } },
	{ .index = 79,	.name = "logfrag",			.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_ENTITY } },
	{ .index = 80,	.name = "infokey",			.retType = QC_TYPE_STRING,	.nParams = 2, .paramTypes = { QC_TYPE_ENTITY, QC_TYPE_STRING } },
	{ .index = 82,	.name = "multicast",		.retType = QC_TYPE_VOID,	.nParams = 2, .paramTypes = { QC_TYPE_VECTOR, QC_TYPE_FLOAT } },
};

const QC_BuiltinInfo *qcQuakeBuiltinsInfo(size_t *nRet){
	if(nRet) *nRet = std::size(qcvm_quakeBuiltinsInfo);
	return qcvm_quakeBuiltinsInfo;
}

}
