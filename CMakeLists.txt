cmake_minimum_required(VERSION 3.20.2)
project(EventHandling LANGUAGES CXX)



add_executable(event_handler_test
	src/main.cpp
)
# add_library(event_handler


target_compile_features(event_handler_test PRIVATE cxx_std_20)
target_compile_options(event_handler_test PUBLIC -Wall)


target_include_directories(event_handler_test
	PUBLIC
	include
)

# target_link_libraries(event_handler_test PUBLIC ...)
