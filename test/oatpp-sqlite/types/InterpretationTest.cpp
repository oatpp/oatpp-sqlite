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

#include "InterpretationTest.hpp"

#include "oatpp-sqlite/orm.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace sqlite { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class PointDto : public oatpp::DTO {

  DTO_INIT(PointDto, DTO);

  DTO_FIELD(Int64, x, "f_x");
  DTO_FIELD(Int64, y, "f_y");
  DTO_FIELD(Int64, z, "f_z");

};

#include OATPP_CODEGEN_END(DTO)

struct VPoint {
  v_int64 x;
  v_int64 y;
  v_int64 z;
};

namespace __class {
  class PointClass;
}

typedef oatpp::data::mapping::type::ObjectWrapper<VPoint, __class::PointClass> Point;

namespace __class {

  class PointClass {
  private:

    class Inter : public oatpp::Type::Interpretation<Point, oatpp::Object<PointDto>>  {
    public:

      oatpp::Object<PointDto> interpret(const Point& value) const override {
        auto dto = PointDto::createShared();
        dto->x = value->x;
        dto->y = value->y;
        dto->z = value->z;
        return dto;
      }

      Point reproduce(const oatpp::Object<PointDto>& value) const override {
        return std::make_shared<VPoint>(VPoint({value->x, value->y, value->z}));
      }

    };

  public:

    static const oatpp::ClassId CLASS_ID;

    static oatpp::Type* getType(){
      static Type type(
        CLASS_ID, nullptr, nullptr,
        {
          {"test", new Inter()}
        }
      );
      return &type;
    }

  };

  const oatpp::ClassId PointClass::CLASS_ID("test::Point");

}

#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {
    oatpp::orm::SchemaMigration migration(executor, "InterpretationTest");
    migration.addFile(1, TEST_DB_MIGRATION "InterpretationTest.sql");
    migration.migrate();

    auto version = executor->getSchemaVersion("InterpretationTest");
    OATPP_LOGD("DbClient", "Migration - OK. Version=%d.", version);

    setEnabledInterpretations({"test"});

  }

  QUERY(insertValues,
        "INSERT INTO test_interpretation_points "
        "(f_x, f_y, f_z) "
        "VALUES "
        "(:point.f_x, :point.f_y, :point.f_z);",
        PARAM(Point, point))

  QUERY(selectAll, "SELECT * FROM test_interpretation_points;")

};

#include OATPP_CODEGEN_END(DbClient)

}

void InterpretationTest::onRun() {

  OATPP_LOGI(TAG, "DB-File='%s'", TEST_DB_FILE);
  std::remove(TEST_DB_FILE);

  auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(TEST_DB_FILE);
  auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionProvider);

  auto client = MyClient(executor);

  {

    auto connection = client.getConnection();

    {
      Point point = std::make_shared<VPoint>();
      point->x = 1;
      point->y = 2;
      point->z = 3;
      client.insertValues(point, connection);
    }

    {
      Point point = std::make_shared<VPoint>();
      point->x = 10;
      point->y = 20;
      point->z = 30;
      client.insertValues(point, connection);
    }

    {
      Point point = std::make_shared<VPoint>();
      point->x = 100;
      point->y = 200;
      point->z = 300;
      client.insertValues(point, connection);
    }

  }

  {
    auto res = client.selectAll();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<Point>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite", "test"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 3);

    {
      auto point = dataset[0];
      OATPP_ASSERT(point->x == 1);
      OATPP_ASSERT(point->y == 2);
      OATPP_ASSERT(point->z == 3);
    }

    {
      auto point = dataset[1];
      OATPP_ASSERT(point->x == 10);
      OATPP_ASSERT(point->y == 20);
      OATPP_ASSERT(point->z == 30);
    }

    {
      auto point = dataset[2];
      OATPP_ASSERT(point->x == 100);
      OATPP_ASSERT(point->y == 200);
      OATPP_ASSERT(point->z == 300);
    }

  }

}

}}}}
