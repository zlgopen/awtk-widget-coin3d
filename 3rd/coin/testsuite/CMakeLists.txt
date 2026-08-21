macro(create_testsuite input)
	get_filename_component(FLNAME "${input}" NAME_WE)
	get_filename_component(FLPATH "${input}" PATH)
	string(REGEX REPLACE ".*[/\\]" "" FLSUBFLD "${FLPATH}")
	set(COIN_STR_TEST_CLASS "${FLNAME}")
	file(READ ${PROJECT_SOURCE_DIR}/${input} f0)
	if(f0 MATCHES "#ifdef[ \t]+COIN_TEST_SUITE")
		# message(STATUS "Parse: ${PROJECT_SOURCE_DIR}/${input} - ${FLPATHSUB}${FLNAME}Test.cpp")
		# get first include from file, which we assume is include to tested class
		string(REGEX MATCH "[\n\r]+#include[ \t]<[^\n]+" iclass "${f0}")
		# get block between '#ifdef COIN_TEST_SUITE' and '#endif'
		string(REGEX REPLACE ".*#ifdef[ \t]+COIN_TEST_SUITE" "" f1 "${f0}")
		string(REGEX REPLACE "#endif[ \t/!]+COIN_TEST_SUITE.*" "" f2 "${f1}")
		# get all #include statements within COIN_TEST_SUITE code block
		string(REGEX MATCHALL "#include[ \t]<[^\n]+" i0 "${f2}")
		string(REPLACE ";" "\n" COIN_STR_TEST_INCL "${i0}")
		set(COIN_STR_TEST_INCL "${iclass}\n${COIN_STR_TEST_INCL}")
		# remove #include statements from test code string (moved to ${COIN_STR_TEST_INCL})
		string(REGEX REPLACE "[\n\r ]*#include[ \t]<[^\n]+" "" COIN_STR_TEST_CODE "${f2}")
		# generate new test code file with extracted snippets
		configure_file(TestSuiteTemplate.cmake.in "${FLSUBFLD}${FLNAME}Test.cpp")
	endif()
endmacro()

