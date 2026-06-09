cmake_minimum_required(VERSION 3.28)
project(uMetadata VERSION 0.0.2 LANGUAGES CXX)
enable_testing()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON) 
option(BUILD_CONTAINER "Build Ubuntu container for deployment" OFF)
option(BUILD_TESTS "Compile regression tests" ON)
option(WITH_CONAN "Build using Conan" OFF)
option(USE_CLANG_TIDY "Build using clang-tidy" OFF)
include(GenerateExportHeader)
include(FetchContent)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
if (NOT BUILD_SHARED_LIBS)
   message("Will attempt to use Boost static libraries")
   set(Boost_USE_STATIC_LIBS ON) 
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(Boost_USE_STATIC_LIBS ON)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)
if(NOT TARGET SQLite3::SQLite3) # CMake < 4.3
  add_library(SQLite3::SQLite3 ALIAS SQLite::SQLite3)
endif()
find_package(Boost REQUIRED program_options REQUIRED)
if (${WITH_CONAN})
   message("Will use Conan OTLP exporters")
   find_package(opentelemetry-cpp CONFIG COMPONENTS exporter_otlp_grpc exporters_otlp_http REQUIRED)
else()
   find_package(opentelemetry-cpp CONFIG COMPONENTS exporters_otlp_http REQUIRED)
endif()
find_package(Threads REQUIRED)
if (BUILD_TESTS)
   find_package(Catch2 3 REQUIRED)
endif()

if (${USE_CLANG_TIDY} AND PROJECT_IS_TOP_LEVEL)
   message("Looking for clang-tidy")
   find_program(CLANG_TIDY_EXECUTABLE NAMES "clang-tidy")
   if (CLANG_TIDY_EXECUTABLE)
      message("Using clang-tidy  ${CLANG_TIDY_EXECUTABLE}")
      set(CMAKE_C_CLANG_TIDY   ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR})
      set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR})
      #set(CLANG_TIDY_COMMAND ${CLANG_TIDY_EXECUTABLE} "-config=${CMAKE_SOURCE_DIR}/.clang-tidy")
      set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
   else()
      message("clang-tidy not found")
   endif()
endif()

find_package(Git)
if (Git_FOUND)
   execute_process(
     COMMAND ${GIT_EXECUTABLE} --no-pager log -n 1 HEAD "--pretty=format:%h"
     OUTPUT_VARIABLE uMetadata_GITTAG
     RESULT_VARIABLE gitresult
     ERROR_QUIET
     OUTPUT_STRIP_TRAILING_WHITESPACE
   )
   if (gitresult EQUAL 0)
      message(STATUS "Last Git commit: ${uMetadata_GITTAG}")
   endif()
else()
   set(uMetadata_GITTAG "")
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/uMetadata/version.hpp)
set(LIBRARY_SRC
    uMetadataAPI/v1/station.proto
    uMetadataAPI/v1/channel.proto
    uMetadataAPI/v1/station_information_service.proto
    uMetadataAPI/v1/all_active_stations_request.proto
    uMetadataAPI/v1/active_station_request.proto
    uMetadataAPI/v1/stations_response.proto
    #uMetadataAPI/v1/telemetry.proto
    src/version.cpp
    src/client.cpp
    src/station.cpp
    src/channel.cpp
    src/database.cpp)
if (BUILD_SHARED_LIBS)
   add_library(uMetadata SHARED ${LIBRARY_SRC})
else()
   add_library(uMetadata STATIC ${LIBRARY_SRC})
endif()
target_sources(uMetadata
               PUBLIC
               FILE_SET HEADERS
               BASE_DIRS
                  ${CMAKE_CURRENT_SOURCE_DIR}
               FILES 
                  include/uMetadata/version.hpp
                  include/uMetadata/station.hpp
                  include/uMetadata/client.hpp
               )
