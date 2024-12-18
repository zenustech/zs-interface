cmake_minimum_required(VERSION 3.18)

include(GenerateExportHeader)
# set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

include("project_info.in")
project(zs-interface
	VERSION ${project_version}
	DESCRIPTION ${project_description}
	LANGUAGES C CXX)

option(ZS_INTERFACE_ENABLE_DOC "Build Doc" ON)

if (CMAKE_VERSION VERSION_LESS "3.21")
    # ref: VulkanMemoryAllocator
    # https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
    string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)
endif()
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_path(GET CMAKE_CXX_COMPILER STEM LAST_ONLY ZPC_CXX_COMPILER_BINARY)
message(STATUS "Current C++ compiler is ${ZPC_CXX_COMPILER_BINARY} (${CMAKE_CXX_COMPILER})")
if(${ZPC_CXX_COMPILER_BINARY} STREQUAL "icx")
    add_compile_options(-Wno-error=register -Wno-enum-constexpr-conversion -Wno-implicit-function-declaration)
endif()

include(CheckLanguage)
check_language(CUDA)
if (CMAKE_CUDA_COMPILER)
	enable_language(CUDA)
endif()

set(ZS_ENABLE_SERIALIZATION ON)
# set(ZS_ENABLE_VULKAN ON)
# set(ZS_ENABLE_VULKAN_VALIDATION OFF)

set(ZS_BUILD_SHARED_LIBS ON)
add_subdirectory(zpc EXCLUDE_FROM_ALL)

set(ZS_OVERWRITE_PYTHON_LIBRARIES ${ZS_OVERWRITE_PYTHON_LIBRARIES} PARENT_SCOPE)
set(ZS_OVERWRITE_PYTHON_INCLUDE_DIR ${ZS_OVERWRITE_PYTHON_INCLUDE_DIR} PARENT_SCOPE)
set(ZS_OVERWRITE_PYTHON_EXECUTABLE ${ZS_OVERWRITE_PYTHON_EXECUTABLE} PARENT_SCOPE)
set(ZS_OVERWRITE_PYTHON_LINK_DIRECTORY ${ZS_OVERWRITE_PYTHON_LINK_DIRECTORY} PARENT_SCOPE)
# useful for windows
set(ZS_PYTHON_DLLS ${ZS_PYTHON_DLLS} PARENT_SCOPE)

###############
## interface ##
###############
add_library(zs_interface SHARED 
	zs/interface/world/value_type/BuiltinObjects.cpp
	zs/interface/world/value_type/ValueInterface.cpp
	zs/interface/world/value_type/ValueCApis.cpp
	zs/interface/world/ObjectInterface.cpp
	zs/interface/world/NodeInterface.cpp
	zs/interface/details/Py.cpp
	zs/interface/details/PyHelper.cpp
)
target_include_directories(zs_interface PUBLIC zs)
target_link_libraries(zs_interface PRIVATE zpc_jit_py zpcbase zswhereami)
# target_compile_definitions(zs_interface PRIVATE -DPy_LIMITED_API) # -DPY_SSIZE_T_CLEAN)
# target_compile_definitions(zs_interface PRIVATE -DPY_SSIZE_T_CLEAN)
target_compile_definitions(zs_interface PRIVATE -DZs_Interface_EXPORT)
target_compile_options(zs_interface 
  PUBLIC $<$<COMPILE_LANGUAGE:CXX>: $<IF:$<CXX_COMPILER_ID:MSVC>, /Zc:__cplusplus /utf-8 /bigobj $<IF:$<CONFIG:Debug>, , /O2> /EHsc, $<IF:$<CXX_COMPILER_ID:Clang>, -Xclang -O3, -O3> >> # -fuse-ld=lld -fvisibility=hidden># -flto=thin -fsanitize=cfi
)
if (NOT ${ZS_ENABLE_SYCL_DPCPP} AND NOT ${ZS_ENABLE_SYCL})
target_precompile_headers(zs_interface 
PUBLIC
  zs/interface/details/PyHelper.hpp
  zs/interface/world/value_type/ValueInterface.hpp
  zs/interface/world/ObjectInterface.hpp
  zs/interface/InterfaceExport.hpp
)
endif()

if (ZS_INTERFACE_ENABLE_DOC)
    add_subdirectory(doc)
endif()