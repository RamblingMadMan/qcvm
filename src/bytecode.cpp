#define QCVM_IMPLEMENTATION

#include "qcvm/bytecode.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <span>
#include <mutex>

extern "C"{

struct QC_ByteCode{
	const QC_Allocator *allocator;
	std::vector<QC_Statement> stmts;
	std::vector<QC_Def> defs;
	std::vector<QC_Field> fields;
	std::vector<QC_Function> fns;
	std::vector<QC_Value> globals;
	std::vector<char> strBuf;
};

QC_ByteCode *qcCreateByteCodeA(const QC_Allocator *allocator, const char *bytes, size_t len){
	if(len < sizeof(QC_Header)){
		qcLogError("invalid bytecode: size smaller than sizeof(QC_Header)");
		return nullptr;
	}

	QC_Header header;
	std::memcpy(&header, bytes, sizeof(header));

	const auto sectionOff = [&](QC_Section section){
		return header.sectionData[(std::size_t(section) * 2)];
	};

	const auto sectionLen = [&](QC_Section section){
		return header.sectionData[(std::size_t(section) * 2) + 1];
	};

	if(header.ver != 0x6){
		qcLogError("invalid bytecode: wrong header version, should be 0x6");
		return nullptr;
	}

	if(header.skip != 0x0){
		qcLogWarn("bad bytecode: wrong header skip value 0x%ux, should be 0x0", header.skip);
	}

	static constexpr auto sectionDataSize = [](QC_Section section) -> std::size_t{
		switch(section){
			case QC_SECTION_STATEMENTS: return sizeof(QC_Statement32);
			case QC_SECTION_DEFS: return sizeof(QC_Def16);
			case QC_SECTION_FIELDS: return sizeof(QC_Field16);
			case QC_SECTION_FUNCTIONS: return sizeof(QC_Function);
			case QC_SECTION_GLOBALS: return 4;
			default: return 1;
		}
	};

	const auto sectionData = [&](QC_Section section){
		auto off = sectionOff(section);
		auto len = sectionLen(section) * sectionDataSize(section);
		return std::span<const char>(bytes + off, len);
	};

	const auto stmtsData = sectionData(QC_SECTION_STATEMENTS);
	const auto defsData  = sectionData(QC_SECTION_DEFS);
	const auto fldsData  = sectionData(QC_SECTION_FIELDS);
	const auto fncsData  = sectionData(QC_SECTION_FUNCTIONS);
	const auto strsData  = sectionData(QC_SECTION_STRINGS);
	const auto glbsData  = sectionData(QC_SECTION_GLOBALS);

	const auto numStmts  = sectionLen(QC_SECTION_STATEMENTS);
	const auto numDefs   = sectionLen(QC_SECTION_DEFS);
	const auto numFields = sectionLen(QC_SECTION_FIELDS);
	const auto numFns    = sectionLen(QC_SECTION_FUNCTIONS);
	const auto numGlbs   = sectionLen(QC_SECTION_GLOBALS);

	// "all allocations"
	std::vector<QC_Statement> stmts;
	std::vector<QC_Def> defs;
	std::vector<QC_Field> fields;
	std::vector<QC_Function> fns;
	std::vector<QC_Value> globals;
	std::vector<char> strBuf;

	stmts.reserve(numStmts);
	defs.reserve(numDefs);
	fields.reserve(numFields);
	fns.reserve(numFns);

	for(QC_Uint32 i = 0; i < numGlbs; i++){
		QC_Value val;
		std::memcpy(&val.u32, glbsData.data() + (i * sizeof(QC_Uint32)), sizeof(QC_Uint32));
		globals.emplace_back(val);
	}

	strBuf.resize(strsData.size());
	std::memcpy(strBuf.data(), strsData.data(), strsData.size()); // strBuf done

	for(QC_Uint32 i = 0; i < numStmts; i++){
		QC_Statement32 stmt;
		std::memcpy(&stmt, stmtsData.data() + (i * sizeof(stmt)), sizeof(stmt));

		if(stmt.op >= QC_OP_COUNT){
			qcLogError("invalid bytecode: unknown instruction 0x%ux", stmt.op);
			return nullptr;
		}

		stmts.push_back(stmt);
	}

	for(QC_Uint32 i = 0; i < numDefs; i++){
		QC_Def16 def;
		std::memcpy(&def, defsData.data() + (i * sizeof(def)), sizeof(def));

		if(def.nameIdx >= strsData.size()){
			qcLogError("invalid name index %u", def.nameIdx);
			return nullptr;
		}
		else if(def.globalIdx >= glbsData.size()){
			qcLogError("Invalid global index %u in def '%s'", def.globalIdx, strsData.data() + def.nameIdx);
			return nullptr;
		}

		defs.push_back(
			QC_Def{ .type = def.type, .globalIdx = def.globalIdx, .nameIdx = def.nameIdx }
		);
	}

	for(QC_Uint32 i = 0; i < numFields; i++){
		QC_Field16 field;
		std::memcpy(&field, fldsData.data() + (i * sizeof(field)), sizeof(field));

		if(field.nameIdx >= strsData.size()){
			qcLogError("invalid field name index %u", field.nameIdx);
			return nullptr;
		}
		else if(field.type >= QC_TYPE_COUNT){
			qcLogError("unrecognized type id 0x%ux for field '%s'", field.type, strsData.data() + field.nameIdx);
			return nullptr;
		}
		else if(field.offset >= glbsData.size()){
			qcLogError("invalid offset %u for field '%s'", field.offset, strsData.data() + field.nameIdx);
			return nullptr;
		}

		fields.push_back(QC_Field{ .type = field.type, .offset = field.offset, .nameIdx = field.nameIdx });
	}

	for(QC_Uint32 i = 0; i < numFns; i++){
		auto &fn = fns.emplace_back();
		std::memcpy(&fn, fncsData.data() + (i * sizeof(fn)), sizeof(fn));

		if(fn.nameIdx >= strsData.size()){
			qcLogError("invalid function name index %u", fn.nameIdx);
			return nullptr;
		}
		else if(fn.entryPoint > 0 && fn.entryPoint >= stmtsData.size()){
			qcLogError("invalid entry point %d for function '%s'", fn.entryPoint, strsData.data() + fn.nameIdx);
			return nullptr;
		}
		else if(fn.fileIdx >= strsData.size()){
			qcLogWarn("invalid file index %d for function '%s'", fn.fileIdx, strsData.data() + fn.nameIdx);
			return nullptr;
		}
	}

	auto bcMem = qcAllocA(allocator, sizeof(QC_ByteCode), alignof(QC_ByteCode));
	if(!bcMem){
		qcLogError("failed to allocate memory for QC_Bytecode");
		return nullptr;
	}

	auto p = new(bcMem) QC_ByteCode;

	p->allocator = allocator;
	p->stmts = std::move(stmts);
	p->defs = std::move(defs);
	p->fields = std::move(fields);
	p->fns = std::move(fns);
	p->globals = std::move(globals);
	p->strBuf = std::move(strBuf);

	return p;
}

bool qcDestroyByteCode(QC_ByteCode *bc){
	if(!bc) return false;

	const auto allocator = bc->allocator;

	std::destroy_at(bc);

	if(!qcFreeA(allocator, bc)){
		qcLogError("failed to free memory at 0x%p, WARNING! OBJECT DESTROYED!", bc);
		return false;
	}

	return true;
}

QC_Uint32 qcByteCodeStringsSize(const QC_ByteCode *bc){ return bc->strBuf.size(); }
const char *qcByteCodeStrings(const QC_ByteCode *bc){ return bc->strBuf.data(); }

QC_Uint32 qcByteCodeNumStatements(const QC_ByteCode *bc){ return bc->stmts.size(); }
const QC_Statement *qcByteCodeStatements(const QC_ByteCode *bc){ return bc->stmts.data(); }

QC_Uint32 qcByteCodeNumDefs(const QC_ByteCode *bc){ return bc->defs.size(); }
const QC_Def *qcByteCodeDefs(const QC_ByteCode *bc){ return bc->defs.data(); }

QC_Uint32 qcByteCodeNumFields(const QC_ByteCode *bc){ return bc->fields.size(); }
const QC_Field *qcByteCodeFields(const QC_ByteCode *bc){ return bc->fields.data(); }

QC_Uint32 qcByteCodeNumFunctions(const QC_ByteCode *bc){ return bc->fns.size(); }
const QC_Function *qcByteCodeFunctions(const QC_ByteCode *bc){ return bc->fns.data(); }

QC_Uint32 qcByteCodeNumGlobals(const QC_ByteCode *bc){ return bc->globals.size(); }
const QC_Value *qcByteCodeGlobals(const QC_ByteCode *bc){ return bc->globals.data(); }

struct QC_ByteCodeBuilder{
	std::mutex mut;
	QC_ByteCode bc;
};

QC_ByteCodeBuilder *qcCreateBuilder(){
	const auto mem = std::aligned_alloc(alignof(QC_ByteCodeBuilder), sizeof(QC_ByteCodeBuilder));
	if(!mem){
		qcLogError("failed to allocate memory for QC_ByteCodeBuilder");
		return nullptr;
	}

	const auto p = new(mem) QC_ByteCodeBuilder;

	return p;
}

bool qcDestroyBuilder(QC_ByteCodeBuilder *builder){
	if(!builder) return false;
	std::destroy_at(builder);
	std::free(builder);
	return true;
}

QC_ByteCode *qcBuilderEmit(QC_ByteCodeBuilder *builder){
	const auto mem = std::aligned_alloc(alignof(QC_ByteCode), sizeof(QC_ByteCode));
	if(!mem){
		qcLogError("failed to allocate memory for QC_ByteCode");
		return nullptr;
	}

	const auto p = new(mem) QC_ByteCode;

	*p = builder->bc;

	return p;
}

QC_Uint32 qcBuilderAddStatement(QC_ByteCodeBuilder *builder, const QC_Statement *stmt){
	if(!builder || !stmt){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}

	if(stmt->op >= QC_OP_COUNT){
		qcLogError("unrecognized op code 0x%ux", stmt->op);
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);
	const auto idx = builder->bc.stmts.size();
	builder->bc.stmts.emplace_back(*stmt);
	return idx;
}

QC_Uint32 qcBuilderAddDef(QC_ByteCodeBuilder *builder, const QC_Def *def){
	if(!builder || !def){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);

	if(def->nameIdx >= builder->bc.strBuf.size()){
		qcLogError("invalid name string index %u", def->nameIdx);
		return UINT32_MAX;
	}
	else if(def->type >= QC_TYPE_COUNT){
		qcLogError("unrecognized type code 0x%ux for def '%s'", def->type, builder->bc.strBuf.data() + def->nameIdx);
		return UINT32_MAX;
	}
	else if(def->globalIdx >= builder->bc.globals.size()){
		qcLogError("invalid global index %u for def '%s'", def->globalIdx, builder->bc.strBuf.data() + def->nameIdx);
		return UINT32_MAX;
	}

	const auto idx = builder->bc.defs.size();
	builder->bc.defs.emplace_back(*def);
	return idx;
}

QC_Uint32 qcBuilderAddField(QC_ByteCodeBuilder *builder, const QC_Field *field){
	if(!builder || !field){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);

	if(field->nameIdx >= builder->bc.strBuf.size()){
		qcLogError("invalid name string index %u", field->nameIdx);
		return UINT32_MAX;
	}
	else if(field->type >= QC_TYPE_COUNT){
		qcLogError("unrecognized type code 0x%ux for field '%s'", field->type, builder->bc.strBuf.data() + field->nameIdx);
		return UINT32_MAX;
	}

	const auto idx = builder->bc.fields.size();
	builder->bc.fields.emplace_back(*field);
	return idx;
}

QC_Uint32 qcBuilderAddFunction(QC_ByteCodeBuilder *builder, const QC_Function *fn){
	if(!builder || !fn){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);

	// Errors
	if(fn->nameIdx >= builder->bc.strBuf.size()){
		qcLogError("invalid name string index %u", fn->nameIdx);
		return UINT32_MAX;
	}
	else if(fn->entryPoint >= builder->bc.stmts.size()){
		qcLogError(
			"invalid entry point 0x%ux for function '%s'",
			fn->entryPoint, builder->bc.strBuf.data() + fn->nameIdx
		);
		return UINT32_MAX;
	}
	else if(fn->numArgs > 8){
		qcLogError(
			"invalid number of arguments %u (max 8) for function '%s'",
			fn->numArgs, builder->bc.strBuf.data() + fn->nameIdx
		);
		return UINT32_MAX;
	}

	// Warnings
	if(fn->fileIdx >= builder->bc.strBuf.size()){
		qcLogWarn(
			"invalid file name string index %u for function '%s'",
			fn->fileIdx, builder->bc.strBuf.data() + fn->nameIdx
		);
	}

	if(fn->profile != 0){
		qcLogWarn(
			"invalid profile value 0x%ux (should be 0x0) for function '%s",
			(QC_Uint32)fn->profile, builder->bc.strBuf.data() + fn->nameIdx
		);
	}

	const auto idx = builder->bc.fns.size();
	builder->bc.fns.emplace_back(*fn);
	return idx;
}

QC_Uint32 qcBuilderAddGlobal(QC_ByteCodeBuilder *builder, QC_Value value){
	if(!builder){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);
	const auto idx = builder->bc.globals.size();
	builder->bc.globals.emplace_back(value);
	return idx;
}

QC_Uint32 qcBuilderAddString(QC_ByteCodeBuilder *builder, const char *str, size_t len){
	if(!builder){
		qcLogError("NULL argument passed");
		return UINT32_MAX;
	}
	else if(!str || !len){
		qcLogError("invalid string argument");
		return UINT32_MAX;
	}
	else if(str[len - 1] != '\0'){
		qcLogError("passed string is not null-terminated");
		return UINT32_MAX;
	}

	std::scoped_lock lock(builder->mut);
	const auto idx = builder->bc.strBuf.size();
	builder->bc.strBuf.insert(builder->bc.strBuf.end(), str, str + len);
	return idx;
}

QC_Uint32 qcTypeSize(QC_Uint32 type){
#define QCVM_UNIMPLEMENTED_SIZE(case_) \
    case case_: qcLogError("unimplemented type code 0x%ux (" #case_ ")", case_); return UINT32_MAX

	switch(type){
		case QC_TYPE_VOID: return 0;
		case QC_TYPE_STRING: return 1;
		case QC_TYPE_FLOAT: return 1;
		case QC_TYPE_VECTOR: return 3;
		case QC_TYPE_ENTITY: return 1;
		case QC_TYPE_FIELD: return 1;
		case QC_TYPE_FUNC: return 1;

		case QC_TYPE_INT32:
		case QC_TYPE_UINT32:
			return 1;

		case QC_TYPE_INT64:
		case QC_TYPE_UINT64:
			return 1;

		case QC_TYPE_DOUBLE:
			return 1;

		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_VARIANT);
		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_STRUCT);
		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_UNION);
		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_ACCESSOR);
		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_ENUM);
		QCVM_UNIMPLEMENTED_SIZE(QC_TYPE_BOOL);
//		case QC_TYPE_VARIANT: return sizeof(QC_Uint64);
//		case QC_TYPE_STRUCT: return sizeof(QC_Uint64);
//		case QC_TYPE_UNION: return sizeof(QC_Uint64);
//		case QC_TYPE_ACCESSOR: return sizeof(QC_Uint64);
//		case QC_TYPE_ENUM: return sizeof(QC_Enum);
//		case QC_TYPE_BOOL: return sizeof(QC_Bool);

		default:{
			qcLogError("invalid type code 0x%ux", type);
			return UINT32_MAX;
		}
	}
}

}