set_target_properties(uMetadata PROPERTIES
                      CXX_STANDARD 23
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(uMetadata
                      PUBLIC gRPC::grpc gRPC::grpc++
                      PRIVATE SQLite::SQLite3 protobuf::libprotobuf spdlog::spdlog_header_only)
target_include_directories(uMetadata
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
protobuf_generate(TARGET uMetadata
                  LANGUAGE cpp)
protobuf_generate(TARGET uMetadata
                  LANGUAGE grpc
                  PLUGIN "protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
                  PLUGIN_OPTIONS generate_mock_code=true
                  GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc)
add_executable(uMetadataServer src/server.cpp)
set_target_properties(uMetadataServer PROPERTIES
                      CXX_STANDARD 23
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uMetadataServer
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_link_libraries(uMetadataServer
                      PRIVATE gRPC::grpc gRPC::grpc++ gRPC::grpc++_reflection
                              uMetadata spdlog::spdlog_header_only SQLite::SQLite3
                              Boost::boost Boost::program_options Threads::Threads)
if (${WITH_CONAN})
   target_link_libraries(uMetadataServer
                         PRIVATE opentelemetry-cpp::opentelemetry-cpp
                                 opentelemetry-cpp::proto_grpc
                                 opentelemetry-cpp::exporter_otlp_grpc
                                 opentelemetry-cpp::exporter_otlp_grpc_client
                                 opentelemetry-cpp::exporter_otlp_grpc_metrics
                                 opentelemetry-cpp::exporter_otlp_http
                                 opentelemetry-cpp::exporter_otlp_http_client
                                 opentelemetry-cpp::exporter_otlp_http_metric
                                 opentelemetry-cpp::exporter_otlp_http_log
                                 opentelemetry-cpp::metrics
                                 opentelemetry-cpp::common
                                 opentelemetry-cpp::api)
   target_compile_definitions(uMetadataServer PRIVATE WITH_OTLP_GRPC)
else()
   target_link_libraries(uMetadataServer
                         PRIVATE opentelemetry-cpp::otlp_http_log_record_exporter
                                 opentelemetry-cpp::otlp_http_metric_exporter)
endif()


add_executable(uMetadataUpdater src/updater.cpp)
set_target_properties(uMetadataUpdater PROPERTIES
                      CXX_STANDARD 23
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_include_directories(uMetadataUpdater
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_link_libraries(uMetadataUpdater
                      PRIVATE uMetadata spdlog::spdlog_header_only SQLite::SQLite3
                              Boost::boost Boost::program_options Threads::Threads)


##########################################################################################
#                                         Tests                                          #
##########################################################################################

add_executable(unitTests 
               testing/station.cpp
               testing/channel.cpp
               testing/database.cpp)
set_target_properties(unitTests PROPERTIES
                      CXX_STANDARD 23
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_link_libraries(unitTests uMetadata
                      SQLite::SQLite3 Threads::Threads SQLite::SQLite3
                      Catch2::Catch2 Catch2::Catch2WithMain)
target_include_directories(unitTests
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/data>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>)
add_test(NAME unitTests 
         COMMAND unitTests)


##########################################################################################
#                                      Installation                                      #
##########################################################################################
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS uMetadata
        EXPORT ${PROJECT_NAME}-targets
        FILE_SET HEADERS
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libraries)
install(TARGETS uMetadataServer uMetadataUpdater
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT applications)
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

##########################################################################################
#                                        Containers                                      #
##########################################################################################
if (${BUILD_CONTAINER})
   include(CPackComponent)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/Dockerfile.u24.in
                  ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile.u24 @ONLY)
   add_custom_target(${PROJECT_NAME}-u24
                     COMMENT "Creating Dockerfile for ${PROJECT_NAME}"
                     COMMAND ${CMAKE_CPACK_COMMAND} -G DEB 
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} VERBATIM)
   add_dependencies(${PROJECT_NAME}-u24 uMetadataServer uMetadataUpdater)
   find_program(buildah_EXECUTABLE buildah)
   if (buildah_EXECUTABLE)
      add_custom_target(buildah
                        COMMENT "Preparing the container with buildah"
                        COMMAND ${buildah_EXECUTABLE} bud -t umetadata:${PROJECT_VERSION} -t umetadata:latest -f ./Dockerfile.u24
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} VERBATIM)
      add_dependencies(buildah ${PROJECT_NAME}-u24)
   endif()
endif()

##########################################################################################
#                                     CPACK Packaging                                    #
##########################################################################################
include(CPackComponent)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "UUSS")
set(CPACK_PACKAGE_CONTACT "ben.baker@utah.edu")
set(CPACK_PACKAGE_LICENSE "MIT")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "UUSS metadata microservice for the UUSS Kubernetes environment.")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
if (${BUILD_SHARED_LIBS})
   cpack_add_component(libraries
                       DISPLAY_NAME "uMetadata common libraries"
                       DEPENDS ${GRPC_LIBRARY} ${GRPC_GRPC++_LIBRARY} ${SQLite3_LIBRARIES})
else()
   cpack_add_component(libraries
                       DISPLAY_NAME "uMetadata common libraries")
endif()
cpack_add_component(headers
                    DISPLAY_NAME "uMetadata library header files"
                    DEPENDS libraries)
cpack_add_component(applications
                    DISPLAY_NAME "uMetadata applications"
                    DEPENDS libraries)
set(CPACK_GENERATOR ZIP TGZ)
if (WIN32)
   list(APPEND CPACK_GENERATOR WIX)
elseif (APPLE)
   list(APPEND CPACK_GENERATOR productbuild)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   find_program(RPMBUILD_PROGRAM rpmbuild)
   if (RPMBUILD_PROGRAM)
      list(APPEND CPACK_GENERATOR RPM)
   endif()
   if (${BUILD_CONTAINER})
      find_program(DPKG_PROGRAM dpkg REQUIRED)
   else()
      find_program(DPKG_PROGRAM dpkg)
   endif()
   if (DPKG_PROGRAM)
      list(APPEND CPACK_GENERATOR DEB)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
      set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) 
   endif()
else()
   message("Unknown operating system")
endif()
set(CPACK_SOURCE_IGNORE_FILES
  /\\.git/
  \\.swp
  \\.orig
  /CMakeLists\\.txt\\.user
  /private/
)
include(CPack) # Put this last!
