cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

####This is a hook for developers to use.
####Please have it print out what changes it makes via the CMake's message API.
include(developer.cmake OPTIONAL)

#SVN-EXTERNALS: cmake-modules/CMakePCHCompiler
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake-modules/CMakePCHCompiler)

project(mm3d VERSION 1.3.11 LANGUAGES CXX CXXPCH)
set(PROJECT_VERSION 1.3.11)

message(STATUS "Compiling MM3D Project Version ${PROJECT_VERSION}")
message(STATUS "Using cmake version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" )

message(STATUS "Installing to ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
#cmake-gui doesn't show this option. But be cautious since it may one day.
set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD} CACHE STRING "Must not be 98" FORCE)
mark_as_advanced(CLEAR CMAKE_CXX_STANDARD)
if(NOT CMAKE_CXX_STANDARD OR 98 EQUAL CMAKE_CXX_STANDARD)
	message("CMAKE_CXX_STANDARD is not set or 98. Set to 11")
	set(CMAKE_CXX_STANDARD 11)
endif()

####CMake-GUI options##########################

set(OpenGL_GL_PREFERENCE "GLVND" CACHE STRING "GLVN or LEGACY")

option(MM3D_EDIT "Build application" ON)
if(MM3D_EDIT)
	add_definitions(-DMM3D_EDIT)
endif()

set(MM3D_PREFIX "$ENV{PREFIX}" CACHE STRING "Autotools style PREFIX")	

#Transitioning from Autotools MM3D needs PREFIX.
add_definitions(-DPREFIX="${CMAKE_INSTALL_PREFIX}")

#GCC has __BYTE_ORDER__.
#Win32 is little endian.
if(NOT WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
#Transitioning from Autotools endianconfig.h wants BYTEORDER.
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
# message(STATUS "BIG_ENDIAN")
add_definitions(-DBYTEORDER=4321)
else()
add_definitions(-DBYTEORDER=1234)
endif()
endif()

####BUILD/INSTALL###############################

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -D_DEBUG")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG")
string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -DNDEBUG")
string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -DNDEBUG")

if(CMAKE_COMPILER_IS_GNUCXX)
add_compile_options(-frtti -fvisibility=hidden -fpermissive)
endif()

#NOTE: OpenGL includes GLU.
find_package(OpenGL REQUIRED)

#GLU included.
set(mm3d_libs ${OPENGL_LIBRARIES})
set(mm3d_incl ${OPENGL_INCLUDE_DIR})

#REMOVE ME
list(APPEND mm3d_incl "src" "src/libmm3d" "src/mm3dcore" "src/implui")
list(APPEND mm3d_incl "src/commands" "src/tools")

file(GLOB libmm3d_files "src/libmm3d/*.cc")
add_library(libmm3d STATIC ${libmm3d_files})
target_include_directories(libmm3d
      PUBLIC
#      "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
      ${mm3d_incl})
target_link_libraries(libmm3d PUBLIC ${mm3d_libs})
set_target_properties(libmm3d PROPERTIES
  DEBUG_POSTFIX "d"
#  SOVERSION ${PROJECT_VERSION}
  VERSION ${PROJECT_VERSION})
  
target_precompiled_header(libmm3d src/libmm3d/mm3dtypes.h)

if(MM3D_EDIT)

file(GLOB mm3dcore_files "src/mm3dcore/*.cc" "src/commands/*.cc" "src/tools/*.cc")      
add_library(mm3dcore_obj OBJECT ${mm3dcore_files})
target_include_directories(mm3dcore_obj
      PUBLIC
#      "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
      ${mm3d_incl})

target_precompiled_header(mm3dcore_obj src/libmm3d/mm3dtypes.h REUSE libmm3d)

find_package(Widgets95 REQUIRED)

file(GLOB implui_files "src/implui/*.cc" "src/*.cc")     
add_library(implui_obj OBJECT ${implui_files})
target_include_directories(implui_obj ${mm3d_incl})
#NOTE: This lets #include <widgets95.h> work without using an old fashioned
#macro like ${WIDGETS95_INCLUDE_DIRS} that requires manual setup that seems
#to not be recommended by the modern CMake guidelines.
target_link_libraries(implui_obj Widgets95::widgets95)       

target_precompiled_header(implui_obj src/libmm3d/mm3dtypes.h REUSE libmm3d)

if(CYGWIN)
#Cygwin windres won't handle the UTF16-LE files VS exclusively uses!
set(rc_file "src/windres_resource.rc")
else()
set(rc_file "src/win_resource.rc")
endif()
                                       
add_executable(mm3d ${rc_file} $<TARGET_OBJECTS:implui_obj> $<TARGET_OBJECTS:mm3dcore_obj>)
target_link_libraries(mm3d libmm3d ${mm3d_libs} Widgets95::widgets95)
target_precompiled_header(mm3d src/libmm3d/mm3dtypes.h REUSE libmm3d)

endif(MM3D_EDIT)

####
# Installation

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
#set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with function 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
write_basic_package_version_file(
    "${version_config}" VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
configure_package_config_file(
    "Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Targets:
install(
    TARGETS mm3d
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
#    INCLUDES DESTINATION "${include_install_dir}"
)

# Config
install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

# Config
install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)
