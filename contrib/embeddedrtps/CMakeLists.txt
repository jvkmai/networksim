project(embeddedRTPS-Desktop)
cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O0 -g -Wall -Wextra -Wnon-virtual-dtor -Wpedantic -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add micro-CDR
add_subdirectory(embeddedrtps/thirdparty/Micro-CDR)

# Add lwIP
SET(LWIP_CONTRIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/contrib")
SET(LWIP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lwip")

SET(PORT_ROOT "${LWIP_CONTRIB_DIR}/ports/unix")
SET(ARCH_DIR "${PORT_ROOT}/port/include")

include(${LWIP_CONTRIB_DIR}/ports/CMakeCommon.cmake)

set (LWIP_DEFINITIONS -DLWIP_DEBUG)

include(${LWIP_DIR}/src/Filelists.cmake)
include(${LWIP_CONTRIB_DIR}/Filelists.cmake)
include(${PORT_ROOT}/Filelists.cmake)

add_library(lwip STATIC ${lwipcore_SRCS} ${lwipcore3_SRCS} ${lwipcontribportunix_SRCS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip/src/include
    ${LWIP_CONTRIB_DIR}
    ${ARCH_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip
    ${WPDPACK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/embeddedrtps/thirdparty/lwip
)

target_compile_options(lwip PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwip PRIVATE ${LWIP_DEFINITIONS})

target_link_libraries(lwip ${LWIP_SANITIZER_LIBS}  lwipcontribaddons lwipallapps lwipcore lwipcontribportunix)

# List of include directories
set(LWIP_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/lwip/src/include"
    "${LWIP_CONTRIB_DIR}"
    "${ARCH_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/lwip"
    "${WPDPACK_INCLUDE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/embeddedrtps/thirdparty/lwip"
)

# Iterate over each directory
foreach(include_dir ${LWIP_INCLUDE_DIRS})
  # Find all header files in the current directory
  file(GLOB_RECURSE LWIP_HEADER_FILES "${include_dir}/*.h")

  # Iterate over each header file
  foreach(header ${LWIP_HEADER_FILES})
    # Calculate the relative path of the header file
    file(RELATIVE_PATH relative_path ${include_dir} ${header})
    get_filename_component(relative_dir ${relative_path} DIRECTORY)

    # Create the corresponding directory in build/include/
    set(destination "${CMAKE_CURRENT_SOURCE_DIR}/../../build/include/${relative_dir}")
    file(MAKE_DIRECTORY ${destination})

    # Write a stub header that includes the original header
    get_filename_component(header_name ${header} NAME)
    file(WRITE ${destination}/${header_name}
      "#include \"${header}\"\n"
    )
  endforeach()
endforeach()

#get_target_property(LWIP_INCLUDE_DIRS lwip INCLUDE_DIRECTORIES)
#message("lwip include directories = ${LWIP_INCLUDE_DIRS}")

install(TARGETS lwip
        EXPORT lwipTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT lwipTargets
        FILE lwipTargets.cmake
        NAMESPACE lwip::
        DESTINATION lib/cmake/lwip)


# embeddedRTPS
add_library(embeddedrtps "")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lwip
    ${CMAKE_CURRENT_SOURCE_DIR}/embeddedrtps/include
    ${microcdr_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rtps
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip/src/include
)

target_sources(embeddedrtps
        PRIVATE
        "embeddedrtps/src/rtps.cpp"
        "embeddedrtps/src/ThreadPool.cpp"
        "embeddedrtps/src/discovery/ParticipantProxyData.cpp"
        "embeddedrtps/src/discovery/SPDPAgent.cpp"
        "embeddedrtps/src/discovery/SEDPAgent.cpp"
        "embeddedrtps/src/discovery/TopicData.cpp"
        "embeddedrtps/src/entities/Domain.cpp"
        "embeddedrtps/src/entities/Participant.cpp"
        "embeddedrtps/src/entities/StatelessReader.cpp"
        "embeddedrtps/src/entities/Reader.cpp"
        "embeddedrtps/src/entities/Writer.cpp"
        "embeddedrtps/src/utils/Lock.cpp"
        
        "embeddedrtps/src/messages/MessageReceiver.cpp"
        "embeddedrtps/src/messages/MessageTypes.cpp"
        "embeddedrtps/src/storages/PBufWrapper.cpp"
)

target_link_libraries(embeddedrtps PRIVATE lwip microcdr)

# List of include directories
set(EMBEDDEDRTPS_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/lwip"
    "${CMAKE_CURRENT_SOURCE_DIR}/embeddedrtps/include"
    "${microcdr_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/rtps"

)

# Iterate over each directory
foreach(include_dir ${EMBEDDEDRTPS_INCLUDE_DIRS})
  # Find all header files in the current directory
  file(GLOB_RECURSE EMBEDDEDRTPS_HEADER_FILES "${include_dir}/*.h")

  # Iterate over each header file
  foreach(header ${EMBEDDEDRTPS_HEADER_FILES})
    # Calculate the relative path of the header file
    file(RELATIVE_PATH relative_path ${include_dir} ${header})
    get_filename_component(relative_dir ${relative_path} DIRECTORY)

    # Create the corresponding directory in build/include/
    set(destination "${CMAKE_CURRENT_SOURCE_DIR}/../../build/include/${relative_dir}")
    file(MAKE_DIRECTORY ${destination})

    # Write a stub header that includes the original header
    get_filename_component(header_name ${header} NAME)
    file(WRITE ${destination}/${header_name}
      "#include \"${header}\"\n"
    )
  endforeach()
endforeach()

install(TARGETS embeddedrtps
        EXPORT embeddedrtpsTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT embeddedrtpsTargets
        FILE embeddedrtpsTargets.cmake
        NAMESPACE embeddedrtps::
        DESTINATION lib/cmake/embeddedrtps)

# Debug message for embeddedrtps target's INCLUDE_DIRECTORIES
#get_target_property(embeddedrtps_includes embeddedrtps INTERFACE_INCLUDE_DIRECTORIES)
#message(STATUS "INTERFACE_INCLUDE_DIRECTORIES for embeddedrtps: ${embeddedrtps_includes}")

# embeddedRTPS test
#add_subdirectory(tests)

# FastDDS
# add_subdirectory(fastdds)
