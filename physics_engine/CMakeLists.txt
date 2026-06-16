cmake_minimum_required(VERSION 3.24)
project(physics_engine LANGUAGES CXX)

file(GLOB_RECURSE PROJECT_SOURCES
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.hpp"
    "src/*.c"
    "src/*.h"
)

add_library(physics_engine STATIC
    ${PROJECT_SOURCES}
)

target_include_directories(physics_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/physics_engine>
    PRIVATE
        src
)


FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)

message(STATUS "Fetching glm")
set(GLM_BUILD_TESTS OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glm)

# Boost dependencies
set(BOOST_OTHER_LIBS config assert core static_assert type_traits move intrusive throw_exception)
foreach(lib ${BOOST_OTHER_LIBS})
    FetchContent_Declare(
        boost_${lib}
        GIT_REPOSITORY https://github.com/boostorg/${lib}.git
        GIT_TAG        boost-1.91.0
    )
endforeach()

FetchContent_Declare(
    boost_container
    GIT_REPOSITORY https://github.com/boostorg/container.git
    GIT_TAG        fd8cb9b2aa5b3ab6f15425358c38b7644eba7ded
)

# Boost
message("Fetching Boost")
FetchContent_MakeAvailable(boost_container)
foreach(lib ${BOOST_OTHER_LIBS})
    FetchContent_MakeAvailable(boost_${lib})
endforeach()

target_link_libraries(physics_engine PUBLIC
    glm::glm-header-only
    Boost::container
    Boost::config
    Boost::assert
    Boost::core
    Boost::static_assert
    Boost::type_traits
    Boost::move
    Boost::intrusive
    Boost::throw_exception
)

set_target_properties(physics_engine PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
