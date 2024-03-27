cmake_minimum_required(VERSION 3.8)
project(rpc-dpdk-client CXX)

message("Root directory of the repository: ${CMAKE_SOURCE_DIR}")
message("Root directory of the repository: ${CMAKE_BINARY_DIR}")

add_executable(client main.cc dpdk.cc worker.cc manager.cc net.cc arp-cfg.cc)
target_include_directories(client PRIVATE ${CMAKE_BINARY_DIR}/dpdk-install-dir/include)

# Link DPDK
find_package(PkgConfig REQUIRED)
set(ENV{PKG_CONFIG_PATH} ${CMAKE_BINARY_DIR}/dpdk-install-dir/lib/aarch64-linux-gnu/pkgconfig)
pkg_check_modules(LIBDPDK REQUIRED libdpdk)

target_compile_options(client PRIVATE ${LIBDPDK_STATIC_CFLAGS})
target_link_libraries(client ${LIBDPDK_STATIC_LDFLAGS})
