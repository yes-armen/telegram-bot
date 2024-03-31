function(link_poco NAME)
  target_link_libraries(${NAME} PocoNet PocoFoundation PocoJSON)
  target_include_directories(${NAME} PRIVATE ${Poco_INCLUDE_DIRS})
endfunction()

add_catch(test_hello_http test.cpp fake/fake_data.cpp SOLUTION_SRCS weather.cpp)
link_poco(test_hello_http)

add_shad_executable(hello_http SOLUTION_SRCS weather.cpp main.cpp)
link_poco(hello_http)
