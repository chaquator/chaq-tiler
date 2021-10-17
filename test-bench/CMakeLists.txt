add_executable(test_bench WIN32 main.cpp)

set_target_properties(
  test_bench
  PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(MINGW)
  target_compile_options(test_bench PRIVATE -flto -municode)
endif()
