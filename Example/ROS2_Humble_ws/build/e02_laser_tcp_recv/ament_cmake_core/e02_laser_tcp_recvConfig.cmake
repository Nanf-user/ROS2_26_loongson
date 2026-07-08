# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_e02_laser_tcp_recv_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED e02_laser_tcp_recv_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(e02_laser_tcp_recv_FOUND FALSE)
  elseif(NOT e02_laser_tcp_recv_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(e02_laser_tcp_recv_FOUND FALSE)
  endif()
  return()
endif()
set(_e02_laser_tcp_recv_CONFIG_INCLUDED TRUE)

# output package information
if(NOT e02_laser_tcp_recv_FIND_QUIETLY)
  message(STATUS "Found e02_laser_tcp_recv: 3.0.0 (${e02_laser_tcp_recv_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'e02_laser_tcp_recv' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${e02_laser_tcp_recv_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(e02_laser_tcp_recv_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${e02_laser_tcp_recv_DIR}/${_extra}")
endforeach()
