
#include "ql_template/ParserTest.hpp"

#include "types/BlobTest.hpp"
#include "types/IntTest.hpp"
#include "types/NumericTest.hpp"
#include "types/InterpretationTest.hpp"

#include "oatpp/Environment.hpp"

namespace {

void runTests() {

  OATPP_RUN_TEST(oatpp::test::sqlite::ql_template::ParserTest);

  OATPP_RUN_TEST(oatpp::test::sqlite::types::IntTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::NumericTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::BlobTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::InterpretationTest);

}

}

int main() {
  oatpp::Environment::init();
  runTests();
  OATPP_ASSERT(oatpp::Environment::getObjectsCount() == 0);
  oatpp::Environment::destroy();
  return 0;
}
