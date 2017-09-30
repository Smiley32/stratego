cmake_minimum_required(VERSION 3.0)
project(STRATEGO
  VERSION 0.1.0
)
find_package(gf REQUIRED)
find_package(Boost REQUIRED system)
add_executable(test_client
  test.cpp
)
add_executable(test_client2
  test2.cpp
)
add_executable(test_serv
  serv.cpp
)
target_link_libraries(test_client
  gf::gf0
)
target_link_libraries(test_client2
  gf::gf0
)
target_link_libraries(test_serv
  gf::gf0
)
