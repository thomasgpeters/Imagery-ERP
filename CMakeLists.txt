cmake_minimum_required(VERSION 3.14)
project(ProjectPlanningApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# =============================================================================
# Build Instructions:
#   mkdir build && cd build
#   cmake ..
#   make -j$(nproc)
#
# Run Instructions:
#   ./project_planner --docroot ../resources --http-address 0.0.0.0 --http-port 8080
#
# Then open http://localhost:8080 in your browser.
#
# Optional Dependencies:
#   - libharu (HPDF): For PDF generation. Install: apt-get install libhpdf-dev
#     If not available, PDF export will generate HTML-based documents.
# =============================================================================

# --- Find Wt ---
# Try CMake config mode first (Wt 4.6+), then fallback to manual search
find_package(wt CONFIG QUIET)
if(wt_FOUND)
    message(STATUS "Found Wt via CMake config")
    set(WT_LIBS Wt::Wt Wt::HTTP)
else()
    find_library(WT_LIB NAMES wt HINTS /usr/local/lib /usr/lib)
    find_library(WT_HTTP_LIB NAMES wthttp HINTS /usr/local/lib /usr/lib)
    find_path(WT_INCLUDE Wt/WApplication.h HINTS /usr/local/include /usr/include)

    if(NOT WT_LIB OR NOT WT_HTTP_LIB)
        message(FATAL_ERROR
            "Wt (Witty) library not found.\n"
            "Install Wt: https://www.webtoolkit.eu/wt/download\n"
            "  Ubuntu/Debian: apt-get install libwt-dev libwthttp-dev\n"
            "  From source: cmake/make/make install from Wt source tree")
    endif()

    message(STATUS "Found Wt: ${WT_LIB}")
    message(STATUS "Found WtHTTP: ${WT_HTTP_LIB}")
    set(WT_LIBS ${WT_LIB} ${WT_HTTP_LIB})
    if(WT_INCLUDE)
        include_directories(${WT_INCLUDE})
    endif()
endif()

# --- Optional: libharu for PDF ---
find_library(HPDF_LIB NAMES hpdf QUIET)
find_path(HPDF_INCLUDE hpdf.h QUIET)
if(HPDF_LIB AND HPDF_INCLUDE)
    message(STATUS "Found libharu: ${HPDF_LIB}")
    add_definitions(-DHAVE_HPDF)
    include_directories(${HPDF_INCLUDE})
    set(PDF_LIBS ${HPDF_LIB})
else()
    message(STATUS "libharu not found - PDF export will use HTML fallback")
    set(PDF_LIBS "")
endif()

# --- Build the application ---
add_executable(project_planner
    src/main.cpp
    src/App.cpp
    src/DashboardView.cpp
    src/ResourceView.cpp
    src/ComponentView.cpp
    src/SprintView.cpp
    src/CostingView.cpp
    src/QuoteView.cpp
    src/ChangeOrderView.cpp
    src/MaterialView.cpp
    src/PdfExporter.cpp
)

target_include_directories(project_planner PRIVATE src)
target_link_libraries(project_planner ${WT_LIBS} ${PDF_LIBS})

# Suppress deprecation warnings from Wt headers (WLogger.h uses sprintf)
target_compile_options(project_planner PRIVATE -Wno-deprecated-declarations)

# Copy resources to build directory
file(COPY resources DESTINATION ${CMAKE_BINARY_DIR})
