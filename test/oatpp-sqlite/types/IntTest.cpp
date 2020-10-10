/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "IntTest.hpp"

#include "oatpp-sqlite/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace sqlite { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(TestEnum, v_int32,
  VALUE(VAL_1, 1024),
  VALUE(VAL_2, 2048),
  VALUE(VAL_3, 4096)
)

class IntsRow : public oatpp::DTO {

  DTO_INIT(IntsRow, DTO);

  DTO_FIELD(Int8, f_int8);
  DTO_FIELD(Int16, f_int16);
  DTO_FIELD(Int32, f_int32);
  DTO_FIELD(Int64, f_int64);
  DTO_FIELD(Boolean, f_bool);
  DTO_FIELD(Enum<TestEnum>::AsNumber, f_enum);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    oatpp::orm::SchemaMigration migration(executor, "IntTest");
    migration.addFile(1, TEST_DB_MIGRATION "IntTest.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion("IntTest");
    OATPP_LOGD("DbClient", "Migration - OK. Version=%d.", version);

  }

  QUERY(insertIntValues,
        "INSERT INTO test_ints "
        "(f_int8, f_int16, f_int32, f_int64, f_bool, f_enum) "
        "VALUES "
        "(:f_int8, :f_int16, :f_int32, :f_int64, :f_bool, :f_enum)",
        PARAM(Int8, f_int8),
        PARAM(Int16, f_int16),
        PARAM(Int32, f_int32),
        PARAM(Int64, f_int64),
        PARAM(Boolean, f_bool),
        PARAM(Enum<TestEnum>::AsNumber, f_enum))

  QUERY(selectAllInts, "SELECT * FROM test_ints")

};

#include OATPP_CODEGEN_END(DbClient)

}

void IntTest::onRun() {

  OATPP_LOGI(TAG, "DB-File='%s'", TEST_DB_FILE);
  std::remove(TEST_DB_FILE);

  auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(TEST_DB_FILE);
  auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionProvider);

  auto client = MyClient(executor);

  {
    auto connection = client.getConnection();

    client.insertIntValues(nullptr,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr,
                           nullptr, connection);

    client.insertIntValues(-1,
                           -1,
                           -1,
                           -1,
                           false,
                           TestEnum::VAL_1, connection);

    client.insertIntValues(std::numeric_limits<v_int8>::min(),
                           std::numeric_limits<v_int16>::min(),
                           std::numeric_limits<v_int32>::min(),
                           std::numeric_limits<v_int64>::min(),
                           true,
                           TestEnum::VAL_2, connection);

    client.insertIntValues(std::numeric_limits<v_int8>::max(),
                           std::numeric_limits<v_int16>::max(),
                           std::numeric_limits<v_int32>::max(),
                           std::numeric_limits<v_int64>::max(),
                           true,
                           TestEnum::VAL_3, connection);

  }

  {
    auto res = client.selectAllInts();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<IntsRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 4);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_int8 == nullptr);
      OATPP_ASSERT(row->f_int16 == nullptr);
      OATPP_ASSERT(row->f_int32 == nullptr);
      OATPP_ASSERT(row->f_int64 == nullptr);
      OATPP_ASSERT(row->f_bool == nullptr);
      OATPP_ASSERT(row->f_enum == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_int8 == -1);
      OATPP_ASSERT(row->f_int16 == -1);
      OATPP_ASSERT(row->f_int32 == -1);
      OATPP_ASSERT(row->f_int64 == -1);
      OATPP_ASSERT(row->f_bool == false);
      OATPP_ASSERT(row->f_enum == TestEnum::VAL_1);
    }

    {
      auto row = dataset[2];
      OATPP_ASSERT(row->f_int8 == std::numeric_limits<v_int8>::min());
      OATPP_ASSERT(row->f_int16 == std::numeric_limits<v_int16>::min());
      OATPP_ASSERT(row->f_int32 == std::numeric_limits<v_int32>::min());
      OATPP_ASSERT(row->f_int64 == std::numeric_limits<v_int64>::min());
      OATPP_ASSERT(row->f_bool == true);
      OATPP_ASSERT(row->f_enum == TestEnum::VAL_2);
    }

    {
      auto row = dataset[3];
      OATPP_ASSERT(row->f_int8 == std::numeric_limits<v_int8>::max());
      OATPP_ASSERT(row->f_int16 == std::numeric_limits<v_int16>::max());
      OATPP_ASSERT(row->f_int32 == std::numeric_limits<v_int32>::max());
      OATPP_ASSERT(row->f_int64 == std::numeric_limits<v_int64>::max());
      OATPP_ASSERT(row->f_bool == true);
      OATPP_ASSERT(row->f_enum == TestEnum::VAL_3);
    }

  }

}

}}}}
