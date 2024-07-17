# Qt5 requires that we find individual component
find_package(Qt5 COMPONENTS Widgets Concurrent Test Network)
find_package(Qt5 OPTIONAL_COMPONENTS Gamepad)
if(NOT Qt5_FOUND)
    list(APPEND CONAN_REQUIRED_LIBS "qt/5.14.1@bincrafters/stable")
endif()

#Qt5 optional components
if(Qt5Gamepad_FOUND)
	add_definitions(-DQT_GAMEPAD)
	list(APPEND QT5_OPTIONAL_LIBS Qt5::Gamepad)
	message(STATUS "Gamepad support enabled")
else()
	message(STATUS "Gamepad support disabled (Qt5 Gamepad not found)")
endif()

set(LD_LIBS ${LD_LIBS} Qt5::Widgets Qt5::Concurrent Qt5::Test Qt5::Network ${QT5_OPTIONAL_LIBS})

function(run_moc)
	qt5_wrap_cpp(MOC_FILES_ ${HPP_FILES})
	qt5_wrap_cpp(TEST_MOC_FILES_ ${TEST_HPP_FILES})
	set(MOC_FILES ${MOC_FILES_} PARENT_SCOPE)
	set(TEST_MOC_FILES ${TEST_MOC_FILES_} PARENT_SCOPE)
endfunction(run_moc)

function(copy_Qt_deps target_dir)
    include(Cmake/WindowsCopyFiles.cmake)

    set(DLL_DEST "$<TARGET_FILE_DIR:${target_dir}>/")
    set(Qt5_DLL_DIR "${Qt5_DIR}/../../../bin")
    set(Qt5_PLATFORMS_DIR "${Qt5_DIR}/../../../plugins/platforms/")
    set(Qt5_STYLES_DIR "${Qt5_DIR}/../../../plugins/styles/")
    set(Qt5_IMAGEFORMATS_DIR "${Qt5_DIR}/../../../plugins/imageformats/")
    set(Qt5_RESOURCES_DIR "${Qt5_DIR}/../../../resources/")
    set(PLATFORMS ${DLL_DEST}platforms/)
    set(STYLES ${DLL_DEST}styles/)
    set(IMAGEFORMATS ${DLL_DEST}imageformats/)

    windows_copy_files(${target_dir} ${Qt5_DLL_DIR} ${DLL_DEST}
        Qt5Core$<$<CONFIG:Debug>:d>.*
        Qt5Gui$<$<CONFIG:Debug>:d>.*
        Qt5Network$<$<CONFIG:Debug>:d>.*
        Qt5Widgets$<$<CONFIG:Debug>:d>.*
        Qt5Svg$<$<CONFIG:Debug>:d>.*
        Qt5OpenGL$<$<CONFIG:Debug>:d>.*
        Qt5PrintSupport$<$<CONFIG:Debug>:d>.*
        Qt5MultimediaWidgets$<$<CONFIG:Debug>:d>.*
        Qt5QuickWidgets$<$<CONFIG:Debug>:d>.*
        Qt5Multimedia$<$<CONFIG:Debug>:d>.*
        Qt5Quick$<$<CONFIG:Debug>:d>.*
        Qt5Sql$<$<CONFIG:Debug>:d>.*
        Qt5XmlPatterns$<$<CONFIG:Debug>:d>.*
        Qt5Qml$<$<CONFIG:Debug>:d>.*
        Qt5Xml$<$<CONFIG:Debug>:d>.*
        Qt5Gamepad$<$<CONFIG:Debug>:d>.*
    )

    windows_copy_files(${target_dir} ${Qt5_PLATFORMS_DIR} ${PLATFORMS} qwindows$<$<CONFIG:Debug>:d>.*)
    windows_copy_files(${target_dir} ${Qt5_IMAGEFORMATS_DIR} ${IMAGEFORMATS} *.dll)
endfunction(copy_Qt_deps)
