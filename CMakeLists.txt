cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
	qcvm
	DESCRIPTION "QuakeC VM"
	VERSION 0.0.1
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(FetchContent)

###
# Configuration
###

set(QCVM_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

option(QCVM_BUILD_SHARED_LIBS "Build the library as a shared library" ON)
option(QCVM_BUILD_TEST "Build the test executable" ${PROJECT_IS_TOP_LEVEL})

configure_file(${QCVM_INCLUDE_DIR}/qcvm/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

###
# Dependencies
###

FetchContent_Declare(
	plf_list_content
	GIT_REPOSITORY https://github.com/mattreecebentley/plf_list.git
	GIT_TAG 6a46224f42f537c76b2dc45ccd3f17f2b8952c32 # latest at time of writing
)

FetchContent_Declare(
	plf_colony_content
	GIT_REPOSITORY https://github.com/mattreecebentley/plf_colony.git
	GIT_TAG 7243b4601b5d7e989550c158a9918ea5a05feaf4 # latest at time of writing
)

FetchContent_Declare(
	parallel_hashmap_content
	GIT_REPOSITORY https://github.com/greg7mdp/parallel-hashmap.git
	GIT_TAG 655dee936b5b65ec0468517dd6728a7ada2b2e02 # latest at time of writing
)

FetchContent_Declare(
	fmtlib_content
	GIT_REPOSITORY https://github.com/fmtlib/fmt.git
	GIT_TAG 8e47cfd1cdfa9cb29cbb5ae037124cd71425a1fe # latest at time of writing
)

set(
	QCVM_CONTENT_DEPS
	plf_list_content
	plf_colony_content
	parallel_hashmap_content
	fmtlib_content
)

if(QCVM_BUILD_TEST)
	FetchContent_Declare(
		catch2_content
		GIT_REPOSITORY https://github.com/catchorg/Catch2.git
		GIT_TAG 20d413b8b6803c6d456874935d7014fe684e9426 # latest v2.x at time of writing
	)

	list(APPEND QCVM_CONTENT_DEPS catch2_content)
endif()

FetchContent_MakeAvailable(${QCVM_CONTENT_DEPS})

add_library(plf_list INTERFACE)
target_include_directories(plf_list INTERFACE ${plf_list_content_SOURCE_DIR})

add_library(plf_colony INTERFACE)
target_include_directories(plf_colony INTERFACE ${plf_colony_content_SOURCE_DIR})

###
# Project targets
###

set(
	QCVM_C_HEADERS
	${QCVM_INCLUDE_DIR}/qcvm/common.h
	${QCVM_INCLUDE_DIR}/qcvm/string.h
	${QCVM_INCLUDE_DIR}/qcvm/amath.h
	${QCVM_INCLUDE_DIR}/qcvm/types.h
	${QCVM_INCLUDE_DIR}/qcvm/bytecode.h
	${QCVM_INCLUDE_DIR}/qcvm/net.h
	${QCVM_INCLUDE_DIR}/qcvm/vm.h
	${QCVM_INCLUDE_DIR}/qcvm/builtins.h
	${QCVM_INCLUDE_DIR}/qcvm/lex.h
	${QCVM_INCLUDE_DIR}/qcvm/ast.h
	${QCVM_INCLUDE_DIR}/qcvm/parse.h
	${QCVM_INCLUDE_DIR}/qcvm/quakec.h
)

set(
	QCVM_CPP_HEADERS
	${QCVM_INCLUDE_DIR}/qcvm/common.hpp
	${QCVM_INCLUDE_DIR}/qcvm/hash.hpp
	${QCVM_INCLUDE_DIR}/qcvm/types.hpp
	${QCVM_INCLUDE_DIR}/qcvm/bytecode.hpp
	${QCVM_INCLUDE_DIR}/qcvm/vm.hpp
	${QCVM_INCLUDE_DIR}/qcvm/parse.hpp
)

add_subdirectory(src)

if(QCVM_BUILD_TEST)
	add_subdirectory(test)
endif()
