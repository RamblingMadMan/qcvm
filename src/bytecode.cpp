#include "qcvm/bytecode.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <span>
#include <mutex>

extern "C"{

struct QC_ByteCode{
	std::vector<QC_Statement> stmts;
	std::vector<QC_Def> defs;
	std::vector<QC_Field> fields;
	std::vector<QC_Function> fns;
	std::vector<QC_Uint32> globals;
	std::vector<char> strBuf;
};

QC_ByteCode *qcLoadByteCode(const char *bytes, size_t len){
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

	std::vector<QC_Statement> stmts;
	std::vector<QC_Def> defs;
	std::vector<QC_Field> fields;
	std::vector<QC_Function> fns;
	std::vector<QC_Uint32> globals;
	std::vector<char> strBuf;

	stmts.reserve(numStmts);
	defs.reserve(numDefs);
	fields.reserve(numFields);
	fns.reserve(numFns);

	globals.resize(numGlbs);
	std::memcpy(globals.data(), glbsData.data(), glbsData.size() * sizeof(QC_Uint32)); // globals done

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

	auto bcMem = std::aligned_alloc(alignof(QC_ByteCode), sizeof(QC_ByteCode));
	if(!bcMem){
		qcLogError("failed to allocate memory for QC_Bytecode");
		return nullptr;
	}

	auto p = new(bcMem) QC_ByteCode;

	p->stmts = std::move(stmts);
	p->defs = std::move(defs);
	p->fields = std::move(fields);
	p->fns = std::move(fns);
	p->globals = std::move(globals);
	p->strBuf = std::move(strBuf);

	return p;
}

bool qcFreeByteCode(QC_ByteCode *bc){
	if(!bc) return false;
	std::destroy_at(bc);
	std::free(bc);
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
const QC_Uint32 *qcByteCodeGlobals(const QC_ByteCode *bc){ return bc->globals.data(); }

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
		qcLogError("unrecognized type code 0x%ux", def->type);
		return UINT32_MAX;
	}
	else if(def->globalIdx)
}

}
