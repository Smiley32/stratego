cmake_minimum_required(VERSION 3.0)
project(STRATEGO
  VERSION 0.1.0
)
find_package(gf REQUIRED)
find_package(Boost REQUIRED system)
add_executable(stratego
  test.cpp
  # and the rest of your sources
)
target_link_libraries(stratego
  gf::gf0
) 
