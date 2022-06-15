set(BUILD_SHARED_LIBS ${QCVM_BUILD_SHARED_LIBS})

add_library(
    qcvm
	${QCVM_C_HEADERS}
	${QCVM_CPP_HEADERS}
	common.cpp
	bytecode.cpp
	vm.cpp
	string.cpp
	builtins.cpp
	lex.cpp
)

target_include_directories(qcvm PUBLIC ${QCVM_INCLUDE_DIR})

target_link_libraries(qcvm PRIVATE phmap plf_colony fmt-header-only)

target_compile_features(qcvm PUBLIC cxx_std_20)
