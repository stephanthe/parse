cmake_minimum_required(VERSION 3.10)
project(WebParser)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(LibXml2 REQUIRED)

add_executable(WebParser main.cpp)

target_link_libraries(WebParser CURL::libcurl LibXml2::LibXml2)
