
#include "types/BlobTest.hpp"
#include "types/FloatTest.hpp"
#include "types/IntTest.hpp"
#include "types/StringTest.hpp"

#include "oatpp/core/base/Environment.hpp"

namespace {

void runTests() {

  OATPP_RUN_TEST(oatpp::test::sqlite::types::IntTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::FloatTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::StringTest);
  OATPP_RUN_TEST(oatpp::test::sqlite::types::BlobTest);

}

}

int main() {
  oatpp::base::Environment::init();
  runTests();
  OATPP_ASSERT(oatpp::base::Environment::getObjectsCount() == 0);
  oatpp::base::Environment::destroy();
  return 0;
}
