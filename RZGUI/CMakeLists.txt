cmake_minimum_required(VERSION 3.5)

project(RZGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(OpenGL REQUIRED)
find_package(OSMesa REQUIRED)
find_package(Freetype REQUIRED)

set(PROJECT_SOURCES
        icons.qrc
        main.cpp
        AboutDialog.h
        AboutDialog.cpp
        AboutDialog.ui
        AsyncDataProductRenderer.h
        AsyncDataProductRenderer.cpp
        AsyncRayTracer.h
        AsyncRayTracer.cpp
        BeamPropertiesDialog.h
        BeamPropertiesDialog.cpp
        BeamPropertiesDialog.ui
        ColorChooserButton.cpp
        ColorChooserButton.h
        ColorChooserButton.ui
        ColorSettings.h
        ColorSettings.cpp
        CustomTextEditDelegate.h
        CustomTextEditDelegate.cpp
        DataProductWidget.h
        DataProductWidget.cpp
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
        ExportViewDialog.h
        ExportViewDialog.cpp
        ExportViewDialog.ui
        ExprEvaluationContext.h
        ExprEvaluationContext.cpp
        FootprintInfoWidget.h
        FootprintInfoWidget.cpp
        FootprintInfoWidget.ui
        GUIHelpers.h
        GUIHelpers.cpp
        ImageNavWidget.h
        ImageNavWidget.cpp
        JsonSerializable.h
        JsonSerializable.cpp
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
        ScatterAsyncRenderer.cpp
        ScatterAsyncRenderer.h
        ScatterPainter.cpp
        ScatterPainter.h
        ScatterWidget.h
        ScatterWidget.cpp
        SessionTabWidget.h
        SessionTabWidget.cpp
        SessionTabWidget.ui
        SettingsDialog.h
        SettingsDialog.cpp
        SettingsDialog.ui
        SimulationProgressDialog.h
        SimulationProgressDialog.cpp
        SimulationProgressDialog.ui
        SimulationProperties.h
        SimulationProperties.cpp
        SimulationPropertiesDialog.h
        SimulationPropertiesDialog.cpp
        SimulationPropertiesDialog.ui
        SimulationSession.h
        SimulationSession.cpp
        SourceEditorWindow.h
        SourceEditorWindow.cpp
        SourceEditorWindow.ui
        SpotDiagramWindow.h
        SpotDiagramWindow.cpp
        SpotDiagramWindow.ui)

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
  ${OPENGL_LIBRARIES}
  ${FREETYPE_LIBRARIES})

target_include_directories(
  RZGUI
  PRIVATE ../LibRZ/include
  ${OPENGL_INCLUDE_DIRS}
  ${OSMesa_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS})

set_target_properties(RZGUI PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER rayzaler.harmoni.cab.inta-csic.es
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS RZGUI DESTINATION bin)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(RZGUI)
endif()
