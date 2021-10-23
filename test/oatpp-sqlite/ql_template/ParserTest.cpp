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

#include "ParserTest.hpp"

#include "oatpp-sqlite/ql_template/Parser.hpp"

namespace oatpp { namespace test { namespace sqlite { namespace ql_template {

namespace {

typedef oatpp::sqlite::ql_template::Parser Parser;

}

void ParserTest::onRun() {

  {
    oatpp::String text = "";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(result == text);

  }

  {
    oatpp::String text = "SELECT * FROM my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(result == text);

  }

  {
    oatpp::String text = "SELECT <!! * !!> FROM my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT  *  FROM my_table;");
    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 7);
      OATPP_ASSERT(s.size == 3);
    }
  }

  {
    oatpp::String text = "<!!SELECT * FROM my_table;!!>";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT * FROM my_table;");
    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 0);
      OATPP_ASSERT(s.size == result->size());
    }
  }

  {
    oatpp::String text = "SELECT <!! * !!> FROM!!> my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT  *  FROM!!> my_table;");
    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 7);
      OATPP_ASSERT(s.size == 3);
    }
  }

  {
    oatpp::String text = "SELECT <!! <!!* !!> FROM!!> my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT  <!!*  FROM!!> my_table;");
    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 7);
      OATPP_ASSERT(s.size == 6);
    }
  }

  {
    oatpp::String text = "SELECT < !! * !! > FROM!!> my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 0);
    OATPP_ASSERT(result == text);
  }

  {
    oatpp::String text = "SELECT <!! schedule[1:2][2] !!> FROM <!!my_table!!>;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 2);
    OATPP_ASSERT(result == "SELECT  schedule[1:2][2]  FROM my_table;");

    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 7);
      OATPP_ASSERT(s.size == 18);
    }

    {
      const auto& s = sections[1];
      OATPP_ASSERT(s.position == 31);
      OATPP_ASSERT(s.size == 8);
    }

  }

  {
    oatpp::String text = "SELECT <!! * '!!>' FROM!!> my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT  * '!!>' FROM my_table;");

    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 7);
      OATPP_ASSERT(s.size == 13);
    }

  }

  {
    oatpp::String text = "SELECT '<!!' * <!! FROM!!> my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT '<!!' *  FROM my_table;");

    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 15);
      OATPP_ASSERT(s.size == 5);
    }

  }

  {
    oatpp::String text = "SELECT * <!!!!> FROM my_table;";
    std::vector<Parser::CleanSection> sections;
    auto result = Parser::preprocess(text, sections);

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(sections.size() == 1);
    OATPP_ASSERT(result == "SELECT *  FROM my_table;");

    {
      const auto& s = sections[0];
      OATPP_ASSERT(s.position == 9);
      OATPP_ASSERT(s.size == 0);
    }

  }

  {
    oatpp::String text = "SELECT <!! name::text !!> FROM my_table WHERE id=:id;";
    std::vector<Parser::CleanSection> sections;
    auto temp = Parser::parseTemplate(text);
    auto result = temp.format("<val>");

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(result == "SELECT  name::text  FROM my_table WHERE id=<val>;");
  }

  {
    oatpp::String text = "SELECT <!! name::text !!> FROM my_table WHERE <!! id=:id !!>;";
    std::vector<Parser::CleanSection> sections;
    auto temp = Parser::parseTemplate(text);
    auto result = temp.format("<val>");

    OATPP_LOGD(TAG, "--- case ---");
    OATPP_LOGD(TAG, "sql='%s'", text->c_str());
    OATPP_LOGD(TAG, "res='%s'", result->c_str());

    OATPP_ASSERT(result == "SELECT  name::text  FROM my_table WHERE  id=:id ;");
  }

}

}}}}
