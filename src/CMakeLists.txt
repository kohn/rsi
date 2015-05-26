set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS_LIST 
  -Wall -Wextra -Wunused-variable -std=c++11 -L${PROJECT_SOURCE_DIR}/lib -g
  )
string (REPLACE ";" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_LIST}")

SET(SRC_LISTS
  native_implement_sysinfo.cpp
  main.cpp
  rsi_server.cpp
  tools.cpp
  json.hpp
  tinyxml2.cpp
  )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(rsi_server ${SRC_LISTS})
target_link_libraries(rsi_server numa)
target_link_libraries(rsi_server vsf)
target_link_libraries(rsi_server proc)
target_link_libraries(rsi_server pthread)
target_link_libraries(rsi_server virt)