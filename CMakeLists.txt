cmake_minimum_required(VERSION 3.28)
project(uMetadata VERSION 0.0.1 LANGUAGES CXX)
enable_testing()

option(BUILD_SHARED_LIBS "Build using shared libraries" ON) 
option(BUILD_FOR_DEBIAN "Build debian container for deployment" OFF)
option(BUILD_TESTS "Compile regression tests" ON)
include(GenerateExportHeader)
include(FetchContent)

set(Boost_USE_STATIC_LIBS ON)
if (NOT ${BUILD_SHARED_LIBS})
   set(ZLIB_USE_STATIC_LIBS ON)
endif()
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED)
find_package(ZLIB REQUIRED)
if (BUILD_TESTS)
   find_package(Catch2 3 REQUIRED)
endif()
configure_file(${CMAKE_SOURCE_DIR}/src/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/uMetadata/version.hpp)
set(LIBRARY_SRC proto/station.proto
    src/version.cpp
    src/client.cpp
    src/station.cpp
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
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(uMetadata PUBLIC gRPC::grpc gRPC::grpc++
                      PRIVATE protobuf::libprotobuf spdlog::spdlog_header_only)
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
add_executable(uMetadataServer
               src/main.cpp)
#               src/database.cpp)
set_target_properties(uMetadataServer PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uMetadataServer
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
target_link_libraries(uMetadataServer
                      PRIVATE uMetadata spdlog::spdlog_header_only SQLite::SQLite3
                              Boost::boost Threads::Threads)

##########################################################################################
#                                         Tests                                          #
##########################################################################################

add_executable(unitTests 
               testing/station.cpp
               testing/database.cpp)# src/database.cpp)
set_target_properties(unitTests PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_link_libraries(unitTests uMetadata
                      SQLite::SQLite3 Threads::Threads
                      Catch2::Catch2 Catch2::Catch2WithMain)
target_include_directories(unitTests
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
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
install(TARGETS uMetadataServer
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
if (${BUILD_FOR_DEBIAN})
   include(CPackComponent)
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/scripts/Dockerfile.deb.in
                  ${CMAKE_CURRENT_BINARY_DIR}/Dockerfile.deb @ONLY)
   add_custom_target(${PROJECT_NAME}-deb
                     COMMENT "Creating deb file for ${PROJECT_NAME}"
                     COMMAND ${CMAKE_CPACK_COMMAND} -G DEB 
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} VERBATIM)
   add_dependencies(${PROJECT_NAME}-deb uMetadataServer)
   find_program(buildah_EXECUTABLE buildah)
   if (buildah_EXECUTABLE)
      add_custom_target(buildah
                        COMMENT "Preparing the container with buildah"
                        COMMAND ${buildah_EXECUTABLE} bud -t umetadata:${PROJECT_VERSION} -t umetadata:latest -f ./Dockerfile.deb
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} VERBATIM)
      add_dependencies(buildah ${PROJECT_NAME}-deb)
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
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "UUSS Metadata microservice for the UUSS Kubernetes environment")
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
   if (${BUILD_FOR_DEBIAN})
      find_program(DPKG_PROGRAM dpkg REQUIRED)
   else()
      find_program(DPKG_PROGRAM dpkg)
   endif()
   if (DPKG_PROGRAM)
      list(APPEND CPACK_GENERATOR DEB)
      set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")# libsodium (>= 1.0.20) libzmq (>=4.3.0)")
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
