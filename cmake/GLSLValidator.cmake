# Create a custom target for GLSL validation
add_custom_target(validate-glsl)

# Check if glslangValidator is available
find_program(GLSLANG_VALIDATOR glslangValidator)

if (GLSLANG_VALIDATOR)
	file(GLOB_RECURSE GLSL_SOURCE_FILES ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/data/shaders/*.vert ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/data/shaders/*.frag)

	file(GLOB THIRDPARTY_DATA_DIRS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/**/data LIST_DIRECTORIES true)
	set(GLSL_PATH_LIST "${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/data/shaders")
	list(APPEND GLSL_PATH_LIST "${PROJECT_SOURCE_DIR}/data/core/shaders")
	foreach(THIRDPARTY_DATA_DIR ${THIRDPARTY_DATA_DIRS})
		list(APPEND GLSL_PATH_LIST "${THIRDPARTY_DATA_DIR}/shaders")
	endforeach()

	set(GLSL_PATH "")
	foreach(PATH ${GLSL_PATH_LIST})
		set(GLSL_PATH "${GLSL_PATH}:${PATH}")
	endforeach()
	message(STATUS "${GLSL_PATH}")


	add_custom_command(
		TARGET validate-glsl
		COMMAND ${PROJECT_SOURCE_DIR}/ci/validate-glsl.sh
		${GLSLANG_VALIDATOR} ${GLSL_PATH} ${GLSL_SOURCE_FILES}
		COMMENT "Validating glsl files..."
		VERBATIM
	)
else()
	message(STATUS "glslangValidator not found")
endif()

