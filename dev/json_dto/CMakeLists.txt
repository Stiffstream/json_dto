cmake_minimum_required(VERSION 3.5)

SET(JSON_DTO json-dto)

include(cmake/target.cmake)

project(${JSON_DTO}) # not to misspell it.

add_library(${JSON_DTO} INTERFACE)
add_library(json-dto::json-dto ALIAS ${JSON_DTO})

IF (${CMAKE_VERSION} VERSION_LESS "3.8.0")
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)
ELSE ()
	target_compile_features(${JSON_DTO} INTERFACE cxx_std_14)
ENDIF ()

get_filename_component(CURRENT_FILE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
get_filename_component(CURRENT_FILE_DIR ${CURRENT_FILE_DIR} DIRECTORY)
SET(JSON_DTO_INCLUDE_PATH ${CURRENT_FILE_DIR})
UNSET(CURRENT_FILE_DIR)

IF (JSON_DTO_FIND_DEPS)
	TARGET_INCLUDE_DIRECTORIES( ${JSON_DTO} INTERFACE ${RAPIDJSON_INCLUDE_DIRS} )
ENDIF ()

# Propagate include directories for json_dto.
TARGET_INCLUDE_DIRECTORIES(${JSON_DTO}
	INTERFACE
		$<BUILD_INTERFACE:${JSON_DTO_INCLUDE_PATH}>
		$<INSTALL_INTERFACE:include>
	)

TARGET_COMPILE_DEFINITIONS(${JSON_DTO} INTERFACE
	-DRAPIDJSON_HAS_STDSTRING=1
	-DRAPIDJSON_HAS_CXX11_RVALUE_REFS=1)

SET(JSON_DTO_HEADERS_ALL
	pub.hpp
	validators.hpp )

IF (JSON_DTO_INSTALL)
	include(GNUInstallDirs)
	include(CMakePackageConfigHelpers)

	SET(JSON_DTO_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/json-dto CACHE STRING
		"Installation directory for cmake files, relative to ${CMAKE_INSTALL_PREFIX}.")
	SET(JSON_DTO_VERSION_CONFIG ${PROJECT_BINARY_DIR}/json-dto-config-version.cmake)
	SET(JSON_DTO_PROJECT_CONFIG ${PROJECT_BINARY_DIR}/json-dto-config.cmake)

	SET(JSON_DTO_INC_DIR ${CMAKE_INSTALL_INCLUDEDIR}/json_dto/ CACHE STRING
		"Installation directory for include files, relative to ${CMAKE_INSTALL_PREFIX}.")

	# Generate the version, config and target files into the build directory.
	write_basic_package_version_file(
		${JSON_DTO_VERSION_CONFIG}
		VERSION ${JSON_DTO_VERSION}
		COMPATIBILITY ExactVersion )

	configure_package_config_file(
		"cmake/json-dto-config.cmake.in"
		${JSON_DTO_PROJECT_CONFIG}
		INSTALL_DESTINATION ${JSON_DTO_CMAKE_DIR}
		PATH_VARS JSON_DTO_VERSION)

	# Install version, config and target files.
	install(
		FILES ${JSON_DTO_PROJECT_CONFIG} ${JSON_DTO_VERSION_CONFIG}
		DESTINATION ${JSON_DTO_CMAKE_DIR})

	set (JSON_DTO_INSTALL_TARGETS ${JSON_DTO})

	install(
		TARGETS ${JSON_DTO_INSTALL_TARGETS}
		EXPORT JSON_DTO_ALL_TARGETS
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		RUNTIME DESTINATION bin
	)

	install(
		EXPORT JSON_DTO_ALL_TARGETS
		NAMESPACE json-dto::
		FILE json-dto-targets.cmake
		DESTINATION ${JSON_DTO_CMAKE_DIR} )

	FOREACH(HEADER_FILE ${JSON_DTO_HEADERS_ALL} )
		get_filename_component( DIR ${HEADER_FILE} PATH )
		install( FILES ${HEADER_FILE} DESTINATION ${JSON_DTO_INC_DIR}/${DIR} )
	ENDFOREACH()

ENDIF ()
