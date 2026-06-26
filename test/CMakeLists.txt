add_definitions (
        -DTEST_DB_FILE="${CMAKE_CURRENT_SOURCE_DIR}/../test_db.sqlite"
        -DTEST_DB_MIGRATION="${CMAKE_CURRENT_SOURCE_DIR}/oatpp-sqlite/migration/"
)

add_executable(oatpp-sqlite-tests
        oatpp-sqlite/ql_template/ParserTest.cpp
        oatpp-sqlite/ql_template/ParserTest.hpp
        oatpp-sqlite/types/BlobTest.cpp
        oatpp-sqlite/types/BlobTest.hpp
        oatpp-sqlite/types/InterpretationTest.cpp
        oatpp-sqlite/types/InterpretationTest.hpp
        oatpp-sqlite/types/IntTest.cpp
        oatpp-sqlite/types/IntTest.hpp
        oatpp-sqlite/types/NumericTest.cpp
        oatpp-sqlite/types/NumericTest.hpp
        oatpp-sqlite/tests.cpp)

set_target_properties(oatpp-sqlite-tests PROPERTIES
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(oatpp-sqlite-tests
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(OATPP_MODULES_LOCATION STREQUAL OATPP_MODULES_LOCATION_EXTERNAL)
    add_dependencies(oatpp-sqlite-tests ${LIB_OATPP_EXTERNAL})
endif()

add_dependencies(oatpp-sqlite-tests ${OATPP_THIS_MODULE_NAME})

target_link_oatpp(oatpp-sqlite-tests)

target_link_libraries(oatpp-sqlite-tests
        PRIVATE ${OATPP_THIS_MODULE_NAME}
)

## TODO link dependencies here (if some)

add_test(oatpp-sqlite-tests oatpp-sqlite-tests)
