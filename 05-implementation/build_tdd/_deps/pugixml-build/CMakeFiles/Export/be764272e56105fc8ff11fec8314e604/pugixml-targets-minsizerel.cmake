#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "pugixml::shared" for configuration "MinSizeRel"
set_property(TARGET pugixml::shared APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(pugixml::shared PROPERTIES
  IMPORTED_IMPLIB_MINSIZEREL "${_IMPORT_PREFIX}/lib/pugixml.lib"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/bin/pugixml.dll"
  )

list(APPEND _cmake_import_check_targets pugixml::shared )
list(APPEND _cmake_import_check_files_for_pugixml::shared "${_IMPORT_PREFIX}/lib/pugixml.lib" "${_IMPORT_PREFIX}/bin/pugixml.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
