set(BUILD_SHARED_LIBS ${QCVM_BUILD_SHARED_LIBS})

find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

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
	ast.cpp)

target_include_directories(qcvm PUBLIC ${QCVM_INCLUDE_DIR} ${GMP_INCLUDE_DIR} ${MPFR_INCLUDE_DIR})

target_link_libraries(qcvm PRIVATE phmap plf_colony fmt-header-only)
target_link_libraries(qcvm PUBLIC ${GMP_LIBRARIES} ${MPFR_LIBRARIES})

target_compile_features(qcvm PUBLIC cxx_std_20)
