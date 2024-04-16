cmake_minimum_required(VERSION 3.8)
project(rpc-dpdk-client CXX)

add_executable(client main.cc net.cc)
target_include_directories(client PRIVATE ${CMAKE_BINARY_DIR}/dpdk-install-dir/include)

target_compile_options(client PRIVATE -Wno-narrowing)

# Link DPDK
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} ${CMAKE_SOURCE_DIR}/../build/dpdk-install-dir/lib/x86_64-linux-gnu/pkgconfig)
pkg_check_modules(LIBDPDK REQUIRED libdpdk)

target_compile_options(client PRIVATE ${LIBDPDK_STATIC_CFLAGS})
target_link_libraries(client ${LIBDPDK_STATIC_LDFLAGS})

# Configure the inter-arrival distribution
# Remember to remove CMakeCache.txt before rebuilding in a different config
if(FIXED)
  target_compile_definitions(client PRIVATE -DFIXED)
  message(STATUS "The interarrival distribution will be FIXED")
elseif(EXPONENTIAL)
  target_compile_definitions(client PRIVATE -DEXPONENTIAL)
  message(STATUS "The interarrival distribution will be EXPONENTIAL")
else()
  message(FATAL_ERROR "No interarrival distribution set. Compilation failed.")
endif()

# Configure the application protocol
if(LOOP)
  target_compile_definitions(client PRIVATE -DLOOP)
  message(STATUS "The application protocol will be LOOP")
else()
  message(FATAL_ERROR "No application protocol. Compilation failed.")
endif()
