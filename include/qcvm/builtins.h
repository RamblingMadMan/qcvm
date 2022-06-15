#ifndef QCVM_BUILTINS_H
#define QCVM_BUILTINS_H 1

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QC_VM QC_VM;

typedef struct QC_BuiltinInfo{
	QC_Uint32 index;
	const char *name;
	QC_Type retType;
	QC_Uint32 nParams;
	QC_Type paramTypes[8];
} QC_BuiltinInfo;

QCVM_API const QC_BuiltinInfo *qcDefaultBuiltinsInfo(size_t *nRet);
QCVM_API const QC_BuiltinInfo *qcQuakeBuiltinsInfo(size_t *nRet);

typedef struct QC_DefaultBuiltins{
#define QCVM_BUILTIN(index, name, ...) (*name)(QC_VM *vm __VA_OPT__(,) __VA_ARGS__)
	QC_Vector	QCVM_BUILTIN(9, normalize, QC_Vector v);
	QC_Float	QCVM_BUILTIN(12, vlen, QC_Vector v);
	QC_String	QCVM_BUILTIN(26, ftos, QC_Float f);
	QC_String	QCVM_BUILTIN(27, vtos, QC_Vector v);
	QC_Float	QCVM_BUILTIN(36, rint, QC_Float v);
	QC_Float	QCVM_BUILTIN(37, floor, QC_Float v);
	QC_Float	QCVM_BUILTIN(38, ceil, QC_Float v);
	QC_Float	QCVM_BUILTIN(43, fabs, QC_Float v);
	QC_Float	QCVM_BUILTIN(81, stof, QC_String s);
#undef QCVM_BUILTIN
} QC_DefaultBuiltins;

typedef struct QC_QuakeBuiltins{
#define QCVM_BUILTIN(index, name, ...) (*name)(QC_VM *vm __VA_OPT__(,) __VA_ARGS__)
	void		QCVM_BUILTIN(1, makevectors, QC_Vector ang); // #1
	void		QCVM_BUILTIN(2, setorigin, QC_Entity e, QC_Vector o);
	void		QCVM_BUILTIN(3, setmodel, QC_Entity e, QC_String m);
	void		QCVM_BUILTIN(4, setsize, QC_Entity e, QC_Vector min, QC_Vector max);
	void		QCVM_BUILTIN(6, break_);
	QC_Float	QCVM_BUILTIN(7, random);
	void		QCVM_BUILTIN(8, sound, QC_Entity e, QC_Float chan, QC_String samp, QC_Float vol, QC_Float atten);
	void		QCVM_BUILTIN(11, objerror, QC_String e);
	QC_Float	QCVM_BUILTIN(13, vectoyaw, QC_Vector v);
	QC_Entity 	QCVM_BUILTIN(14, spawn);
	void		QCVM_BUILTIN(15, remove, QC_Entity e);
	void		QCVM_BUILTIN(16, traceline, QC_Vector v1, QC_Vector v2, QC_Float nomonsters, QC_Entity forent);
	QC_Entity	QCVM_BUILTIN(17, checkclient);
	QC_Entity	QCVM_BUILTIN(18, find, QC_Entity start, QC_String field, QC_String match);
	QC_String	QCVM_BUILTIN(19, precache_sound, QC_String s);
	QC_String	QCVM_BUILTIN(20, precache_model, QC_String s);
	void		QCVM_BUILTIN(21, stuffcmd, QC_Entity client, QC_String s);
	QC_Entity	QCVM_BUILTIN(22, findradius, QC_Vector org, QC_Float rad);
	void		QCVM_BUILTIN(23, bprint, QC_Float level, QC_String s);
	void		QCVM_BUILTIN(24, sprint, QC_Entity client, QC_Float level, QC_String s);
	void		QCVM_BUILTIN(25, dprint, QC_String s);
	void		QCVM_BUILTIN(28, coredump);
	void		QCVM_BUILTIN(29, traceon);
	void		QCVM_BUILTIN(30, traceoff);
	void		QCVM_BUILTIN(31, eprint, QC_Entity e);
	QC_Float	QCVM_BUILTIN(32, walkmove, QC_Float yaw, QC_Float dist);
	QC_Float	QCVM_BUILTIN(34, droptofloor, QC_Float yaw, QC_Float dist);
	void		QCVM_BUILTIN(35, lightstyle, QC_Float style, QC_String value);
	QC_Float	QCVM_BUILTIN(40, checkbottom, QC_Entity e);
	QC_Float	QCVM_BUILTIN(41, pointcontents, QC_Vector v);
	QC_Vector	QCVM_BUILTIN(44, aim, QC_Entity e, QC_Float speed);
	QC_Float	QCVM_BUILTIN(45, cvar, QC_String s);
	void		QCVM_BUILTIN(46, localcmd, QC_String s);
	QC_Entity	QCVM_BUILTIN(47, nextent, QC_Entity e);
	void		QCVM_BUILTIN(49, ChangeYaw);
	QC_Vector	QCVM_BUILTIN(51, vectoangles, QC_Vector v);
	void		QCVM_BUILTIN(52, WriteByte, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(53, WriteChar, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(54, WriteShort, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(55, WriteLong, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(56, WriteCoord, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(57, WriteAngle, QC_Float to, QC_Float f);
	void		QCVM_BUILTIN(58, WriteString, QC_Float to, QC_String s);
	void		QCVM_BUILTIN(59, WriteEntity, QC_Float to, QC_Entity e);
	void		QCVM_BUILTIN(67, movetogoal, QC_Float step);
	QC_String	QCVM_BUILTIN(68, precache_file, QC_String s);
	void		QCVM_BUILTIN(69, makestatic, QC_Entity e);
	void		QCVM_BUILTIN(70, changelevel, QC_String s);
	void		QCVM_BUILTIN(71, cvar_set, QC_String var, QC_String val);
	void		QCVM_BUILTIN(73, centerprint, QC_Entity client, QC_String s);
	void		QCVM_BUILTIN(74, ambientsound, QC_String samp, QC_Float vol, QC_Float atten);
	QC_String	QCVM_BUILTIN(75, precache_model2, QC_String s);
	QC_String	QCVM_BUILTIN(76, precache_sound2, QC_String s);
	QC_String	QCVM_BUILTIN(77, precache_file2, QC_String s);
	void		QCVM_BUILTIN(78, setspawnparms, QC_Entity e);
	void		QCVM_BUILTIN(79, logfrag, QC_Entity killer, QC_Entity killee);
	QC_String	QCVM_BUILTIN(80, infokey, QC_Entity e, QC_String key);
	void		QCVM_BUILTIN(82, multicast, QC_Vector where, QC_Float set);
#undef QCVM_BUILTIN
} QC_QuakeBuiltins;

#ifdef __cplusplus
}
#endif

#endif // !QCVM_BUILTINS_H
