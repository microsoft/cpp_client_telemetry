# Based on group_source_files_by_folder() from dev_buildtools/cmake/Modules/VisualStudioUtils.cmake (2015-02-19)
function(create_source_files_groups_per_folder)
  set(dirs "")
  foreach(src ${ARGV})
    get_filename_component(dir ${src} DIRECTORY)
    list(APPEND dirs ${dir})
  endforeach()
  list(REMOVE_DUPLICATES dirs)

  # Default source group to apply for files outside of any subdirectory.
  # One space " " instead of an empty name is a widespread trick to make it
  # actually appear outside of any folder in Visual Studio.
  source_group(" " REGULAR_EXPRESSION "")

  foreach(dir ${dirs})
    string(REPLACE "/" "\\\\" folder ${dir})
    # [-^ seems to be the only way to include ], escaping did not work.
    string(REGEX REPLACE "[.[-^$*+?|()]" "\\\\\\0" pattern ${dir})
    source_group(${folder} REGULAR_EXPRESSION "/${pattern}/[^/]+$")
  endforeach()
endfunction()
