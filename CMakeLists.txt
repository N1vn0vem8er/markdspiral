cmake_minimum_required(VERSION 3.19)
project(markdspiral LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets Concurrent)
find_library(MD4C_HTML_LIB NAMES md4c-html)
find_package(Nuspell REQUIRED)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(markdspiral
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/editor/editor.h src/editor/editor.cpp
    src/editor/markdownhighlighter.h src/editor/markdownhighlighter.cpp
    src/widgets/gitwidget.ui src/widgets/gitwidget.h src/widgets/gitwidget.cpp
    src/widgets/gitfilesview.h src/widgets/gitfilesview.cpp
    src/widgets/gitbranchdialog.ui src/widgets/gitbranchdialog.h src/widgets/gitbranchdialog.cpp
    src/widgets/gitfilestatusitemdelegate.h src/widgets/gitfilestatusitemdelegate.cpp
    src/widgets/gitfilestatusmodel.h src/widgets/gitfilestatusmodel.cpp
    resources/styles.qrc
    src/widgets/GitFileStatus.h
    src/processmanager.h src/processmanager.cpp
    src/widgets/runningprocesseslabel.h src/widgets/runningprocesseslabel.cpp
    src/editor/dictionaryprovider.h src/editor/dictionaryprovider.cpp
    src/widgets/filesystemtree.h src/widgets/filesystemtree.cpp
    src/utils/mimefinderworker.h src/utils/mimefinderworker.cpp
    src/widgets/textinputdialog.ui src/widgets/textinputdialog.h src/widgets/textinputdialog.cpp
)

include_directories(src widgets)

target_link_libraries(markdspiral
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::WebEngineWidgets
        ${MD4C_HTML_LIB}
        Nuspell::nuspell
        Qt::Concurrent
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
