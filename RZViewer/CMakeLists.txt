set(RZVIEWER_INCLUDEDIR include)
set(RZVIEWER_SRCDIR src)

find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(OSMesa REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	${RZVIEWER_SRCDIR}/*.c
	${RZVIEWER_SRCDIR}/*.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	${RZVIEWER_INCLUDEDIR}/*.h
	${RZVIEWER_INCLUDEDIR}/*.hpp)

add_executable(RZViewer ${HEADER_FILES} ${SOURCE_FILES})

target_link_directories(RZViewer PRIVATE ${LIBRZ_LIBDIR})
target_link_libraries(RZViewer PRIVATE RZ ${OSMesa_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
target_compile_options(RZViewer PRIVATE ${LIBRZ_CFLAGS})

target_include_directories(
  RZViewer
  PRIVATE ../LibRZ/include
  ${RZVIEWER_INCLUDEDIR}
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${OSMesa_INCLUDE_DIRS})
