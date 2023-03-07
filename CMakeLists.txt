# CMakeList.txt : CMake project for Graphics, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)
project("Graphics")

# download and include the CPM package manager
set(CPM_DOWNLOAD_VERSION 0.38.0)

if(CPM_SOURCE_CACHE)
  set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
  set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
  file(DOWNLOAD
       https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
       ${CPM_DOWNLOAD_LOCATION}
  )
endif()

include(${CPM_DOWNLOAD_LOCATION})

# add dependencies
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:glfw/glfw#3.3.8")

# glfw build options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Add source to this project's executable.
add_executable(Graphics "src/Graphics.cpp" "src/GLAD/glad.c" "src/Result.cpp")
set_property(TARGET Graphics PROPERTY CXX_STANDARD 20)
target_link_libraries(Graphics PRIVATE fmt::fmt glfw)

# TODO: Add tests and install targets if needed.
