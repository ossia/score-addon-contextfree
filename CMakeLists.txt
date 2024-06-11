cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include(ScoreExternalAddon)
endif()

if(NOT TARGET score_plugin_avnd)
  return()
endif()

project(score_addon_contextfree LANGUAGES CXX)

add_library(contextfree STATIC
    3rdparty/context-free/src-agg/src/agg_bezier_arc.cpp
    3rdparty/context-free/src-agg/src/agg_color_rgba.cpp
    3rdparty/context-free/src-agg/src/agg_curves.cpp
    3rdparty/context-free/src-agg/src/agg_trans_affine.cpp
    3rdparty/context-free/src-agg/src/agg_vcgen_contour.cpp
    3rdparty/context-free/src-agg/src/agg_vcgen_stroke.cpp

    3rdparty/context-free/src-common/pathIterator.cpp

    3rdparty/context-free/src-common/abstractPngCanvas.cpp
    3rdparty/context-free/src-common/aggCanvas.cpp
    3rdparty/context-free/src-common/ast.cpp
    3rdparty/context-free/src-common/astexpression.cpp
    3rdparty/context-free/src-common/astreplacement.cpp
    3rdparty/context-free/src-common/bounds.cpp
    3rdparty/context-free/src-common/builder.cpp
    3rdparty/context-free/src-common/cfdg.cpp
    3rdparty/context-free/src-common/cfdgimpl.cpp
    3rdparty/context-free/src-common/CmdInfo.cpp
    3rdparty/context-free/src-common/HSBColor.cpp
    3rdparty/context-free/src-common/makeCFfilename.cpp
    3rdparty/context-free/src-common/prettyint.cpp
    3rdparty/context-free/src-common/primShape.cpp
    3rdparty/context-free/src-common/Rand64.cpp
    3rdparty/context-free/src-common/rendererAST.cpp
    3rdparty/context-free/src-common/renderimpl.cpp
    3rdparty/context-free/src-common/shape.cpp
    3rdparty/context-free/src-common/shapeSTL.cpp
    3rdparty/context-free/src-common/stacktype.cpp
    3rdparty/context-free/src-common/tempfile.cpp
    3rdparty/context-free/src-common/tiledCanvas.cpp
    3rdparty/context-free/src-common/variation.cpp

    3rdparty/context-free/src-win/derived/cfdg.tab.cpp
    3rdparty/context-free/src-win/derived/lex.yy.cpp
)

set_source_files_properties(
    3rdparty/context-free/src-win/derived/cfdg.tab.cpp
    3rdparty/context-free/src-win/derived/lex.yy.cpp
  PROPERTIES
    LANGUAGE CXX
    SKIP_UNITY_BUILD_INCLUSION 1
)

if(MSVC)
else()
  target_compile_options(contextfree PRIVATE -fno-char8_t)
endif()

if(WIN32)
    target_link_libraries(contextfree PRIVATE Shlwapi)
endif()
target_include_directories(contextfree
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-common>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-common/agg-extras>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-agg>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-agg/agg2>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-win/derived>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/context-free/src-unix>
)

avnd_score_plugin_init(
  BASE_TARGET score_addon_contextfree
)

avnd_score_plugin_add(
  BASE_TARGET score_addon_contextfree
  SOURCES
    ContextFree/ContextFreeModel.hpp
    ContextFree/ContextFreeModel.cpp
    ContextFree/ContextFreeLoad.cpp
    ContextFree/ContextFreeLoad.hpp
  TARGET contextfree
  MAIN_CLASS ContextFree
  NAMESPACE vo
)
target_link_libraries(score_addon_contextfree PRIVATE contextfree)

avnd_score_plugin_finalize(
  BASE_TARGET score_addon_contextfree
  PLUGIN_VERSION 1
  PLUGIN_UUID "e3b22418-ea16-48e5-8f0b-b00aa49f4a81"
)
