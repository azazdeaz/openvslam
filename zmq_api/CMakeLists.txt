


# Protobuf
find_package(Protobuf REQUIRED)
if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Could not find protoc executable (PROTOBUF_PROTOC_EXECUTABLE)")
endif()
message(STATUS "Found protoc executable: ${PROTOBUF_PROTOC_EXECUTABLE}")

# ----- Protobuf transpile -----

protobuf_generate_cpp(API_PB_SOURCE API_PB_HEADER proto/openvslam_api.proto)

if(NOT MSVC)
    set_source_files_properties(${API_PB_HEADER} ${API_PB_SOURCE}
                                COMPILE_FLAGS -Wno-unused-variable)
endif()


# ----- Find dependencies -----
set(cppzmq_DIR ${PROJECT_SOURCE_DIR}/dependencies)
#find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq)

# popl
set(popl_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rd/popl/include)

set(USE_STACK_TRACE_LOGGER OFF CACHE BOOL "Enable automatic stack trace logger of google-glog")
if(USE_STACK_TRACE_LOGGER)
    find_package(Glog REQUIRED)
    include_directories(${GLOG_INCLUDE_DIR})
    message(STATUS "Stack trace logger: ENABLED")
else()
    message(STATUS "Stack trace logger: DISABLED")
endif()

set(USE_GOOGLE_PERFTOOLS OFF CACHE BOOL "Enable profiler of google-perftools")
if(USE_GOOGLE_PERFTOOLS)
    find_package(Gperftools REQUIRED)
    include_directories(${GPERFTOOLS_INCLUDE_DIRS})
    message(STATUS "Google Perftools: ENABLED")
else()
    message(STATUS "Google Perftools: DISABLED")
endif()

# ----- Build example executables -----

set(EXECUTABLE_TARGETS "")

add_executable(run_api run_api.cc ${API_PB_SOURCE})
list(APPEND EXECUTABLE_TARGETS run_api)

foreach(EXECUTABLE_TARGET IN LISTS EXECUTABLE_TARGETS)
    # Set output directory for executables
    set_target_properties(${EXECUTABLE_TARGET} PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
                          RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}"
                          RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}"
                          RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${PROJECT_BINARY_DIR}"
                          RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}")

    # Setup stack trace logger
    if(USE_STACK_TRACE_LOGGER)
        target_compile_definitions(${EXECUTABLE_TARGET} PRIVATE USE_STACK_TRACE_LOGGER)
        target_link_libraries(${EXECUTABLE_TARGET} PRIVATE ${GLOG_LIBRARIES})
    endif()

    # Setup google-perftools
    if(USE_GOOGLE_PERFTOOLS)
        target_compile_definitions(${EXECUTABLE_TARGET} PRIVATE USE_GOOGLE_PERFTOOLS)
        target_link_libraries(${EXECUTABLE_TARGET} PRIVATE ${GPERFTOOLS_LIBRARIES})
    endif()


    # Link OpenVSLAM
    target_link_libraries(${EXECUTABLE_TARGET}
                          PRIVATE
                          ${PROJECT_NAME}
                          cppzmq
                          ${PROTOBUF_LIBRARIES}
                          opencv_imgcodecs
                          opencv_videoio)

    # include popl and spdlog headers
    target_include_directories(${EXECUTABLE_TARGET}
                               PUBLIC
                               ${PROTOBUF_INCLUDE_DIRS}
                               PRIVATE
                               $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/3rd/popl/include>
                               $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/3rd/spdlog/include>
                               ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
