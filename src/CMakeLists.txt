set (CMAKE_CPP_COMPILER g++)

set (CMAKE_CXX_FLAGS_LIST 
  -Wall -Wextra -Wunused-variable -std=c++14
  )

SET(SRC_LISTS
  sysinfo.cpp
  main.cpp
  rsi_server.cpp
  get_config.cpp
  )
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(rsi_server ${SRC_LISTS})
target_link_libraries(rsi_server numa)
