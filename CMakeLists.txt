cmake_minimum_required(VERSION 3.2)
project(MQTT-C)

# Add this include directory for all applications
include_directories(include)

# Common source files
set(MQTT_C_SOURCES src/mqtt.c src/mqtt_pal.c)


# Search for threading library
find_package(Threads REQUIRED)

# Search for openssl
find_package(OpenSSL REQUIRED)

# Search the cmocka  unittest library
find_library(CMOCKA_LIBRARY cmocka)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=gnu99 -Iinclude -Wno-unused-parameter -Wno-unused-variable -Wno-duplicate-decl-specifier")

# Simple projects
add_executable(simple_publisher examples/simple_publisher.c ${MQTT_C_SOURCES})
target_link_libraries(simple_publisher Threads::Threads)

add_executable(simple_subscriber examples/simple_subscriber.c ${MQTT_C_SOURCES})
target_link_libraries(simple_subscriber Threads::Threads)

add_executable(reconnect_subscriber examples/reconnect_subscriber.c ${MQTT_C_SOURCES})
target_link_libraries(reconnect_subscriber Threads::Threads)

# Use BIO sockets
add_executable(bio_publisher examples/bio_publisher.c ${MQTT_C_SOURCES})
target_link_libraries(bio_publisher Threads::Threads OpenSSL::SSL)
target_compile_definitions(bio_publisher PUBLIC MQTT_USE_BIO)

# Encrypt connection using OpenSSL
add_executable(openssl_publisher examples/openssl_publisher.c ${MQTT_C_SOURCES})
target_link_libraries(openssl_publisher Threads::Threads OpenSSL::SSL)
target_compile_definitions(openssl_publisher PUBLIC MQTT_USE_BIO)


# Unit-tests
add_executable(tests tests.c ${MQTT_C_SOURCES})
target_include_directories(tests PUBLIC ${CMOCKA_INCLUDE_DIR})
target_link_libraries(tests ${CMOCKA_LIBRARY})

