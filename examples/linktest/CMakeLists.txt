cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
include_directories( . /usr/include /usr/local/include /usr/local/include/aria )

add_executable(linktest linktest.cpp)
source_group(" " REGULAR_EXPRESSION "")
link_directories(/usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

#target_link_libraries(linktest aria tcmalloc)
target_link_libraries(linktest aria curl z pthread sqlite3)
