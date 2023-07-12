set(_GUROBI_KNOWN_VERSIONS ${PROJECT_SOURCE_DIR}/gurobi1002 ${PROJECT_SOURCE_DIR}/gurobi1001)

find_path(GUROBI_HOME
        NAMES include/gurobi_c++.h include/gurobi_c.h gurobi_c++.h gurobi_c.h
        PATHS ${GUROBI_DIR} $ENV{GUROBI_HOME} ${_GUROBI_KNOWN_VERSIONS}
        PATH_SUFFIXES include linux64 win64 mac64 macos macos_universal2
        $ENV{GUROBI_HOME}
      )

set(GUROBI_INCLUDE_DIR "${GUROBI_HOME}/include")   
set(GUROBI_BIN_DIR "${GUROBI_HOME}/bin")
set(GUROBI_LIB_DIR "${GUROBI_HOME}/lib")

message("GUROBI_INCLUDE_DIR: ${GUROBI_INCLUDE_DIR}")
message("GUROBI_BIN_DIR: ${GUROBI_BIN_DIR}")
message("GUROBI_LIB_DIR: ${GUROBI_LIB_DIR}")

if (WIN32)
    file(GLOB GUROBI_LIBRARY_LIST
            RELATIVE ${GUROBI_BIN_DIR}
            ${GUROBI_BIN_DIR}/gurobi*.dll
            )
else()
    message("I am in ELSE")
    file(GLOB GUROBI_LIBRARY_LIST
            RELATIVE ${GUROBI_LIB_DIR}
            ${GUROBI_LIB_DIR}/libgurobi*.dylib
            )
endif()

message("GUROBI_LIBRARY_LIST: ${GUROBI_LIBRARY_LIST}")

# Ignore libgurobiXY_light.so, libgurobi.so (without version):
string(REGEX MATCHALL
        "gurobi([0-9]+)\\..*"
        GUROBI_LIBRARY_LIST
        "${GUROBI_LIBRARY_LIST}"
        )

string(REGEX REPLACE
        ".*gurobi([0-9]+)\\..*"
        "\\1"
        GUROBI_LIBRARY_VERSIONS
        "${GUROBI_LIBRARY_LIST}")
list(LENGTH GUROBI_LIBRARY_VERSIONS GUROBI_NUMVER)

message("GUROBI_LIBRARY_LIST: ${GUROBI_LIBRARY_LIST}")
message("GUROBI LIB VERSIONS: ${GUROBI_LIBRARY_VERSIONS}")

if (GUROBI_NUMVER EQUAL 0)
    message(STATUS "Found no Gurobi library version, GUROBI_HOME = ${GUROBI_HOME}.")
elseif (GUROBI_NUMVER EQUAL 1)
    list(GET GUROBI_LIBRARY_VERSIONS 0 GUROBI_LIBRARY_VERSION)
else()
    # none or more than one versioned library -let's try without suffix,
    # maybe the user added a symlink to the desired library
    message(STATUS "Found more than one Gurobi library version (${GUROBI_LIBRARY_VERSIONS}), trying without suffix. Set GUROBI_LIBRARY if you want to pick a certain one.")
    set(GUROBI_LIBRARY_VERSION "")
  endif()

  if (WIN32)
    find_library(GUROBI_LIBRARY
            NAMES "gurobi${GUROBI_LIBRARY_VERSION}"
            PATHS
            ${GUROBI_BIN_DIR}
            )
    find_library(GUROBI_IMPLIB
            NAMES "gurobi${GUROBI_LIBRARY_VERSION}"
            PATHS
            ${GUROBI_LIB_DIR}
            )
    mark_as_advanced(GUROBI_IMPLIB)
else ()
    find_library(GUROBI_LIBRARY
            NAMES "gurobi${GUROBI_LIBRARY_VERSION}"
            PATHS
            ${GUROBI_LIB_DIR}
            )
endif()
mark_as_advanced(GUROBI_LIBRARY)

if(GUROBI_LIBRARY AND NOT TARGET Gurobi::GurobiC)
    add_library(Gurobi::GurobiC SHARED IMPORTED)
    target_include_directories(Gurobi::GurobiC INTERFACE ${GUROBI_INCLUDE_DIR})
    set_target_properties(Gurobi::GurobiC PROPERTIES IMPORTED_LOCATION ${GUROBI_LIBRARY})
    if (GUROBI_IMPLIB)
        set_target_properties(Gurobi::GurobiC PROPERTIES IMPORTED_IMPLIB ${GUROBI_IMPLIB})
    endif()
endif()

# Gurobi ships with some compiled versions of its C++ library for specific
# compilers, however it also comes with the source code. We will compile
# the source code outselves -- this is much safer, as it guarantees the same
# compiler version and flags.
# (Note: doing this is motivated by actual sometimes-subtle ABI compatibility bugs)

find_path(GUROBI_SRC_DIR NAMES "Model.h" PATHS "${GUROBI_HOME}/src/cpp/")
mark_as_advanced(GUROBI_SRC_DIR)

file(GLOB GUROBI_CXX_SRC CONFIGURE_DEPENDS ${GUROBI_SRC_DIR}/*.cpp)
if(TARGET Gurobi::GurobiC AND GUROBI_CXX_SRC AND NOT TARGET Gurobi::GurobiCXX)
    add_library(GurobiCXX STATIC EXCLUDE_FROM_ALL ${GUROBI_CXX_SRC})
    add_library(Gurobi::GurobiCXX ALIAS GurobiCXX)

    if(MSVC)
        target_compile_definitions(GurobiCXX PRIVATE "WIN64")
    endif()

    target_include_directories(GurobiCXX PUBLIC ${GUROBI_INCLUDE_DIR})
    target_link_libraries(GurobiCXX PUBLIC Gurobi::GurobiC)

    # We need to be able to link this into a shared library
    # and set the SYSTEM flag for the include dirs of the Gurobi libs to suppress
    # warnings
    set_target_properties(GurobiCXX PROPERTIES POSITION_INDEPENDENT_CODE ON
      INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:GurobiCXX,INTERFACE_INCLUDE_DIRECTORIES>)# cmake-lint: disable=C0307
endif()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gurobi DEFAULT_MSG GUROBI_LIBRARY GUROBI_INCLUDE_DIR GUROBI_SRC_DIR)
