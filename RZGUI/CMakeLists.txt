cmake_minimum_required(VERSION 3.5)

project(RZGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(OSMesa REQUIRED)

set(PROJECT_SOURCES
        icons.qrc
        main.cpp
        AsyncRayTracer.h
        AsyncRayTracer.cpp
        ColorChooserButton.cpp
        ColorChooserButton.h
        ColorChooserButton.ui
        CustomTextEditDelegate.h
        CustomTextEditDelegate.cpp
        DetectorWindow.h
        DetectorWindow.cpp
        DetectorWindow.ui
        DOFAdjustWidget.h
        DOFAdjustWidget.cpp
        DOFAdjustWidget.ui
        DOFWidget.h
        DOFWidget.cpp
        DOFWidget.ui
        ElementPropertyModel.h
        ElementPropertyModel.cpp
        GUIHelpers.h
        GUIHelpers.cpp
        ImageNavWidget.h
        ImageNavWidget.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        OMTreeModel.h
        OMTreeModel.cpp
        PropertyAndDofExprModel.h
        PropertyAndDofExprModel.cpp
        PropertyAndDofTableModel.h
        PropertyAndDofTableModel.cpp
        RZGUI.h
        RZGUI.cpp
        RZGUIGLWidget.h
        RZGUIGLWidget.cpp
        RZMHighLighter.h
        RZMHighLighter.cpp
        SessionTabWidget.h
        SessionTabWidget.cpp
        SessionTabWidget.ui
        SimulationProgressDialog.h
        SimulationProgressDialog.cpp
        SimulationProgressDialog.ui
        SimulationPropertiesDialog.h
        SimulationPropertiesDialog.cpp
        SimulationPropertiesDialog.ui
        SimulationSession.h
        SimulationSession.cpp
        SourceEditorWindow.h
        SourceEditorWindow.cpp
        SourceEditorWindow.ui)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RZGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RZGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(RZGUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RZGUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_directories(RZGUI PRIVATE ${LIBRZ_LIBDIR})
target_compile_options(RZGUI PRIVATE ${LIBRZ_CFLAGS})

target_link_libraries(
  RZGUI PRIVATE
  RZ
  ${OSMesa_LIBRARIES}
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::OpenGLWidgets
  ${OPENGL_LIBRARIES})

target_include_directories(
  RZGUI
  PRIVATE ../LibRZ/include
  ${OPENGL_INCLUDE_DIRS}
  ${OSMesa_INCLUDE_DIRS})

set_target_properties(RZGUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER rayzaler.harmoni.cab.inta-csic.es
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RZGUI)
endif()
