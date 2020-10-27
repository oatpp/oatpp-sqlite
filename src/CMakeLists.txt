if(OATPP_SQLITE_AMALGAMATION)

    add_library(sqlite
            sqlite/sqlite3.c
            sqlite/sqlite3.h
    )

    target_include_directories(sqlite
            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sqlite>
    )

    if(OATPP_INSTALL)
        include(GNUInstallDirs)
        target_include_directories(sqlite
                PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/oatpp-${OATPP_THIS_MODULE_VERSION}/${OATPP_THIS_MODULE_NAME}/sqlite>
        )
    endif()

    if(CMAKE_SYSTEM_NAME MATCHES Linux)
        find_package(Threads REQUIRED)
        target_link_libraries(sqlite INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
    endif()

endif()

add_library(${OATPP_THIS_MODULE_NAME}
        oatpp-sqlite/mapping/type/Blob.cpp
        oatpp-sqlite/mapping/type/Blob.hpp
        oatpp-sqlite/mapping/Deserializer.cpp
        oatpp-sqlite/mapping/Deserializer.hpp
        oatpp-sqlite/mapping/ResultMapper.cpp
        oatpp-sqlite/mapping/ResultMapper.hpp
        oatpp-sqlite/mapping/Serializer.cpp
        oatpp-sqlite/mapping/Serializer.hpp
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
        oatpp-sqlite/orm.hpp
        oatpp-sqlite/Utils.cpp
        oatpp-sqlite/Utils.hpp
)

set_target_properties(${OATPP_THIS_MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(${OATPP_THIS_MODULE_NAME} ${LIB_OATPP_EXTERNAL})
endif()

target_include_directories(${OATPP_THIS_MODULE_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_oatpp(${OATPP_THIS_MODULE_NAME})

if (OATPP_SQLITE_AMALGAMATION)

    target_link_libraries(${OATPP_THIS_MODULE_NAME}
            PUBLIC sqlite
            )

    add_dependencies(${OATPP_THIS_MODULE_NAME} sqlite)

else ()
    #In case of vcpkg, specific linking is required because of unofficiality
    if (VCPKG_TOOLCHAIN)
        message("\nvcpkg is on\n")
        target_link_libraries(${OATPP_THIS_MODULE_NAME}
                PUBLIC unofficial::sqlite3::sqlite3
                )

    else ()

        target_link_libraries(${OATPP_THIS_MODULE_NAME}
                PUBLIC SQLite::SQLite3
                )

    endif ()


endif ()

#######################################################################################################
## install targets

if(OATPP_INSTALL)
    include("../cmake/module-install.cmake")
endif()
