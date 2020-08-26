
#include "oatpp-sqlite/Executor.hpp"
#include "oatpp-sqlite/Types.hpp"

#include "oatpp-test/UnitTest.hpp"

#include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/base/Environment.hpp"

#include <iostream>

#include "oatpp/orm/DbClient.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class Ints : public oatpp::DTO {

  DTO_INIT(Ints, DTO)

  DTO_FIELD(Int8, f_int8);
  DTO_FIELD(UInt8, f_uint8);

  DTO_FIELD(Int16, f_int16);
  DTO_FIELD(UInt16, f_uint16);

  DTO_FIELD(Int32, f_int32);
  DTO_FIELD(UInt32, f_uint32);

  DTO_FIELD(Int64, f_int64);
  DTO_FIELD(UInt64, f_uint64);

};

#include OATPP_CODEGEN_END(DTO)

#include OATPP_CODEGEN_BEGIN(DTO)
#include OATPP_CODEGEN_BEGIN(DbClient)

class MyClient : public oatpp::orm::DbClient {
public:

  MyClient(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {}

  QUERY(getUserById, "SELECT * FROM user WHERE tag=$<text>$a$<text>$ AND userId=:userId AND role=:role",
        PARAM(oatpp::String, userId),
        PARAM(oatpp::String, role))

  QUERY(createUser,
        "INSERT INTO EXAMPLE_USER "
        "(userId, login, password, email) VALUES "
        "(uuid_generate_v4(), :login, :password, :email) "
        "RETURNING *;",
        PARAM(oatpp::String, login),
        PARAM(oatpp::String, password),
        PARAM(oatpp::String, email),
        PREPARE(true))

  QUERY(createUserUuid,
        "INSERT INTO EXAMPLE_USER "
        "(userId, login, password, email) VALUES "
        "(:userId, :login, :password, :email) "
        "RETURNING *;",
        PARAM(oatpp::sqlite::Uuid, userId),
        PARAM(oatpp::String, login),
        PARAM(oatpp::String, password),
        PARAM(oatpp::String, email),
        PREPARE(true))

  QUERY(selectUsers, "SELECT * FROM EXAMPLE_USER", PREPARE(true))

  QUERY(insertStrs,
        "INSERT INTO test_strs "
        "(f_str1, f_str2, f_str3) VALUES "
        "(:f_str1.param, :f_str2.param, :f_str3.param);",
        PARAM(oatpp::String, f_str1, "f_str1.param"),
        PARAM(oatpp::String, f_str2, "f_str2.param"),
        PARAM(oatpp::String, f_str3, "f_str3.param"),
        PREPARE(true))


  class InsertStrsDto : public oatpp::DTO {

    DTO_INIT(InsertStrsDto, DTO)

    DTO_FIELD(oatpp::String, f_str1);
    DTO_FIELD(oatpp::String, f_str2);
    DTO_FIELD(oatpp::String, f_str3);

  };

  QUERY(insertStrsWithDtoParams,
        "INSERT INTO test_strs "
        "(f_str1, f_str2, f_str3) VALUES "
        "(:dto.f_str1, :dto.f_str2, :dto.f_str3);",
        PARAMS_DTO(oatpp::Object<InsertStrsDto>, rowDto, "dto"),
        PREPARE(true))

  QUERY(selectStrs, "SELECT * FROM test_strs", PREPARE(true))

  QUERY(insertInts,
        "INSERT INTO test_ints "
        "(f_int8, f_uint8, f_int16, f_uint16, f_int32, f_uint32, f_int64) VALUES "
        "(:f_int8, :f_uint8, :f_int16, :f_uint16, :f_int32, :f_uint32, :f_int64);",
        PARAM(oatpp::Int8, f_int8), PARAM(oatpp::UInt8, f_uint8),
        PARAM(oatpp::Int16, f_int16), PARAM(oatpp::UInt16, f_uint16),
        PARAM(oatpp::Int32, f_int32), PARAM(oatpp::UInt32, f_uint32),
        PARAM(oatpp::Int64, f_int64),
        PREPARE(true))

  QUERY(selectInts, "SELECT * FROM test_ints")

  QUERY(insertFloats,
        "INSERT INTO test_floats "
        "(f_float32, f_float64) VALUES "
        "(:f_float32, :f_float64);",
        PARAM(oatpp::Float32, f_float32), PARAM(oatpp::Float64, f_float64),
        PREPARE(true))

  QUERY(selectFloats, "SELECT * FROM test_floats")

  std::shared_ptr<oatpp::orm::QueryResult> insertMultipleUsers() {
    auto t = beginTransaction();
    createUser("admin5", "AdMiN", "admin5@admin.com", t.getConnection());
    createUser("admin6", "AdMiN", "admin6@admin.com", t.getConnection());
    return t.commit();
  }

};

#include OATPP_CODEGEN_END(DbClient)
#include OATPP_CODEGEN_END(DTO)

class Test : public oatpp::test::UnitTest {
public:
  Test() : oatpp::test::UnitTest("MyTag")
  {}

