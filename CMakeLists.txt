cmake_minimum_required(VERSION 3.2)
project(MQTT-C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

# Add this include directory for all applications
include_directories(include)

# Common source files
set(MQTT_C_SOURCES src/mqtt.c src/mqtt_pal.c)


# Search for threading library
find_package(Threads REQUIRED)

# Search for openssl
find_package(OpenSSL REQUIRED)

find_package(CMocka REQUIRED)

if(NOT MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=gnu99 -Iinclude -Wno-unused-parameter -Wno-unused-variable -Wno-duplicate-decl-specifier")

    # GCC specific posix socket examples
    foreach(SIMPLE_EXAMPLE IN ITEMS simple_publisher simple_subscriber reconnect_subscriber)
        add_executable(${SIMPLE_EXAMPLE} examples/${SIMPLE_EXAMPLE}.c ${MQTT_C_SOURCES})
        target_link_libraries(${SIMPLE_EXAMPLE} Threads::Threads)
    endforeach()

    # GCC specific BIO socket examples
    foreach(OPENSSL_EXAMPLE IN ITEMS bio_publisher openssl_publisher)
        add_executable(${OPENSSL_EXAMPLE} examples/${OPENSSL_EXAMPLE}.c ${MQTT_C_SOURCES})
        target_link_libraries(${OPENSSL_EXAMPLE} Threads::Threads OpenSSL::SSL)
        target_compile_definitions(${OPENSSL_EXAMPLE} PUBLIC MQTT_USE_BIO)
    endforeach()

    # No platform specific libraries are required
    set(PLATFORM_LIBRARIES )
else()
    # Disable CRT warnings on 'unsecure' api, disable some crap in the windows headers
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX -DWIN32_LEAN_AND_MEAN)

    # Add the Winsock 2 library
    set(PLATFORM_LIBRARIES ws2_32)

    # Here is the fun stuff:
    # By default, the cmocka setup installs the cmocka.dll to ${CMOCKA_ROOT_DIR}\bin\cmocka.dll
    # However, this directory is not added to the path.
    # So, to ensure we can run the unittest, we iterate all configurations (they can be switched without cmake)
    # and copy the cmocka.dll file to their output dir
    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        configure_file("${CMOCKA_ROOT_DIR}/bin/cmocka.dll" "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_TYPE}/" COPYONLY)
    endforeach()
endif()

# Unit-tests
add_executable(tests tests.c ${MQTT_C_SOURCES})
target_include_directories(tests PUBLIC ${CMOCKA_INCLUDE_DIR})
target_link_libraries(tests ${CMOCKA_LIBRARY} ${PLATFORM_LIBRARIES})

