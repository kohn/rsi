set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_LIST 
  -Wall -Wextra -Wunused-variable -g
  )
string (REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_LIST}")

SET(SRC_LISTS
  native_implement_sysinfo.cpp
  main.cpp
  rsi_server.cpp
  tools.cpp
  jsoncpp.cpp
  tinyxml2.cpp
  )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(rsi_server ${SRC_LISTS})
target_link_libraries(rsi_server numa)
target_link_libraries(rsi_server pthread)
target_link_libraries(rsi_server virt)