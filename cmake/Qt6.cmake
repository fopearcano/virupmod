# Qt6 requires that we find individual component
find_package(Qt6 COMPONENTS Widgets Concurrent Test Network OpenGL)
if(NOT Qt6_FOUND)
	list(APPEND CONAN_REQUIRED_LIBS "qt/6.7.1@bincrafters/stable")
endif()

#Qt6 optional components
# QT6GAMEPAD
if(Qt6Gamepad_FOUND)
	add_definitions(-DQT_GAMEPAD)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${QT6GAMEPAD_INCLUDE_DIRS})
	list(APPEND QT6_OPTIONAL_LIBS ${QT6GAMEPAD_LIBRARIES})
	message(STATUS "Gamepad support enabled")
else()
	message(STATUS "Gamepad support disabled (Qt6 Gamepad not found)")
endif()

set(LD_LIBS ${LD_LIBS} Qt6::Widgets Qt6::Concurrent Qt6::Test Qt6::Network Qt6::OpenGL ${QT6_OPTIONAL_LIBS})

function(run_moc)
	qt6_wrap_cpp(MOC_FILES_ ${HPP_FILES})
	qt6_wrap_cpp(TEST_MOC_FILES_ ${TEST_HPP_FILES})
	set(MOC_FILES ${MOC_FILES_} PARENT_SCOPE)
	set(TEST_MOC_FILES ${TEST_MOC_FILES_} PARENT_SCOPE)
endfunction(run_moc)

function(copy_Qt_deps target_dir)
    include(Cmake/WindowsCopyFiles.cmake)

    set(DLL_DEST "$<TARGET_FILE_DIR:${target_dir}>/")
    set(Qt6_DLL_DIR "${Qt6_DIR}/../../../bin")
    set(Qt6_PLATFORMS_DIR "${Qt6_DIR}/../../../plugins/platforms/")
    set(Qt6_STYLES_DIR "${Qt6_DIR}/../../../plugins/styles/")
    set(Qt6_IMAGEFORMATS_DIR "${Qt6_DIR}/../../../plugins/imageformats/")
    set(Qt6_RESOURCES_DIR "${Qt6_DIR}/../../../resources/")
    set(PLATFORMS ${DLL_DEST}platforms/)
    set(STYLES ${DLL_DEST}styles/)
    set(IMAGEFORMATS ${DLL_DEST}imageformats/)

    windows_copy_files(${target_dir} ${Qt6_DLL_DIR} ${DLL_DEST}
        Qt6Core$<$<CONFIG:Debug>:d>.*
        Qt6Gui$<$<CONFIG:Debug>:d>.*
        Qt6Network$<$<CONFIG:Debug>:d>.*
        Qt6Widgets$<$<CONFIG:Debug>:d>.*
        Qt6Svg$<$<CONFIG:Debug>:d>.*
        Qt6OpenGL$<$<CONFIG:Debug>:d>.*
        Qt6PrintSupport$<$<CONFIG:Debug>:d>.*
        Qt6MultimediaWidgets$<$<CONFIG:Debug>:d>.*
        Qt6QuickWidgets$<$<CONFIG:Debug>:d>.*
        Qt6Multimedia$<$<CONFIG:Debug>:d>.*
        Qt6Quick$<$<CONFIG:Debug>:d>.*
        Qt6Sql$<$<CONFIG:Debug>:d>.*
        Qt6XmlPatterns$<$<CONFIG:Debug>:d>.*
        Qt6Qml$<$<CONFIG:Debug>:d>.*
        Qt6Xml$<$<CONFIG:Debug>:d>.*
        Qt6Gamepad$<$<CONFIG:Debug>:d>.*
    )

    windows_copy_files(${target_dir} ${Qt6_PLATFORMS_DIR} ${PLATFORMS} qwindows$<$<CONFIG:Debug>:d>.*)
    windows_copy_files(${target_dir} ${Qt6_IMAGEFORMATS_DIR} ${IMAGEFORMATS} *.dll)
endfunction(copy_Qt_deps)
