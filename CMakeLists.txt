cmake_minimum_required(VERSION 3.24)
project(LayoutShop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(LAYOUTSHOP_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LAYOUTSHOP_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(APPLE)
    # Homebrew QtGui links against AGL.framework on some macOS setups.
    # Newer SDK sysroots may not expose AGL, but the framework still exists
    # in /System/Library/Frameworks, so add that directory explicitly.
    add_link_options(-F/System/Library/Frameworks)
endif()


find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        PrintSupport
        SvgWidgets
        REQUIRED)
find_package(Eigen3 REQUIRED)
find_path(RAPIDJSON_INCLUDE_DIR NAMES rapidjson/document.h REQUIRED)
find_package(Torch REQUIRED)

function(remove_agl_entries input_list output_var)
    set(_filtered "")
    set(_pending_framework FALSE)
    foreach(_item IN LISTS ${input_list})
        if(_pending_framework)
            if(_item STREQUAL "AGL")
                list(POP_BACK _filtered)
            else()
                list(APPEND _filtered "${_item}")
            endif()
            set(_pending_framework FALSE)
        elseif(_item STREQUAL "-framework")
            list(APPEND _filtered "${_item}")
            set(_pending_framework TRUE)
        elseif(_item STREQUAL "-framework AGL")
            continue()
        else()
            list(APPEND _filtered "${_item}")
        endif()
    endforeach()
    set(${output_var} "${_filtered}" PARENT_SCOPE)
endfunction()

# Some libtorch/Caffe2 builds on macOS still inject the legacy AGL framework.
# Recent macOS SDKs no longer ship it, so strip it from both variables and
# imported target interfaces before linking the app.
remove_agl_entries(TORCH_LIBRARIES TORCH_LIBRARIES)

if(TARGET OpenGL::GL)
    get_target_property(_opengl_link_libs OpenGL::GL INTERFACE_LINK_LIBRARIES)
    if(_opengl_link_libs)
        remove_agl_entries(_opengl_link_libs _opengl_link_libs_filtered)
        set_target_properties(OpenGL::GL PROPERTIES
                INTERFACE_LINK_LIBRARIES "${_opengl_link_libs_filtered}")
    endif()
endif()

if(TARGET torch)
    get_target_property(_torch_interface_libs torch INTERFACE_LINK_LIBRARIES)
    if(_torch_interface_libs)
        remove_agl_entries(_torch_interface_libs _torch_interface_libs_filtered)
        set_target_properties(torch PROPERTIES
                INTERFACE_LINK_LIBRARIES "${_torch_interface_libs_filtered}")
    endif()
endif()

if(TARGET torch_library)
    get_target_property(_torch_library_interface_libs torch_library INTERFACE_LINK_LIBRARIES)
    if(_torch_library_interface_libs)
        remove_agl_entries(_torch_library_interface_libs _torch_library_interface_libs_filtered)
        set_target_properties(torch_library PROPERTIES
                INTERFACE_LINK_LIBRARIES "${_torch_library_interface_libs_filtered}")
    endif()
endif()

set(GUROBI_HOME "" CACHE PATH "Path to a Gurobi installation, for example /Library/gurobi1001/macos_universal2")
find_path(GUROBI_INCLUDE_DIR
        NAMES gurobi_c++.h
        HINTS
            "${GUROBI_HOME}"
            "$ENV{GUROBI_HOME}"
            "/Library/gurobi1001/macos_universal2"
        PATH_SUFFIXES include)
find_library(GUROBI_CXX_LIBRARY
        NAMES gurobi_c++
        HINTS
            "${GUROBI_HOME}"
            "$ENV{GUROBI_HOME}"
            "/Library/gurobi1001/macos_universal2"
        PATH_SUFFIXES lib)
find_library(GUROBI_LIBRARY
        NAMES gurobi gurobi120 gurobi110 gurobi100
        HINTS
            "${GUROBI_HOME}"
            "$ENV{GUROBI_HOME}"
            "/Library/gurobi1001/macos_universal2"
        PATH_SUFFIXES lib)
if(NOT GUROBI_INCLUDE_DIR OR NOT GUROBI_CXX_LIBRARY OR NOT GUROBI_LIBRARY)
    message(FATAL_ERROR "Gurobi was not found. Install Gurobi and configure with -DGUROBI_HOME=/path/to/gurobi.")
endif()

add_executable(LayoutShop
        ${LAYOUTSHOP_SRC_DIR}/LayoutShop.ui
        ${LAYOUTSHOP_SRC_DIR}/TemplateDialog.ui
        ${LAYOUTSHOP_SRC_DIR}/RefineWidget.ui
        ${LAYOUTSHOP_SRC_DIR}/LayoutShop.qrc
        ${LAYOUTSHOP_SRC_DIR}/CNodeMatch.cpp
        ${LAYOUTSHOP_SRC_DIR}/CCombineTreeProcessor.cpp
        ${LAYOUTSHOP_SRC_DIR}/Article.cpp
        ${LAYOUTSHOP_SRC_DIR}/CHungarianAlgorithm.h
        ${LAYOUTSHOP_SRC_DIR}/LayoutShop.cpp
        ${LAYOUTSHOP_SRC_DIR}/CCombineTreeNode.h
        ${LAYOUTSHOP_SRC_DIR}/LayoutShop.h
        ${LAYOUTSHOP_SRC_DIR}/CNodeMatch.h
        ${LAYOUTSHOP_SRC_DIR}/Article.h
        ${LAYOUTSHOP_SRC_DIR}/CGroupTreeNode.h
        ${LAYOUTSHOP_SRC_DIR}/CLayoutTree.cpp
        ${LAYOUTSHOP_SRC_DIR}/CDelauntor.h
        ${LAYOUTSHOP_SRC_DIR}/CLayoutTree.h
        ${LAYOUTSHOP_SRC_DIR}/CCombineTreeNode.cpp
        ${LAYOUTSHOP_SRC_DIR}/CMds.h
        ${LAYOUTSHOP_SRC_DIR}/CCombineTreeProcessor.h
        ${LAYOUTSHOP_SRC_DIR}/CombineTreeHandler.cpp
        ${LAYOUTSHOP_SRC_DIR}/CHungarianAlgorithm.cpp
        ${LAYOUTSHOP_SRC_DIR}/CNode.h
        ${LAYOUTSHOP_SRC_DIR}/CombineTreeHandler.h
        ${LAYOUTSHOP_SRC_DIR}/InputWidget.cpp
        ${LAYOUTSHOP_SRC_DIR}/InputWidget.h
        ${LAYOUTSHOP_SRC_DIR}/main.cpp
        ${LAYOUTSHOP_SRC_DIR}/OutputView.cpp
        ${LAYOUTSHOP_SRC_DIR}/OutputView.h
        ${LAYOUTSHOP_SRC_DIR}/LayoutGeneratorMultiPage.cpp
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsItem.cpp
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsScene.cpp
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsTextItem.cpp
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsView.cpp
        ${LAYOUTSHOP_SRC_DIR}/LayoutEstimator.cpp
        ${LAYOUTSHOP_SRC_DIR}/LayoutPainter.cpp
        ${LAYOUTSHOP_SRC_DIR}/resource.h
        ${LAYOUTSHOP_SRC_DIR}/ResultPreview.h
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsItem2.cpp
        ${LAYOUTSHOP_SRC_DIR}/CustomGraphicsItem2.h
        ${LAYOUTSHOP_SRC_DIR}/PictureBox.cpp
        ${LAYOUTSHOP_SRC_DIR}/PictureBox.h
        ${LAYOUTSHOP_SRC_DIR}/ProjectPaths.h
        ${LAYOUTSHOP_SRC_DIR}/TemplateBox.cpp
        ${LAYOUTSHOP_SRC_DIR}/TemplateBox.h
        ${LAYOUTSHOP_SRC_DIR}/TemplateDialog.cpp
        ${LAYOUTSHOP_SRC_DIR}/TemplateDialog.h
        ${LAYOUTSHOP_SRC_DIR}/FlowLayout.cpp
        ${LAYOUTSHOP_SRC_DIR}/FlowLayout.h
        ${LAYOUTSHOP_SRC_DIR}/DGCNConv.h
        ${LAYOUTSHOP_SRC_DIR}/DGCN.h
        ${LAYOUTSHOP_SRC_DIR}/RefineWidget.cpp
        ${LAYOUTSHOP_SRC_DIR}/RefineWidget.h)

target_compile_definitions(LayoutShop PRIVATE
        LAYOUTSHOP_ASSETS_DIR="${LAYOUTSHOP_ASSETS_DIR}")

target_compile_options(LayoutShop PRIVATE ${TORCH_CXX_FLAGS})
target_include_directories(LayoutShop PRIVATE
        "${LAYOUTSHOP_SRC_DIR}"
        "${RAPIDJSON_INCLUDE_DIR}"
        "${GUROBI_INCLUDE_DIR}")

target_link_libraries(LayoutShop
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::SvgWidgets
        Qt6::PrintSupport
        Eigen3::Eigen
        "${GUROBI_CXX_LIBRARY}"
        "${GUROBI_LIBRARY}"
        "${TORCH_LIBRARIES}"
        )

get_target_property(_layoutshop_link_libs LayoutShop LINK_LIBRARIES)
if(_layoutshop_link_libs)
    remove_agl_entries(_layoutshop_link_libs _layoutshop_link_libs_filtered)
    set_target_properties(LayoutShop PROPERTIES
            LINK_LIBRARIES "${_layoutshop_link_libs_filtered}")
endif()
