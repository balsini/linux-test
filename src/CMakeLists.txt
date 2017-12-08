# Add include directories
include_directories(.)

# Create a library called "linux-test" which includes the source files.
add_library(${PROJECT_NAME} ${LIBRARY_TYPE} sched_setattr_interface.c)

#target_compile_features (${PROJECT_NAME} PRIVATE cxx_range_for)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set_property(TARGET ${PROJECT_NAME} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             ${PROJECT_SOURCE_DIR}/src)
# Export.
export(TARGETS ${PROJECT_NAME} FILE "./linuxTestConfig.cmake")
export(PACKAGE ${PROJECT_NAME})

# Define a macro to simplify tests creation
function(create_test name)
    add_executable(${name} ${ARGN})
    target_compile_features(${name} PRIVATE cxx_range_for)
    target_link_libraries(${name} ${PROJECT_NAME} Threads::Threads)
    add_test(NAME ${name} COMMAND ${name})
endfunction(create_test)

add_executable(sched_setattr_perf sched_setattr_perf.c)

target_link_libraries(sched_setattr_perf
    ${PROJECT_NAME}
    Threads::Threads
)

create_test(sched_setattr sched_setattr.c)
