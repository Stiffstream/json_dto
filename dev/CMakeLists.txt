cmake_minimum_required(VERSION 3.14)

project(json_dto_root CXX)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Determine if RESTinio is built as a subproject (using add_subdirectory)
# or if it is the master project.
SET(JSON_DTO_MASTER_PROJECT OFF)
IF (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	SET(JSON_DTO_MASTER_PROJECT ON)
ENDIF ()

option(JSON_DTO_INSTALL "Generate the install target." ${JSON_DTO_MASTER_PROJECT})
option(JSON_DTO_TEST "Build the tests." ${JSON_DTO_MASTER_PROJECT})
option(JSON_DTO_SAMPLE "Build samples." ${JSON_DTO_MASTER_PROJECT})
option(JSON_DTO_INSTALL_SAMPLES "Build install samples." ${JSON_DTO_MASTER_PROJECT})
option(JSON_DTO_FIND_DEPS "Get json_dto dependencies with `find_package()`." OFF)

IF (JSON_DTO_MASTER_PROJECT)

	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	# ------------------------------------------------------------------------------
	# json_dto dependencies:

	IF (JSON_DTO_FIND_DEPS)
		# Require necessary packages.
		find_package(RapidJSON REQUIRED)
	ELSE ()
		include(FetchContent)

		set(FETCHCONTENT_QUIET OFF)

		set(RAPIDJSON_BUILD_DOC OFF CACHE INTERNAL "")
		set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE INTERNAL "")
		set(RAPIDJSON_BUILD_TESTS OFF CACHE INTERNAL "")
		set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE INTERNAL "")
		set(RAPIDJSON_HAS_STDSTRING ON CACHE INTERNAL "")

		FetchContent_Declare(rapidjson
			GIT_REPOSITORY https://github.com/Tencent/rapidjson
			GIT_TAG v1.1.0
			GIT_SHALLOW TRUE
			OVERRIDE_FIND_PACKAGE
		)

		set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE INTERNAL "")

		FetchContent_Declare(doctest
			GIT_REPOSITORY https://github.com/doctest/doctest
			GIT_TAG v2.4.12
			OVERRIDE_FIND_PACKAGE
		)

		FetchContent_MakeAvailable(rapidjson doctest)

		# FIXME: document this trick!
		add_library(RapidJSON INTERFACE)
		target_include_directories(RapidJSON INTERFACE ${rapidjson_SOURCE_DIR}/include)

	ENDIF ()

ENDIF (JSON_DTO_MASTER_PROJECT)

# json_dto itself.
add_subdirectory(json_dto)

# ------------------------------------------------------------------------------
# Tests
IF (JSON_DTO_TEST)
	enable_testing()
	add_subdirectory(test)

# FIXME: is it really needed?
#	IF (WIN32)
#		configure_file(${CMAKE_SOURCE_DIR}/cmake/run_tests.bat ${CMAKE_BINARY_DIR} NEWLINE_STYLE WIN32)
#	ENDIF ()
ENDIF ()

# ------------------------------------------------------------------------------
# Samples
IF (JSON_DTO_SAMPLE)
	add_subdirectory(sample)
ENDIF ()

