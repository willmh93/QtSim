# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/QtSim_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/QtSim_autogen.dir/ParseCache.txt"
  "QtSim_autogen"
  )
endif()
