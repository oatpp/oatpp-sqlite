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

#include "BlobTest.hpp"

#include "oatpp-sqlite/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace sqlite { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class BlobsRow : public oatpp::DTO {

  DTO_INIT(BlobsRow, DTO);

  DTO_FIELD(String, f_string);
  DTO_FIELD(oatpp::sqlite::Blob, f_blob);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    oatpp::orm::SchemaMigration migration(executor, "BlobTest");
    migration.addFile(1, TEST_DB_MIGRATION "BlobTest.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion("BlobTest");
    OATPP_LOGD("DbClient", "Migration - OK. Version=%d.", version);

  }

  QUERY(insertBlobValues,
        "INSERT INTO test_blobs "
        "(f_string, f_blob) "
        "VALUES "
        "(:row.f_string, :row.f_blob);",
        PARAM(oatpp::Object<BlobsRow>, row))

  QUERY(deleteAllBlobs,
        "DELETE FROM test_blobs;")

  QUERY(selectAllBlobs, "SELECT * FROM test_blobs;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void BlobTest::onRun() {

  OATPP_LOGI(TAG, "DB-File='%s'", TEST_DB_FILE);
  std::remove(TEST_DB_FILE);

  auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(TEST_DB_FILE);
  auto connectionPool = oatpp::sqlite::ConnectionPool::createShared(connectionProvider,
                                                                    10,
                                                                    std::chrono::seconds(3));

  auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionPool);

  auto client = MyClient(executor);

  {
    auto res = client.selectAllBlobs();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<BlobsRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 3);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_string == nullptr);
      OATPP_ASSERT(row->f_blob == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_string == "");
      OATPP_ASSERT(oatpp::String(row->f_blob.getPtr()) == "");
    }

    {
      auto row = dataset[2];
      OATPP_ASSERT(row->f_string == "hello world");
      OATPP_ASSERT(oatpp::String(row->f_blob.getPtr()) == "hello world");
    }

  }

  {
    auto res = client.deleteAllBlobs();
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
      auto row = BlobsRow::createShared();
      row->f_string = nullptr;
      row->f_blob = nullptr;
      client.insertBlobValues(row, connection);
    }

    {
      auto row = BlobsRow::createShared();
      row->f_string = "";
      row->f_blob = std::make_shared<std::string>("");
      client.insertBlobValues(row, connection);
    }

    {
      auto row = BlobsRow::createShared();
      row->f_string = "Hello Oat++";
      row->f_blob = std::make_shared<std::string>("Hello Oat++");
      client.insertBlobValues(row, connection);
    }
  }

  {
    auto res = client.selectAllBlobs();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<BlobsRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 3);

    {
      auto row = dataset[0];
      OATPP_ASSERT(row->f_string == nullptr);
      OATPP_ASSERT(row->f_blob == nullptr);
    }

    {
      auto row = dataset[1];
      OATPP_ASSERT(row->f_string == "");
      OATPP_ASSERT(oatpp::String(row->f_blob.getPtr()) == "");
    }

    {
      auto row = dataset[2];
      OATPP_ASSERT(row->f_string == "Hello Oat++");
      OATPP_ASSERT(oatpp::String(row->f_blob.getPtr()) == "Hello Oat++");
    }

  }

  connectionPool->stop();

}

}}}}
