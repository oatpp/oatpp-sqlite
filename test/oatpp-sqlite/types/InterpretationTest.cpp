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
#include "oatpp/core/utils/ConversionUtils.hpp"

#include <limits>
#include <cstdio>

namespace oatpp { namespace test { namespace sqlite { namespace types {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

struct VPoint {
  v_int32 x;
  v_int32 y;
  v_int32 z;
};

struct VLine {
  VPoint p1;
  VPoint p2;
};

namespace __class {
 class PointClass;
 class LineClass;
 class StringNumberClass;
}

typedef oatpp::data::mapping::type::Primitive<VPoint, __class::PointClass> Point;
typedef oatpp::data::mapping::type::Primitive<VLine, __class::LineClass> Line;
typedef oatpp::data::mapping::type::ObjectWrapper<std::string, __class::StringNumberClass> StringNumber;

namespace __class {

  class PointClass {
  private:

    class PointDto : public oatpp::DTO {

      DTO_INIT(PointDto, DTO)

      DTO_FIELD(Int32, x, "f_x");
      DTO_FIELD(Int32, y, "f_y");
      DTO_FIELD(Int32, z, "f_z");

    };

    class Inter : public oatpp::Type::Interpretation<Point, oatpp::Object<PointDto>> {
    public:

      oatpp::Object<PointDto> interpret(const Point &value) const override {
        OATPP_LOGD("Point::Interpretation", "interpret");
        auto dto = PointDto::createShared();
        dto->x = value->x;
        dto->y = value->y;
        dto->z = value->z;
        return dto;
      }

      Point reproduce(const oatpp::Object<PointDto> &value) const override {
        OATPP_LOGD("Point::Interpretation", "reproduce");
        return Point({value->x, value->y, value->z});
      }

    };

  private:

    static oatpp::Type* createType() {
      oatpp::Type::Info info;
      info.interpretationMap = {{"test", new Inter()}};
      return new oatpp::Type(CLASS_ID, info);
    }

  public:

    static const oatpp::ClassId CLASS_ID;

    static oatpp::Type *getType() {
      static Type* type = createType();
      return type;
    }

  };

  const oatpp::ClassId PointClass::CLASS_ID("test::Point");

  class LineClass {
  private:

    class LineDto : public oatpp::DTO {

      DTO_INIT(LineDto, DTO)

      DTO_FIELD(Point, p1);
      DTO_FIELD(Point, p2);

    };

    class Inter : public oatpp::Type::Interpretation<Line, oatpp::Object<LineDto>> {
    public:

      oatpp::Object<LineDto> interpret(const Line &value) const override {
        OATPP_LOGD("Line::Interpretation", "interpret");
        auto dto = LineDto::createShared();
        dto->p1 = {value->p1.x, value->p1.y, value->p1.z};
        dto->p2 = {value->p2.x, value->p2.y, value->p2.z};
        return dto;
      }

      Line reproduce(const oatpp::Object<LineDto> &value) const override {
        OATPP_LOGD("Line::Interpretation", "reproduce");
        return Line({{value->p1->x, value->p1->y, value->p1->z},
                     {value->p2->x, value->p2->y, value->p2->z}});
      }

    };

  private:

    static oatpp::Type* createType() {
      oatpp::Type::Info info;
      info.interpretationMap = {{"test", new Inter()}};
      return new oatpp::Type(CLASS_ID, info);
    }

  public:

    static const oatpp::ClassId CLASS_ID;

    static oatpp::Type *getType() {
      static Type* type = createType();
      return type;
    }

  };

  const oatpp::ClassId LineClass::CLASS_ID("test::Line");

  class StringNumberClass {
  private:

    class Inter : public oatpp::Type::Interpretation<StringNumber, oatpp::Int64> {
    public:

      oatpp::Int64 interpret(const StringNumber& value) const override {
        OATPP_LOGD("StringNumber::Interpretation", "interpret");
        return oatpp::utils::conversion::strToInt64(value->c_str());
      }

      StringNumber reproduce(const oatpp::Int64& value) const override {
        OATPP_LOGD("StringNumber::Interpretation", "reproduce");
        return oatpp::utils::conversion::int64ToStr(value).getPtr();
      }

    };

