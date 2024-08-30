cmake_minimum_required(VERSION 3.5)

project(tetris VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(UNIX)
	set(CURSES_NEED_NCURSES TRUE)
	find_package(Curses REQUIRED)
	find_package(Boost REQUIRED)
	find_package(fmt REQUIRED)
	find_package(spdlog REQUIRED)
endif()

set(PROJECT_SOURCES
        main.cpp

        gui/mainwindow.cpp
        gui/mainwindow.h
        gui/boardrenderer.cpp
        gui/boardrenderer.h
        gui/minorenderer.cpp
        gui/minorenderer.h
        gui/minocolors.h
        gui/timer.cpp
        gui/timer.h

        tui/tuiapp.cpp
        tui/tuiapp.h
        tui/asioterminal.cpp
        tui/asioterminal.h
        tui/asioinput.cpp
        tui/asioinput.h
        tui/asiotimer.cpp
        tui/asiotimer.h
        tui/keycode.h
        tui/keymodifier.h
        tui/character.h
        tui/attribute.h
        tui/size.h
        tui/position.h
        tui/boardrenderer.cpp
        tui/boardrenderer.h
        tui/minorenderer.cpp
        tui/minorenderer.h
        tui/timer.cpp
        tui/timer.h

        game.cpp
        game.h
        inputevent.h
        bagofseven.cpp
        bagofseven.h
        tetrominotype.h
        board.cpp
        board.h
        gridposition.h
        grid.cpp
        grid.h
        tetromino.cpp
        tetromino.h
        offset.h
        rotationdirection.h
        tetrominocolor.h
        itimer.cpp
        itimer.h
        playingtetromino.cpp
        playingtetromino.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_resources(RESOURCES resources.qrc)
else()
    qt5_add_resources(RESOURCES resources.qrc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tetris
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${RESOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tetris APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tetris SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tetris
            ${PROJECT_SOURCES}
	        ${RESOURCES}
        )
    endif()
endif()

target_link_libraries(tetris PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(UNIX)
	target_include_directories(tetris PRIVATE ${CURSES_INCLUDE_DIRS})
	target_link_libraries(tetris PRIVATE ${CURSES_LIBRARIES})
	target_include_directories(tetris PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
	include(CheckIncludeFile)
	check_include_file("term.h" HAVE_TERM_H)
	check_include_file("termio.h" HAVE_TERMIO_H)
	check_include_file("termios.h" HAVE_TERMIOS_H)
	configure_file(curses-config.h.in ${CMAKE_CURRENT_BINARY_DIR}/curses-config.h @ONLY)
	target_link_libraries(tetris PRIVATE Boost::boost fmt::fmt spdlog::spdlog)
endif()


target_include_directories(tetris PRIVATE ${CMAKE_CURRENT_LIST_DIR})

option(TETRIS_TREAT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
# Set the compiler warning options
set(GCC_WARN_OPTS -Wall -Wextra -pedantic)
set(MSVC_WARN_OPTS /W4)
if(TETRIS_TREAT_WARNINGS_AS_ERRORS)
	list(APPEND GCC_WARN_OPTS -Werror)
	list(APPEND MSVC_WARN_OPTS /WX)
endif()
target_compile_options(tetris PRIVATE
	"$<$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>:$<BUILD_INTERFACE:${GCC_WARN_OPTS}>>"
	"$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:$<BUILD_INTERFACE:${MSVC_WARN_OPTS}>>"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.tetris)
endif()
set_target_properties(tetris PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS tetris
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tetris)
endif()
