find_path(SQLite3_INCLUDE_DIR sqlite3.h)
find_library(SQLite3_LIBRARY NAMES sqlite3)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3
  FOUND_VAR SQLite3_FOUND
  REQUIRED_VARS SQLite3_LIBRARY SQLite3_INCLUDE_DIR
)

if(SQLite3_FOUND AND NOT TARGET SQLite3::SQLite3)
  add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
  set_target_properties(SQLite3::SQLite3 PROPERTIES
    IMPORTED_LOCATION "${SQLite3_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}"
  )
endif()