  private:

    static oatpp::Type* createType() {
      oatpp::Type::Info info;
      info.interpretationMap = {{"test", new Inter()}};
      return new oatpp::Type(CLASS_ID, info);
    }

  public:

    static const oatpp::ClassId CLASS_ID;

    static oatpp::Type *getType() {
      static Type* type = createType();
      return type;
    }

  };

  const oatpp::ClassId StringNumberClass::CLASS_ID("test::StringNumber");


}

class LineInterRow : public oatpp::DTO {

  DTO_INIT(LineInterRow, DTO);

  DTO_FIELD(Int64, f_x1);
  DTO_FIELD(Int64, f_y1);
  DTO_FIELD(Int64, f_z1);

  DTO_FIELD(StringNumber, f_x2);
  DTO_FIELD(StringNumber, f_y2);
  DTO_FIELD(StringNumber, f_z2);

};

#include OATPP_CODEGEN_END(DTO)

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

  QUERY(insertPoints,
        "INSERT INTO test_interpretation_points "
        "(f_x, f_y, f_z) "
        "VALUES "
        "(:point.f_x, :point.f_y, :point.f_z);",
        PARAM(Point, point))

  QUERY(selectPoints, "SELECT * FROM test_interpretation_points;")

  QUERY(insertLines,
        "INSERT INTO test_interpretation_lines "
        "(f_x1, f_y1, f_z1, f_x2, f_y2, f_z2) "
        "VALUES "
        "(:line.p1.f_x, :line.p1.f_y, :line.p1.f_z, :line.p2.f_x, :line.p2.f_y, :line.p2.f_z);",
        PARAM(Line, line))

  QUERY(selectLines, "SELECT * FROM test_interpretation_lines;")

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
      client.insertPoints(point, connection);
    }

    {
      Point point = std::make_shared<VPoint>();
      point->x = 10;
      point->y = 20;
      point->z = 30;
      client.insertPoints(point, connection);
    }

    {
      Point point = std::make_shared<VPoint>();
      point->x = 100;
      point->y = 200;
      point->z = 300;
      client.insertPoints(point, connection);
    }

  }

  {
    auto res = client.selectPoints();
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

  {

    auto connection = client.getConnection();

    {
      Line line = std::make_shared<VLine>();
      line->p1.x = 11;
      line->p1.y = 12;
      line->p1.z = 13;
      line->p2.x = 21;
      line->p2.y = 22;
      line->p2.z = 23;
      client.insertLines(line, connection);
    }

    {
      Line line = std::make_shared<VLine>();
      line->p1.x = 110;
      line->p1.y = 120;
      line->p1.z = 130;
      line->p2.x = 210;
      line->p2.y = 220;
      line->p2.z = 230;
      client.insertLines(line, connection);
    }

    {
      Line line = std::make_shared<VLine>();
      line->p1.x = 1100;
      line->p1.y = 1200;
      line->p1.z = 1300;
      line->p2.x = 2100;
      line->p2.y = 2200;
      line->p2.z = 2300;
      client.insertLines(line, connection);
    }

  }


  {
    auto res = client.selectLines();
    if(res->isSuccess()) {
      OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
    } else {
      auto message = res->getErrorMessage();
      OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
    }

    auto dataset = res->fetch<oatpp::Vector<oatpp::Object<LineInterRow>>>();

    oatpp::parser::json::mapping::ObjectMapper om;
    om.getSerializer()->getConfig()->useBeautifier = true;
    om.getSerializer()->getConfig()->enabledInterpretations = {"sqlite", "test"};

    auto str = om.writeToString(dataset);

    OATPP_LOGD(TAG, "res=%s", str->c_str());

    OATPP_ASSERT(dataset->size() == 3);

    {
      auto line = dataset[0];
      OATPP_ASSERT(line->f_x1 == 11);
      OATPP_ASSERT(line->f_y1 == 12);
      OATPP_ASSERT(line->f_z1 == 13);
      OATPP_ASSERT(*line->f_x2.get() == "21");
      OATPP_ASSERT(*line->f_y2.get() == "22");
      OATPP_ASSERT(*line->f_z2.get() == "23");
    }

  }


}

}}}}
