# This file is indended to be included in end-user CMakeLists.txt
# include(/path/to/Filelists.cmake)
# It assumes the variable LWIP_CONTRIB_DIR is defined pointing to the
# root path of lwIP contrib sources.
#
# This file is NOT designed (on purpose) to be used as cmake
# subdir via add_subdirectory()
# The intention is to provide greater flexibility to users to 
# create their own targets using the *_SRCS variables.

set(lwipcontribportunix_SRCS
    ${LWIP_CONTRIB_DIR}/ports/unix/port/sys_arch.c
    ${LWIP_CONTRIB_DIR}/ports/unix/port/perf.c
)

set(lwipcontribportunixnetifs_SRCS
    ${LWIP_CONTRIB_DIR}/ports/unix/port/netif/tapif.c
    ${LWIP_CONTRIB_DIR}/ports/unix/port/netif/list.c
    ${LWIP_CONTRIB_DIR}/ports/unix/port/netif/sio.c
    ${LWIP_CONTRIB_DIR}/ports/unix/port/netif/fifo.c
)

add_library(lwipcontribportunix EXCLUDE_FROM_ALL ${lwipcontribportunix_SRCS} ${lwipcontribportunixnetifs_SRCS})
target_include_directories(lwipcontribportunix PRIVATE ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
target_compile_options(lwipcontribportunix PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwipcontribportunix PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_link_libraries(lwipcontribportunix PUBLIC ${LWIP_MBEDTLS_LINK_LIBRARIES})

install(TARGETS lwipcontribportunix
  EXPORT lwipTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
export(TARGETS lwipcontribportunix
  NAMESPACE lwip::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/lwip/lwipTargets.cmake"
)


if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    find_library(LIBUTIL util)
    find_library(LIBPTHREAD pthread)
    find_library(LIBRT rt)
    target_link_libraries(lwipcontribportunix PUBLIC ${LIBUTIL} ${LIBPTHREAD} ${LIBRT})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # Darwin doesn't have pthreads or POSIX real-time extensions libs
    find_library(LIBUTIL util)
    target_link_libraries(lwipcontribportunix PUBLIC ${LIBUTIL})
endif()
