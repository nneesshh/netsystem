# Install script for directory: D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/libevent")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Debug/event.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Release/event.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/include/event2/buffer.h;C:/Program Files (x86)/libevent/include/event2/bufferevent.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_compat.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_struct.h;C:/Program Files (x86)/libevent/include/event2/buffer_compat.h;C:/Program Files (x86)/libevent/include/event2/dns.h;C:/Program Files (x86)/libevent/include/event2/dns_compat.h;C:/Program Files (x86)/libevent/include/event2/dns_struct.h;C:/Program Files (x86)/libevent/include/event2/event.h;C:/Program Files (x86)/libevent/include/event2/event_compat.h;C:/Program Files (x86)/libevent/include/event2/event_struct.h;C:/Program Files (x86)/libevent/include/event2/http.h;C:/Program Files (x86)/libevent/include/event2/http_compat.h;C:/Program Files (x86)/libevent/include/event2/http_struct.h;C:/Program Files (x86)/libevent/include/event2/keyvalq_struct.h;C:/Program Files (x86)/libevent/include/event2/listener.h;C:/Program Files (x86)/libevent/include/event2/rpc.h;C:/Program Files (x86)/libevent/include/event2/rpc_compat.h;C:/Program Files (x86)/libevent/include/event2/rpc_struct.h;C:/Program Files (x86)/libevent/include/event2/tag.h;C:/Program Files (x86)/libevent/include/event2/tag_compat.h;C:/Program Files (x86)/libevent/include/event2/thread.h;C:/Program Files (x86)/libevent/include/event2/util.h;C:/Program Files (x86)/libevent/include/event2/visibility.h;C:/Program Files (x86)/libevent/include/event2/event-config.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/include/event2" TYPE FILE FILES
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/keyvalq_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/listener.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/thread.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/util.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/visibility.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/include/event2/event-config.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event_core.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Debug/event_core.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event_core.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Release/event_core.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/include/event2/buffer.h;C:/Program Files (x86)/libevent/include/event2/bufferevent.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_compat.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_struct.h;C:/Program Files (x86)/libevent/include/event2/buffer_compat.h;C:/Program Files (x86)/libevent/include/event2/dns.h;C:/Program Files (x86)/libevent/include/event2/dns_compat.h;C:/Program Files (x86)/libevent/include/event2/dns_struct.h;C:/Program Files (x86)/libevent/include/event2/event.h;C:/Program Files (x86)/libevent/include/event2/event_compat.h;C:/Program Files (x86)/libevent/include/event2/event_struct.h;C:/Program Files (x86)/libevent/include/event2/http.h;C:/Program Files (x86)/libevent/include/event2/http_compat.h;C:/Program Files (x86)/libevent/include/event2/http_struct.h;C:/Program Files (x86)/libevent/include/event2/keyvalq_struct.h;C:/Program Files (x86)/libevent/include/event2/listener.h;C:/Program Files (x86)/libevent/include/event2/rpc.h;C:/Program Files (x86)/libevent/include/event2/rpc_compat.h;C:/Program Files (x86)/libevent/include/event2/rpc_struct.h;C:/Program Files (x86)/libevent/include/event2/tag.h;C:/Program Files (x86)/libevent/include/event2/tag_compat.h;C:/Program Files (x86)/libevent/include/event2/thread.h;C:/Program Files (x86)/libevent/include/event2/util.h;C:/Program Files (x86)/libevent/include/event2/visibility.h;C:/Program Files (x86)/libevent/include/event2/event-config.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/include/event2" TYPE FILE FILES
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/keyvalq_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/listener.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/thread.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/util.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/visibility.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/include/event2/event-config.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event_extra.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Debug/event_extra.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/lib/event_extra.lib")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/lib" TYPE STATIC_LIBRARY FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/lib/Release/event_extra.lib")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/include/event2/buffer.h;C:/Program Files (x86)/libevent/include/event2/bufferevent.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_compat.h;C:/Program Files (x86)/libevent/include/event2/bufferevent_struct.h;C:/Program Files (x86)/libevent/include/event2/buffer_compat.h;C:/Program Files (x86)/libevent/include/event2/dns.h;C:/Program Files (x86)/libevent/include/event2/dns_compat.h;C:/Program Files (x86)/libevent/include/event2/dns_struct.h;C:/Program Files (x86)/libevent/include/event2/event.h;C:/Program Files (x86)/libevent/include/event2/event_compat.h;C:/Program Files (x86)/libevent/include/event2/event_struct.h;C:/Program Files (x86)/libevent/include/event2/http.h;C:/Program Files (x86)/libevent/include/event2/http_compat.h;C:/Program Files (x86)/libevent/include/event2/http_struct.h;C:/Program Files (x86)/libevent/include/event2/keyvalq_struct.h;C:/Program Files (x86)/libevent/include/event2/listener.h;C:/Program Files (x86)/libevent/include/event2/rpc.h;C:/Program Files (x86)/libevent/include/event2/rpc_compat.h;C:/Program Files (x86)/libevent/include/event2/rpc_struct.h;C:/Program Files (x86)/libevent/include/event2/tag.h;C:/Program Files (x86)/libevent/include/event2/tag_compat.h;C:/Program Files (x86)/libevent/include/event2/thread.h;C:/Program Files (x86)/libevent/include/event2/util.h;C:/Program Files (x86)/libevent/include/event2/visibility.h;C:/Program Files (x86)/libevent/include/event2/event-config.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/include/event2" TYPE FILE FILES
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/bufferevent_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/buffer_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/dns_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/event_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/http_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/keyvalq_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/listener.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/rpc_struct.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/tag_compat.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/thread.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/util.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event2/visibility.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/include/event2/event-config.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/include/evdns.h;C:/Program Files (x86)/libevent/include/evrpc.h;C:/Program Files (x86)/libevent/include/event.h;C:/Program Files (x86)/libevent/include/evhttp.h;C:/Program Files (x86)/libevent/include/evutil.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/include" TYPE FILE FILES
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/evdns.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/evrpc.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/event.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/evhttp.h"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/include/evutil.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/cmake/LibeventConfig.cmake;C:/Program Files (x86)/libevent/cmake/LibeventConfigVersion.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/cmake" TYPE FILE FILES
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build//CMakeFiles/LibeventConfig.cmake"
    "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/LibeventConfigVersion.cmake"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}C:/Program Files (x86)/libevent/cmake/LibeventTargets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}C:/Program Files (x86)/libevent/cmake/LibeventTargets.cmake"
         "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/CMakeFiles/Export/C_/Program_Files_(x86)/libevent/cmake/LibeventTargets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}C:/Program Files (x86)/libevent/cmake/LibeventTargets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}C:/Program Files (x86)/libevent/cmake/LibeventTargets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "C:/Program Files (x86)/libevent/cmake/LibeventTargets.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/cmake" TYPE FILE FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/CMakeFiles/Export/C_/Program_Files_(x86)/libevent/cmake/LibeventTargets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/cmake/LibeventTargets-debug.cmake")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/cmake" TYPE FILE FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/CMakeFiles/Export/C_/Program_Files_(x86)/libevent/cmake/LibeventTargets-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files (x86)/libevent/cmake/LibeventTargets-release.cmake")
    if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
    if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    endif()
file(INSTALL DESTINATION "C:/Program Files (x86)/libevent/cmake" TYPE FILE FILES "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/CMakeFiles/Export/C_/Program_Files_(x86)/libevent/cmake/LibeventTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/dd/sdk32/support/libevent/libevent-release-2.1.8-stable/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
