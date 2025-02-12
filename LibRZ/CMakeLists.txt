# Add all the source files needed to build the executable

set(LIBRZ_INCLUDEDIR include)
set(LIBRZ_SRCDIR     src)
set(LIBRZ_GRAMMARDIR grammar)

set(CMAKE_CXX_STANDARD 17)

set(LIBRZ_LIBDIR "${CMAKE_CURRENT_BINARY_DIR}" PARENT_SCOPE)

find_package(BISON REQUIRED)
find_package(Python REQUIRED COMPONENTS Development NumPy)
find_package(Freetype REQUIRED)
find_package(NumPy REQUIRED)

# Make sure CMAKE_INSTALL_LIBDIR is defined for all systems
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()

set(LIBRZ_SOURCES
  ${LIBRZ_SRCDIR}/CompositeElement.cpp
  ${LIBRZ_SRCDIR}/CPURayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/DataProduct.cpp
  ${LIBRZ_SRCDIR}/Element.cpp
  ${LIBRZ_SRCDIR}/ExprTkEvaluator.cpp
  ${LIBRZ_SRCDIR}/FT2Facade.cpp
  ${LIBRZ_SRCDIR}/GenericCompositeModel.cpp
  ${LIBRZ_SRCDIR}/GLModel.cpp
  ${LIBRZ_SRCDIR}/GLRenderEngine.cpp
  ${LIBRZ_SRCDIR}/GLHelpers.cpp
  ${LIBRZ_SRCDIR}/Helpers.cpp
  ${LIBRZ_SRCDIR}/IncrementalRotation.cpp
  ${LIBRZ_SRCDIR}/Library.cpp
  ${LIBRZ_SRCDIR}/Linalg.cpp
  ${LIBRZ_SRCDIR}/Logger.cpp
  ${LIBRZ_SRCDIR}/ModelRenderer.cpp
  ${LIBRZ_SRCDIR}/OMModel.cpp
  ${LIBRZ_SRCDIR}/OpticalElement.cpp
  ${LIBRZ_SRCDIR}/ParserContext.cpp
  ${LIBRZ_SRCDIR}/Random.cpp
  ${LIBRZ_SRCDIR}/RayProcessors.cpp
  ${LIBRZ_SRCDIR}/RayTracingEngine.cpp
  ${LIBRZ_SRCDIR}/Recipe.cpp
  ${LIBRZ_SRCDIR}/ReferenceFrame.cpp
  ${LIBRZ_SRCDIR}/RotatedFrame.cpp
  ${LIBRZ_SRCDIR}/RZGLModel.cpp
  ${LIBRZ_SRCDIR}/Singleton.cpp
  ${LIBRZ_SRCDIR}/SkySampler.cpp
  ${LIBRZ_SRCDIR}/SurfaceShape.cpp
  ${LIBRZ_SRCDIR}/TopLevelModel.cpp
  ${LIBRZ_SRCDIR}/TripodFrame.cpp
  ${LIBRZ_SRCDIR}/TranslatedFrame.cpp
  ${LIBRZ_SRCDIR}/WorldFrame.cpp
  ${LIBRZ_SRCDIR}/Zernike.cpp
  
  ${LIBRZ_SRCDIR}/DataProducts/Scatter.cpp
  ${LIBRZ_SRCDIR}/DataProducts/ScatterTree.cpp

  ${LIBRZ_SRCDIR}/Elements/ApertureStop.cpp
  ${LIBRZ_SRCDIR}/Elements/BenchElement.cpp
  ${LIBRZ_SRCDIR}/Elements/BlockElement.cpp
  ${LIBRZ_SRCDIR}/Elements/CircularWindow.cpp
  ${LIBRZ_SRCDIR}/Elements/ConicLens.cpp
  ${LIBRZ_SRCDIR}/Elements/ConicMirror.cpp
  ${LIBRZ_SRCDIR}/Elements/Detector.cpp
  ${LIBRZ_SRCDIR}/Elements/FlatMirror.cpp
  ${LIBRZ_SRCDIR}/Elements/IdealLens.cpp
  ${LIBRZ_SRCDIR}/Elements/LensletArray.cpp
  ${LIBRZ_SRCDIR}/Elements/Obstruction.cpp
  ${LIBRZ_SRCDIR}/Elements/ParabolicLens.cpp
  ${LIBRZ_SRCDIR}/Elements/ParabolicMirror.cpp
  ${LIBRZ_SRCDIR}/Elements/PhaseScreen.cpp
  ${LIBRZ_SRCDIR}/Elements/RayBeamElement.cpp
  ${LIBRZ_SRCDIR}/Elements/RectangularStop.cpp
  ${LIBRZ_SRCDIR}/Elements/RodElement.cpp
  ${LIBRZ_SRCDIR}/Elements/SphericalLens.cpp
  ${LIBRZ_SRCDIR}/Elements/SphericalMirror.cpp
  ${LIBRZ_SRCDIR}/Elements/StlMesh.cpp
  ${LIBRZ_SRCDIR}/Elements/Tripod.cpp
  ${LIBRZ_SRCDIR}/Elements/TubeElement.cpp

  ${LIBRZ_SRCDIR}/RayProcessors/ApertureStop.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/CircularWindow.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/ConicLens.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/ConicMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/FlatMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/IdealLens.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/InfiniteMirror.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/LensletArray.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/Obstruction.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/PassThrough.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/PhaseScreen.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/RectangularStop.cpp
  ${LIBRZ_SRCDIR}/RayProcessors/SquareFlatSurface.cpp

  ${LIBRZ_SRCDIR}/Surfaces/Array.cpp
  ${LIBRZ_SRCDIR}/Surfaces/Circular.cpp
  ${LIBRZ_SRCDIR}/Surfaces/Conic.cpp
  ${LIBRZ_SRCDIR}/Surfaces/Rectangular.cpp

  ${LIBRZ_SRCDIR}/Samplers/Circular.cpp
  ${LIBRZ_SRCDIR}/Samplers/Map.cpp
  ${LIBRZ_SRCDIR}/Samplers/Point.cpp
  ${LIBRZ_SRCDIR}/Samplers/Ring.cpp
  ${LIBRZ_SRCDIR}/Samplers/Sampler.cpp)

