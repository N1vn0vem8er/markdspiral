cmake_minimum_required(VERSION 3.19)
project(markdspiral LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)
find_library(MD4C_HTML_LIB NAMES md4c-html)

qt_standard_project_setup()

qt_add_executable(markdspiral
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/editor/editor.h src/editor/editor.cpp
    src/editor/markdownhighlighter.h src/editor/markdownhighlighter.cpp
)

include_directories(src)

target_link_libraries(markdspiral
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::WebEngineWidgets
        ${MD4C_HTML_LIB}
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
