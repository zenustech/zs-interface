cmake_minimum_required(VERSION 3.18)
project(OpenVDBDocs LANGUAGES NONE)

find_package(Doxygen REQUIRED doxygen)

set(DOXY_FILES
  projects/core/interface
  projects/core/interface/details
  projects/core/interface/world
  projects/core/interface/world/value_type

  doc/doc.md
  doc/build.md
  doc/design.md
  doc/example.md
  doc/faq.md
  doc/codingstyle.md
  doc/changes.md
  )

# list(APPEND DOXY_FILES
#   
# )

set(DOXYGEN_OUTPUT_LANGUAGE Chinese)

set(DOXYGEN_PROJECT_LOGO doc/img/zensim_logo.png)
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE doc/doc.md)
set(DOXYGEN_MACRO_EXPANSION ON)
set(DOXYGEN_PREDEFINED __cplusplus)

set(DOXYGEN_PROJECT_NAME "ZsInterface")
set(DOXYGEN_PROJECT_NUMBER "${project_version}")
set(DOXYGEN_PROJECT_BRIEF "")
set(DOXYGEN_FILE_PATTERNS *.h *h.in *.hpp) # headers only
set(DOXYGEN_EXTENSION_MAPPING .in=C) # parse CMake config headers as C
set(DOXYGEN_IMAGE_PATH "doc/img")
set(DOXYGEN_RECURSIVE NO)
set(DOXYGEN_QUIET YES)
set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/doc)

set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_GENERATE_TODOLIST NO)

# matrix-like theme
set(DOXYGEN_HTML_COLORSTYLE_HUE 120)
set(DOXYGEN_HTML_COLORSTYLE_SAT 233)
set(DOXYGEN_HTML_TIMESTAMP NO) # for simpler website diffs

set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_EXTRACT_LOCAL_CLASSES NO)

# Allow for root directory paths to be listed (e.g. openvdb/io/File.h)
# Rather than just the file names (e.g. File.h). This makes it far
# easier to navigate the docs
set(DOXYGEN_FULL_PATH_NAMES YES)
#[=[
set(DOXYGEN_STRIP_FROM_PATH
  "${CMAKE_SOURCE_DIR}/openvdb"
  "${CMAKE_SOURCE_DIR}/openvdb_ax"
  "${CMAKE_SOURCE_DIR}/nanovdb"
  "${CMAKE_SOURCE_DIR}/openvdb_houdini"
  "${CMAKE_SOURCE_DIR}")
set(DOXYGEN_STRIP_FROM_INC_PATH
  "${CMAKE_SOURCE_DIR}/openvdb"
  "${CMAKE_SOURCE_DIR}/openvdb_ax"
  "${CMAKE_SOURCE_DIR}/nanovdb"
  "${CMAKE_SOURCE_DIR}/openvdb_houdini")
]=]
# Shows which source files generated the respected doxygen docs
# at the bottom of each html page. Don't bother listing these.
set(DOXYGEN_SHOW_USED_FILES NO)
# When doxygen adds the source code for each header (see VERBATIM_HEADERS)
# don't remove the actual doxygen comments
set(DOXYGEN_STRIP_CODE_COMMENTS NO)
# Don't add an alpha index of all classes (it's not particularly useful)
set(DOXYGEN_ALPHABETICAL_INDEX NO)
set(DOXYGEN_DISTRIBUTE_GROUP_DOC YES)
set(DOXYGEN_HIDE_IN_BODY_DOCS YES)
set(DOXYGEN_HIDE_SCOPE_NAMES YES)
set(DOXYGEN_INLINE_INHERITED_MEMB YES)
set(DOXYGEN_MACRO_EXPANSION YES)
# @todo use dot with class diagrams for relevant classes
set(DOXYGEN_CLASS_DIAGRAMS NO)

set(DOXYGEN_VERBATIM_VARS DOXYGEN_ALIASES DOXYGEN_PREDEFINED)

doxygen_add_docs(zs_interface_doc ${DOXY_FILES}
  ALL WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Generating API documentation with Doxygen")