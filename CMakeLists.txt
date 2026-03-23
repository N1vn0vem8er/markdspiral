cmake_minimum_required(VERSION 3.19)
project(markdspiral LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

qt_standard_project_setup()

qt_add_executable(markdspiral
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/editor/editor.h src/editor/editor.cpp
)

include_directories(src)

target_link_libraries(markdspiral
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::WebEngineWidgets
)

include(GNUInstallDirs)

install(TARGETS markdspiral
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET markdspiral
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
