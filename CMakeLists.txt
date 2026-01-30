cmake_minimum_required(VERSION 3.16.3)
project(EasyChatServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)

# 手动查找 MySQL 库（避免依赖 FindMySQL.cmake）
# 知识点：在 Linux 系统中，MySQL 库通常位于 /usr/lib 或 /usr/lib/x86_64-linux-gnu
# 头文件通常位于 /usr/include/mysql
find_library(MYSQL_LIBRARY NAMES mysqlclient PATHS /usr/lib /usr/lib/x86_64-linux-gnu)
find_path(MYSQL_INCLUDE_DIR NAMES mysql/mysql.h PATHS /usr/include)

if(NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "MySQL library not found")
endif()

if(NOT MYSQL_INCLUDE_DIR)
    message(FATAL_ERROR "MySQL include directory not found")
endif()

message(STATUS "Found MySQL library: ${MYSQL_LIBRARY}")
message(STATUS "Found MySQL include directory: ${MYSQL_INCLUDE_DIR}")

# 收集所有源文件
file(GLOB_RECURSE COMMON_SOURCES "src/common/*.cpp")
file(GLOB_RECURSE NETWORK_SOURCES "src/network/*.cpp")
file(GLOB_RECURSE THREADPOOL_SOURCES "src/threadpool/*.cpp")
file(GLOB_RECURSE DATABASE_SOURCES "src/database/*.cpp")
file(GLOB_RECURSE BUSINESS_SOURCES "src/business/*.cpp")

# 查找线程库
find_package(Threads REQUIRED)
# 查找OpenSSL库
find_package(OpenSSL REQUIRED)

add_executable(EasyChatServer
        src/main.cpp
        ${COMMON_SOURCES}
        ${NETWORK_SOURCES}
        ${THREADPOOL_SOURCES}
        ${DATABASE_SOURCES}
        ${BUSINESS_SOURCES}
        src/common/logger.cpp
)

target_link_libraries(EasyChatServer
        PRIVATE
        Threads::Threads
        ${MYSQL_LIBRARY}
        OpenSSL::Crypto
)

target_include_directories(EasyChatServer PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${MYSQL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(EasyChatServer PRIVATE DEBUG)
endif()

install(TARGETS EasyChatServer DESTINATION bin)
install(FILES config/server.conf DESTINATION config)

