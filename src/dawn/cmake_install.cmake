# Install script for directory: /Users/artaleee/nir/dawn/src/dawn

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/homebrew/opt/llvm/bin/llvm-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/artaleee/nir/dawn/src/dawn/partition_alloc/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/common/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/platform/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/native/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/wire/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/utils/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/glfw/cmake_install.cmake")
  include("/Users/artaleee/nir/dawn/src/dawn/samples/cmake_install.cmake")

endif()

