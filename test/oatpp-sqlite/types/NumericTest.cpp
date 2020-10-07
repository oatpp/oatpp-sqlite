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

#include "NumericTest.hpp"

#include "oatpp-sqlite/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace sqlite { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class NumsRow : public oatpp::DTO {

  DTO_INIT(NumsRow, DTO);

  DTO_FIELD(Int64, f_number);
  DTO_FIELD(Float64, f_decimal);
  DTO_FIELD(Boolean, f_bool);
  DTO_FIELD(String, f_date);
  DTO_FIELD(String, f_datetime);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    oatpp::orm::SchemaMigration migration(executor, "NumericTest");
    migration.addFile(1, TEST_DB_MIGRATION "NumericTest.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion("NumericTest");
    OATPP_LOGD("DbClient", "Migration - OK. Version=%d.", version);

  }

  QUERY(insertNumValues,
        "INSERT INTO test_numerics "
        "(f_number, f_decimal, f_bool, f_date, f_datetime) "
        "VALUES "
        "(:row.f_number, :row.f_decimal, :row.f_bool, date(:row.f_date), datetime(:row.f_datetime));",
        PARAM(oatpp::Object<NumsRow>, row))

  QUERY(deleteAllNums,
        "DELETE FROM test_numerics;")

  QUERY(selectAllNums, "SELECT * FROM test_numerics;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void NumericTest::onRun() {

  OATPP_LOGI(TAG, "DB-File='%s'", TEST_DB_FILE);
  std::remove(TEST_DB_FILE);

  auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(TEST_DB_FILE);
  auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionProvider);

  auto client = MyClient(executor);

  {
    auto res = client.selectAllNums();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<NumsRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 4);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_number == nullptr);
      OATPP_ASSERT(row->f_decimal == nullptr);
      OATPP_ASSERT(row->f_bool == nullptr);
      OATPP_ASSERT(row->f_date == nullptr);
      OATPP_ASSERT(row->f_datetime == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_number == 0);
      OATPP_ASSERT(row->f_decimal == 0);
      OATPP_ASSERT(row->f_bool == false);
      OATPP_ASSERT(row->f_date == "2020-09-03");
      OATPP_ASSERT(row->f_datetime == "2020-09-03 23:59:59");
    }

    {
      auto row = dataset[2];
      OATPP_ASSERT(row->f_number == 1);
      OATPP_ASSERT(row->f_decimal == 1);
      OATPP_ASSERT(row->f_bool == true);
      OATPP_ASSERT(row->f_date == "2020-09-03");
      OATPP_ASSERT(row->f_datetime == "2020-09-03 23:59:59");
    }

    {
      auto row = dataset[3];
      OATPP_ASSERT(row->f_number == 1);
      OATPP_ASSERT(row->f_decimal == 3.14);
      OATPP_ASSERT(row->f_bool == true);
      OATPP_ASSERT(row->f_date == "2020-09-03");
      OATPP_ASSERT(row->f_datetime == "2020-09-03 23:59:59");
    }

  }

  {
    auto res = client.deleteAllNums();
    if (res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    OATPP_ASSERT(res->isSuccess());
  }

  {
    auto connection = client.getConnection();
    {
      auto row = NumsRow::createShared();
      row->f_number = nullptr;
      row->f_decimal = nullptr;
      row->f_bool = nullptr;
      row->f_date = nullptr;
      row->f_datetime = nullptr;
      client.insertNumValues(row, connection);
    }

    {
      auto row = NumsRow::createShared();
      row->f_number = 10;
      row->f_decimal = 10;
      row->f_bool = true;
      row->f_date = "2020-09-04";
      row->f_datetime = "2020-09-04 00:00:00";
      client.insertNumValues(row, connection);
    }
  }

  {
    auto res = client.selectAllNums();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<NumsRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 2);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_number == nullptr);
      OATPP_ASSERT(row->f_decimal == nullptr);
      OATPP_ASSERT(row->f_bool == nullptr);
      OATPP_ASSERT(row->f_date == nullptr);
      OATPP_ASSERT(row->f_datetime == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_number == 10);
      OATPP_ASSERT(row->f_decimal == 10);
      OATPP_ASSERT(row->f_bool == true);
      OATPP_ASSERT(row->f_date == "2020-09-04");
      OATPP_ASSERT(row->f_datetime == "2020-09-04 00:00:00");
    }

  }

}

}}}}