macro(create_file_test input testfunc filter)
	file(GLOB_RECURSE COIN_IV_FILES RELATIVE ${PROJECT_SOURCE_DIR} ../${input}/*.wrl ../${input}/*.vrml ../${input}/*.gz ../${input}/*.iv)
	string(REGEX REPLACE "[\\./\\-]+" "" TESTSUITENAME "${input}")
	set(COIN_STR_TEST_INCL "")
	set(COIN_STR_TEST_CODE "const char *readError[] = { \"Coin read error:\", 0 };\n")
	set(COIN_STR_TEST_CLASS "${TESTSUITENAME}")
	set(extra_macro_args ${ARGN})
	list(LENGTH extra_macro_args num_extra_args)
	foreach(INPUTFILE ${COIN_IV_FILES})
		if(${num_extra_args} EQUAL 0 OR NOT ("${INPUTFILE}" MATCHES "${extra_macro_args}"))
			string(REGEX REPLACE "[\\./\\-]+" "_" TESTNAME "${INPUTFILE}")
			if (${filter})
				set(COIN_STR_TEST_CODE "${COIN_STR_TEST_CODE}\nBOOST_AUTO_TEST_CASE(${TESTNAME}) { PushMessageSuppressFilters(readError); ResetReadErrorCount(); test_file(\"${PROJECT_SOURCE_DIR}/${INPUTFILE}\", &${testfunc}); PopMessageSuppressFilters(); }")
			else()
				set(COIN_STR_TEST_CODE "${COIN_STR_TEST_CODE}\nBOOST_AUTO_TEST_CASE(${TESTNAME}) { ResetReadErrorCount(); test_file(\"${PROJECT_SOURCE_DIR}/${INPUTFILE}\", &${testfunc}); }")
			endif()
		endif()
	endforeach()
	configure_file(TestSuiteTemplate.cmake.in "${TESTSUITENAME}Test.cpp")
endmacro()

# Parse all source files for embedded '#ifdef COIN_TEST_SUITE' and extract those blocks
# to separate test source files.
file(GLOB_RECURSE COIN_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src/*.cpp)
foreach(INPUTFILE ${COIN_SRC_FILES})
	create_testsuite(${INPUTFILE})
endforeach()

# Create test cases for all scenes under models folder (this replaces StandardTests.cpp).
# The original StandardTests.cpp also tested additional folders:
# - "killers" with testInCorrectFile
# - "slackers" with testOutOfSpecFile
# However, those folders are not in the repository.
create_file_test("models" "testCorrectFile" false "incorrect")
# bug039.iv and bug055-5.iv contain errors but load anyway: ignore for now
create_file_test("models/io/incorrect" "testIncorrectFile" true "(bug039|bug055-5)")

# Include all extracted '*Tests.cpp' files in the target.
FILE(GLOB COIN_TEST_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/*Test.cpp")

if(COIN_GL_MODERN)
  add_executable(sogl_ff_stubs_compat_decl_test
    ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_ff_stubs_compat_decl_test.cpp)
  target_include_directories(sogl_ff_stubs_compat_decl_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
  )
  add_test(NAME sogl_ff_stubs_compat_decl_test COMMAND sogl_ff_stubs_compat_decl_test)

  add_executable(sogl_ff_stubs_core_decl_test
    ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_ff_stubs_core_decl_test.cpp)
  target_include_directories(sogl_ff_stubs_core_decl_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
  )
  add_test(NAME sogl_ff_stubs_core_decl_test COMMAND sogl_ff_stubs_core_decl_test)

  add_executable(sogl_ff_stubs_gles3_decl_test
    ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_ff_stubs_gles3_decl_test.cpp)
  target_include_directories(sogl_ff_stubs_gles3_decl_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include
  )
  add_test(NAME sogl_ff_stubs_gles3_decl_test COMMAND sogl_ff_stubs_gles3_decl_test)
endif()

add_executable(sogl_es_gldouble_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es_gldouble_test.cpp)
target_include_directories(sogl_es_gldouble_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_es_gldouble_test COMMAND sogl_es_gldouble_test)

add_executable(sogl_es_desktop_api_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es_desktop_api_test.cpp)
target_include_directories(sogl_es_desktop_api_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_es_desktop_api_test COMMAND sogl_es_desktop_api_test)

add_executable(sogl_es_glad_api_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es_glad_api_test.cpp)
target_include_directories(sogl_es_glad_api_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_es_glad_api_test COMMAND sogl_es_glad_api_test)

add_executable(sogl_es_glx_decl_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es_glx_decl_test.cpp)
target_include_directories(sogl_es_glx_decl_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_es_glx_decl_test COMMAND sogl_es_glx_decl_test)

add_executable(sogl_es_glversion_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es_glversion_test.cpp)
target_include_directories(sogl_es_glversion_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_es_glversion_test COMMAND sogl_es_glversion_test)

add_executable(sogl_es2_getstringi_decl_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_es2_getstringi_decl_test.cpp)
add_test(NAME sogl_es2_getstringi_decl_test COMMAND sogl_es2_getstringi_decl_test)

add_executable(sogl_gl3_linux_vbo_decl_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_gl3_linux_vbo_decl_test.cpp)
target_include_directories(sogl_gl3_linux_vbo_decl_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_gl3_linux_vbo_decl_test COMMAND sogl_gl3_linux_vbo_decl_test)

add_executable(sogl_gl3_core_vbo_decl_test
  ${PROJECT_SOURCE_DIR}/src/rendering/glmodern/sogl_gl3_core_vbo_decl_test.cpp)
target_include_directories(sogl_gl3_core_vbo_decl_test PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)
add_test(NAME sogl_gl3_core_vbo_decl_test COMMAND sogl_gl3_core_vbo_decl_test)

add_executable(CoinTests TestSuiteMain.cpp TestSuiteUtils.cpp TestSuiteMisc.cpp ${COIN_TEST_SOURCES})
set_target_properties(CoinTests PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
target_link_libraries(CoinTests Coin ${COIN_TARGET_LINK_LIBRARIES})
target_include_directories(CoinTests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/include/Inventor/annex
	${PROJECT_BINARY_DIR}/include
	${COIN_TARGET_INCLUDE_DIRECTORIES}
)
if (USE_PTHREAD)
	target_link_libraries(CoinTests pthread)
endif()
add_test(NAME CoinTests COMMAND CoinTests)

# Many warnings are generated from test macros on macOS with Xcode.
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-Wtautological-compare DISABLE_WARNING_TAUTOLOGCOMPARE)
if(DISABLE_WARNING_TAUTOLOGCOMPARE)
	target_compile_options(CoinTests PUBLIC -Wno-tautological-compare)
endif()
