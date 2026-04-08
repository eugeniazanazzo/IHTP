cmake_minimum_required(VERSION 3.2)
project(integrated_healthcare)
#c++flags
set(CMAKE_CXX_STANDARD 17) # -std=c++17
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wcomment")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(EASYLOCAL_ROOT "external/easylocal-3")
set(BOOST_ROOT "/opt/homebrew/Cellar/boost/1.86.0_2")

find_package(Boost REQUIRED COMPONENTS program_options)
add_library(easylocal INTERFACE)
target_include_directories(easylocal INTERFACE
	${EASYLOCAL_ROOT}/include
)

add_executable(ih
	IH_Main.cc
	IH_Output.cc
	IH_Input.cc
	IH_Helpers.cc
)

target_include_directories(ih PRIVATE 
	${CMAKE_SOURCE_DIR} 
	${CMAKE_SOURCE_DIR}/csv2-master/include
)
target_link_libraries(ih PRIVATE easylocal Boost::program_options)
