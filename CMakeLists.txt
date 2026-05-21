cmake_minimum_required(VERSION 3.20)

project(checkmate LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(asio REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(checkmate
  src/checkmate.cc
  src/checkmate/handling/auth/pbkdf2.cpp
  src/checkmate/handling/auth/routes.cpp
  src/checkmate/handling/dashboard/routes.cpp
  src/checkmate/handling/guard/routes.cpp
  src/checkmate/handling/admin/routes.cpp
)

add_executable(checkmate-create_user
  cmd/checkmate-create_user.cc
  src/checkmate/handling/auth/pbkdf2.cpp
)

function(checkmate_target_includes target)
  target_include_directories(${target} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  target_include_directories(${target} SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
  )
endfunction()

checkmate_target_includes(checkmate)
target_link_libraries(checkmate PRIVATE asio::asio SQLite3::SQLite3)

checkmate_target_includes(checkmate-create_user)
target_link_libraries(checkmate-create_user PRIVATE SQLite3::SQLite3)