  void onRun() override {

    {
      v_char8 data1[16] = "012345670123456";
      oatpp::sqlite::Uuid uuid1(data1);
      oatpp::sqlite::Uuid uuid2(oatpp::String("8e6d32f7-aca2-4601-b89a-f2c55b010962"));

      OATPP_LOGD(TAG, "uuid1='%s'", uuid1->toString()->c_str());
      OATPP_LOGD(TAG, "uuid2='%s'", uuid2->toString()->c_str());

      if(uuid1 == uuid2) {
        OATPP_LOGD(TAG, "eq");
      } else {
        OATPP_LOGD(TAG, "!eq");
      }
    }

    oatpp::String connStr = "/Users/leonid/Documents/test/db/db1";
    auto connectionProvider = std::make_shared<oatpp::sqlite::ConnectionProvider>(connStr);
    auto connectionPool = oatpp::sqlite::ConnectionPool::createShared(
      connectionProvider,
      10,
      std::chrono::seconds(1)
    );

    auto executor = std::make_shared<oatpp::sqlite::Executor>(connectionPool);
    auto client = MyClient(executor);

    //client.createUser("my-login1", "pass1", "email@email.com1", connection);
    //client.createUser("my-login2", "pass2", "email@email.com2", connection);

    //client.insertInts(8, 8, 16, 16, 32, 32, 64, connection);
    //client.insertInts(-1, -1, -1, -1, -1, -1, -1, connection);

    //client.insertFloats(0.32, 0.64, connection);
    //client.insertFloats(-0.32, -0.64, connection);

//    client.insertStrs("Hello", "Dot", "Param");
//    client.insertStrs("Hello", "World", "oatpp");
//    client.insertStrs("Yeah", "Ops", "!!!");

    {
      auto row = MyClient::InsertStrsDto::createShared();
      row->f_str1 = "A_3";
      row->f_str2 = "B_3";
      row->f_str3 = "C_3";
      //client.insertStrsWithDtoParams(row);
    }

    {

      //auto res = client.createUser("admin1", "AdMiN", "admin1@admin.com");
      auto res = client.selectStrs();
      //auto res = client.insertMultipleUsers();
      //auto res = client.insertStrs("Hello", "Dot", "Param");

      if(res->isSuccess()) {
        OATPP_LOGD(TAG, "OK, knownCount=%d, hasMore=%d", res->getKnownCount(), res->hasMoreToFetch());
      } else {
        auto message = res->getErrorMessage();
        OATPP_LOGD(TAG, "Error, message=%s", message->c_str());
      }

      auto dataset = res->fetch<oatpp::Vector<oatpp::Fields<oatpp::Any>>>();

      oatpp::parser::json::mapping::ObjectMapper om;
      om.getSerializer()->getConfig()->useBeautifier = true;
      om.getSerializer()->getConfig()->enableInterpretations = {"sqlite"};

      auto str = om.writeToString(dataset);

      OATPP_LOGD(TAG, "res=%s", str->c_str());

    }

    connectionPool->stop();

  }

};

void runTests() {
  OATPP_RUN_TEST(Test);
}

}

int main() {

  oatpp::base::Environment::init();

  runTests();

  /* Print how much objects were created during app running, and what have left-probably leaked */
  /* Disable object counting for release builds using '-D OATPP_DISABLE_ENV_OBJECT_COUNTERS' flag for better performance */
  std::cout << "\nEnvironment:\n";
  std::cout << "objectsCount = " << oatpp::base::Environment::getObjectsCount() << "\n";
  std::cout << "objectsCreated = " << oatpp::base::Environment::getObjectsCreated() << "\n\n";

  OATPP_ASSERT(oatpp::base::Environment::getObjectsCount() == 0);

  oatpp::base::Environment::destroy();

  return 0;
}
