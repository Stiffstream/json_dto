cmake_minimum_required(VERSION 3.5)

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
	# ------------------------------------------------------------------------------
	# json_dto dependencies:

	IF (JSON_DTO_FIND_DEPS)
		# Require necessary packages.
		find_package(RapidJSON REQUIRED)
	ELSE ()
		include_directories(rapidjson/include)
	ENDIF ()

ENDIF (JSON_DTO_MASTER_PROJECT)

# json_dto itself.
add_subdirectory(json_dto)

# ------------------------------------------------------------------------------
# Tests
IF (JSON_DTO_TEST)
	enable_testing()
	add_subdirectory(test)

	IF (WIN32)
		configure_file(${CMAKE_SOURCE_DIR}/cmake/run_tests.bat ${CMAKE_BINARY_DIR} NEWLINE_STYLE WIN32)
	ENDIF ()
ENDIF ()

# ------------------------------------------------------------------------------
# Samples
IF (JSON_DTO_SAMPLE)
	add_subdirectory(sample)
ENDIF ()