set(LIBRZ_HEADERS
  ${LIBRZ_INCLUDEDIR}/CompositeElement.h
  ${LIBRZ_INCLUDEDIR}/CPURayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Element.h
  ${LIBRZ_INCLUDEDIR}/ElementMacros.h
  ${LIBRZ_INCLUDEDIR}/ExprTkEvaluator.h
  ${LIBRZ_INCLUDEDIR}/exprtk.hpp
  ${LIBRZ_INCLUDEDIR}/FT2Facade.h
  ${LIBRZ_INCLUDEDIR}/GenericCompositeModel.h
  ${LIBRZ_INCLUDEDIR}/GLHelpers.h
  ${LIBRZ_INCLUDEDIR}/GLModel.h
  ${LIBRZ_INCLUDEDIR}/GLRenderEngine.h
  ${LIBRZ_INCLUDEDIR}/Helpers.h
  ${LIBRZ_INCLUDEDIR}/IncrementalRotation.h
  ${LIBRZ_INCLUDEDIR}/Linalg.h
  ${LIBRZ_INCLUDEDIR}/Logger.h
  ${LIBRZ_INCLUDEDIR}/Matrix.h
  ${LIBRZ_INCLUDEDIR}/ModelRenderer.h
  ${LIBRZ_INCLUDEDIR}/OMModel.h
  ${LIBRZ_INCLUDEDIR}/OpticalElement.h
  ${LIBRZ_INCLUDEDIR}/ParserContext.h
  ${LIBRZ_INCLUDEDIR}/Random.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors.h
  ${LIBRZ_INCLUDEDIR}/RayTracingEngine.h
  ${LIBRZ_INCLUDEDIR}/Recipe.h
  ${LIBRZ_INCLUDEDIR}/ReferenceFrame.h
  ${LIBRZ_INCLUDEDIR}/RotatedFrame.h
  ${LIBRZ_INCLUDEDIR}/RZGLModel.h
  ${LIBRZ_INCLUDEDIR}/Singleton.h
  ${LIBRZ_INCLUDEDIR}/SkySampler.h
  ${LIBRZ_INCLUDEDIR}/SurfaceShape.h
  ${LIBRZ_INCLUDEDIR}/TopLevelModel.h
  ${LIBRZ_INCLUDEDIR}/TripodFrame.h
  ${LIBRZ_INCLUDEDIR}/TranslatedFrame.h
  ${LIBRZ_INCLUDEDIR}/WorldFrame.h
  ${LIBRZ_INCLUDEDIR}/Vector.h
  ${LIBRZ_INCLUDEDIR}/Zernike.h

  ${LIBRZ_INCLUDEDIR}/DataProducts/Scatter.h
  ${LIBRZ_INCLUDEDIR}/DataProducts/ScatterTree.h
  
  ${LIBRZ_INCLUDEDIR}/Elements/ApertureStop.h
  ${LIBRZ_INCLUDEDIR}/Elements/BenchElement.h
  ${LIBRZ_INCLUDEDIR}/Elements/BlockElement.h
  ${LIBRZ_INCLUDEDIR}/Elements/CircularWindow.h
  ${LIBRZ_INCLUDEDIR}/Elements/ConicLens.h
  ${LIBRZ_INCLUDEDIR}/Elements/ConicMirror.h
  ${LIBRZ_INCLUDEDIR}/Elements/Detector.h
  ${LIBRZ_INCLUDEDIR}/Elements/FlatMirror.h
  ${LIBRZ_INCLUDEDIR}/Elements/IdealLens.h
  ${LIBRZ_INCLUDEDIR}/Elements/LensletArray.h
  ${LIBRZ_INCLUDEDIR}/Elements/Obstruction.h
  ${LIBRZ_INCLUDEDIR}/Elements/ParabolicLens.h
  ${LIBRZ_INCLUDEDIR}/Elements/ParabolicMirror.h
  ${LIBRZ_INCLUDEDIR}/Elements/PhaseScreen.h
  ${LIBRZ_INCLUDEDIR}/Elements/RayBeamElement.h
  ${LIBRZ_INCLUDEDIR}/Elements/RectangularStop.h
  ${LIBRZ_INCLUDEDIR}/Elements/RodElement.h
  ${LIBRZ_INCLUDEDIR}/Elements/SphericalLens.h
  ${LIBRZ_INCLUDEDIR}/Elements/SphericalMirror.h
  ${LIBRZ_INCLUDEDIR}/Elements/StlMesh.h
  ${LIBRZ_INCLUDEDIR}/Elements/stl_reader.h
  ${LIBRZ_INCLUDEDIR}/Elements/Tripod.h
  ${LIBRZ_INCLUDEDIR}/Elements/TubeElement.h

  ${LIBRZ_INCLUDEDIR}/RayProcessors/ApertureStop.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/CircularWindow.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/ConicLens.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/ConicMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/FlatMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/IdealLens.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/InfiniteMirror.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/LensletArray.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/Obstruction.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/PassThrough.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/PhaseScreen.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/RectangularStop.h
  ${LIBRZ_INCLUDEDIR}/RayProcessors/SquareFlatSurface.h
  
  ${LIBRZ_INCLUDEDIR}/Surfaces/Array.h
  ${LIBRZ_INCLUDEDIR}/Surfaces/Circular.h
  ${LIBRZ_INCLUDEDIR}/Surfaces/Conic.h
  
  ${LIBRZ_INCLUDEDIR}/Samplers/Circular.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Map.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Point.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Ring.h
  ${LIBRZ_INCLUDEDIR}/Samplers/Sampler.h)

