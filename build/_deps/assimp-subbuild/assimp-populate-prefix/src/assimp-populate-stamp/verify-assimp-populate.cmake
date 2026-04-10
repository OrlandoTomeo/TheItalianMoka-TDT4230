# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

if("C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip")
  message(FATAL_ERROR "File not found: C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip")
endif()

if("" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip'")

file("" "C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  C:/Users/orlan/Desktop/TheItalianMoka-TDT4230/external/assimp-5.0.1.zip
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
