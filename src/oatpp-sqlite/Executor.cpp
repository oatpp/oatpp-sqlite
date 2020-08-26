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

#include "Executor.hpp"

#include "ql_template/Parser.hpp"
#include "ql_template/TemplateValueProvider.hpp"

#include "QueryResult.hpp"

#include "oatpp/core/data/stream/ChunkedBuffer.hpp"

#include <vector>

namespace oatpp { namespace sqlite {

Executor::Executor(const std::shared_ptr<provider::Provider<Connection>>& connectionProvider)
  : m_connectionProvider(connectionProvider)
  , m_resultMapper(std::make_shared<mapping::ResultMapper>())
{}

data::share::StringTemplate Executor::parseQueryTemplate(const oatpp::String& name,
                                                         const oatpp::String& text,
                                                         const ParamsTypeMap& paramsTypeMap,
                                                         bool prepare)
{

  auto&& t = ql_template::Parser::parseTemplate(text);

  auto extra = std::make_shared<ql_template::Parser::TemplateExtra>();
  t.setExtraData(extra);

  extra->prepare = prepare;
  extra->templateName = name;
  ql_template::TemplateValueProvider valueProvider;
  extra->preparedTemplate = t.format(&valueProvider);

  return t;

}

std::shared_ptr<orm::Connection> Executor::getConnection() {
  return m_connectionProvider->get();
}

void Executor::bindParams(sqlite3_stmt* stmt,
                          const StringTemplate& queryTemplate,
                          const std::unordered_map<oatpp::String, oatpp::Void>& params)
{

  auto count = queryTemplate.getTemplateVariables().size();

  for(v_uint32 i = 0; i < count; i ++) {
    const auto& var = queryTemplate.getTemplateVariables()[i];
    auto it = params.find(var.name);
    if(it == params.end()) {
      throw std::runtime_error("[oatpp::sqlite::Executor::bindParams()]: "
                               "Error. Parameter not found " + var.name->std_str());
    }

    m_serializer.serialize(stmt, i + 1, it->second);

  }

}

std::shared_ptr<orm::QueryResult> Executor::execute(const StringTemplate& queryTemplate,
                                                    const std::unordered_map<oatpp::String, oatpp::Void>& params,
                                                    const std::shared_ptr<orm::Connection>& connection)
{

  std::shared_ptr<orm::Connection> conn = connection;
  if(!conn) {
    conn = getConnection();
  }

  auto pgConnection = std::static_pointer_cast<sqlite::Connection>(conn);

  auto extra = std::static_pointer_cast<ql_template::Parser::TemplateExtra>(queryTemplate.getExtraData());

  sqlite3_stmt* stmt = nullptr;
  auto res = sqlite3_prepare_v2(pgConnection->getHandle(),
                                extra->preparedTemplate->c_str(),
                                extra->preparedTemplate->getSize(),
                                &stmt,
                                nullptr);

  // TODO check for res

  bindParams(stmt, queryTemplate, params);

  return std::make_shared<QueryResult>(stmt, pgConnection, m_connectionProvider, m_resultMapper);

}

std::shared_ptr<orm::QueryResult> Executor::begin(const std::shared_ptr<orm::Connection>& connection) {

  std::shared_ptr<orm::Connection> conn = connection;
  if(!conn) {
    conn = getConnection();
  }

  auto pgConnection = std::static_pointer_cast<sqlite::Connection>(conn);
  sqlite3_stmt* stmt;
  auto res = sqlite3_prepare_v2(pgConnection->getHandle(), "BEGIN", -1, &stmt, nullptr);
  return std::make_shared<QueryResult>(stmt, pgConnection, m_connectionProvider, m_resultMapper);

}

std::shared_ptr<orm::QueryResult> Executor::commit(const std::shared_ptr<orm::Connection>& connection) {
  auto pgConnection = std::static_pointer_cast<sqlite::Connection>(connection);
  sqlite3_stmt* stmt;
  auto res = sqlite3_prepare_v2(pgConnection->getHandle(), "COMMIT", -1, &stmt, nullptr);
  return std::make_shared<QueryResult>(stmt, pgConnection, m_connectionProvider, m_resultMapper);
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const std::shared_ptr<orm::Connection>& connection) {
  auto pgConnection = std::static_pointer_cast<sqlite::Connection>(connection);
  sqlite3_stmt* stmt;
  auto res = sqlite3_prepare_v2(pgConnection->getHandle(), "ROLLBACK", -1, &stmt, nullptr);
  return std::make_shared<QueryResult>(stmt, pgConnection, m_connectionProvider, m_resultMapper);
}

}}