if(Python_FOUND)
  set(LIBRZ_SOURCES
    ${LIBRZ_SOURCES}
    ${LIBRZ_SRCDIR}/PythonAPI.cpp
    ${LIBRZ_SRCDIR}/ScriptLoader.cpp)

  set(LIBRZ_HEADERS
    ${LIBRZ_HEADERS}
    ${LIBRZ_INCLUDEDIR}/ScriptLoader.h)
endif()

BISON_TARGET(
  OMFParser 
  ${LIBRZ_GRAMMARDIR}/model.yy
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

add_library(
  RZ
  SHARED
  ${LIBRZ_SOURCES}
  ${LIBRZ_HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/parser.h)

if(Python_FOUND)
  set(LIBRZ_LOCAL_CFLAGS "-DPYTHON_SCRIPT_SUPPORT")
  target_compile_options(RZ PRIVATE ${LIBRZ_LOCAL_CFLAGS})
  target_include_directories(RZ PRIVATE ${Python_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(RZ PRIVATE ${Python_LIBRARIES} ${FREETYPE_LIBRARIES})
endif()

set(LIBRZ_CFLAGS ${LIBRZ_LOCAL_CFLAGS} PARENT_SCOPE)
target_include_directories(RZ PRIVATE ${LIBRZ_INCLUDEDIR} ${PNG++_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(RZ PRIVATE ${PNG++_LIBRARIES})

install(FILES ${LIBRZ_HEADERS} DESTINATION include/RZ)
install(TARGETS RZ DESTINATION ${CMAKE_INSTALL_LIBDIR})
