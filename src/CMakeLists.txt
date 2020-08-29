add_library(sqlite
        sqlite/sqlite3.c
        sqlite/sqlite3.h
)

target_include_directories(sqlite PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_library(${OATPP_THIS_MODULE_NAME}
        oatpp-sqlite/mapping/type/Blob.cpp
        oatpp-sqlite/mapping/type/Blob.hpp
        oatpp-sqlite/mapping/Deserializer.cpp
        oatpp-sqlite/mapping/Deserializer.hpp
        oatpp-sqlite/mapping/ResultMapper.cpp
        oatpp-sqlite/mapping/ResultMapper.hpp
        oatpp-sqlite/mapping/Serializer.cpp
        oatpp-sqlite/mapping/Serializer.hpp
        oatpp-sqlite/mapping/TypeMapper.cpp
        oatpp-sqlite/mapping/TypeMapper.hpp
        oatpp-sqlite/ql_template/Parser.cpp
        oatpp-sqlite/ql_template/Parser.hpp
        oatpp-sqlite/ql_template/TemplateValueProvider.cpp
        oatpp-sqlite/ql_template/TemplateValueProvider.hpp
        oatpp-sqlite/Connection.cpp
        oatpp-sqlite/Connection.hpp
        oatpp-sqlite/ConnectionProvider.cpp
        oatpp-sqlite/ConnectionProvider.hpp
        oatpp-sqlite/Executor.cpp
        oatpp-sqlite/Executor.hpp
        oatpp-sqlite/QueryResult.cpp
        oatpp-sqlite/QueryResult.hpp
        oatpp-sqlite/Types.hpp
        oatpp-sqlite/orm.hpp)

set_target_properties(${OATPP_THIS_MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(${OATPP_THIS_MODULE_NAME} ${LIB_OATPP_EXTERNAL})
endif()

target_link_oatpp(${OATPP_THIS_MODULE_NAME})

target_link_libraries(${OATPP_THIS_MODULE_NAME}
        PUBLIC sqlite
)

target_include_directories(${OATPP_THIS_MODULE_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_dependencies(${OATPP_THIS_MODULE_NAME} sqlite)

#######################################################################################################
## install targets

if(OATPP_INSTALL)
    include("../cmake/module-install.cmake")
endif()
