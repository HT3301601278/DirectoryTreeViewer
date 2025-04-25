# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\DirectoryTreeViewer_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\DirectoryTreeViewer_autogen.dir\\ParseCache.txt"
  "DirectoryTreeViewer_autogen"
  )
endif()
