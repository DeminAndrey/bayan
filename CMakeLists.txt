cmake_minimum_required(VERSION 3.12)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(bayan VERSION ${PROJECT_VESRION})

configure_file(version.h.in version.h)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

find_package(Boost
  COMPONENTS
  filesystem
  regex
  program_options)

add_executable(bayan
  FileScanner.h
  FileScanner.cxx
  main.cxx)
  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -pedantic")
set_target_properties(${PROJECT_NAME} PROPERTIES
                CXX_STANDARD 17
                CXX_STANDARD_REQUIRED ON
                COMPILE_OPTIONS "-O0;-Wall;"
)

target_include_directories(bayan
  PRIVATE "${CMAKE_BINARY_DIR}")

target_include_directories(bayan
  PRIVATE ${Boost_INCLUDE_DIR})

target_link_libraries(bayan
  PRIVATE
  Boost::filesystem
  Boost::regex
  Boost::program_options)
  
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
