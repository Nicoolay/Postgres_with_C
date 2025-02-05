cmake_minimum_required(VERSION 3.20)
project(Postgres_with_C)
add_executable(Postgres_with_C main.cpp)


add_subdirectory (C:/Netology/Homework/SQL/libpqxx  libpqxx-build)

target_compile_features(Postgres_with_C PRIVATE cxx_std_20)

target_link_libraries(Postgres_with_C pqxx)
