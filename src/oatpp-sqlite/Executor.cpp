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
#include "Types.hpp"

#include "oatpp/orm/Transaction.hpp"
#include "oatpp/core/data/stream/ChunkedBuffer.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include <vector>

namespace oatpp { namespace sqlite {

namespace {

#include OATPP_CODEGEN_BEGIN(DTO)

class VersionRow : public oatpp::DTO {

  DTO_INIT(VersionRow, DTO);

  DTO_FIELD(Int64, version);

};

#include OATPP_CODEGEN_END(DTO)

}

Executor::Executor(const std::shared_ptr<provider::Provider<Connection>>& connectionProvider)
  : m_connectionProvider(connectionProvider)
  , m_resultMapper(std::make_shared<mapping::ResultMapper>())
{
  m_objectTraverser.addKnownTypes({
    Blob::Class::CLASS_ID
  });
}

data::share::StringTemplate Executor::parseQueryTemplate(const oatpp::String& name,
                                                         const oatpp::String& text,
                                                         const ParamsTypeMap& paramsTypeMap,
                                                         bool prepare)
{

  (void) paramsTypeMap;

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

Executor::DtoParam Executor::paramNameAsDtoParam(const oatpp::String& paramName) {

  parser::Caret caret(paramName);
  auto nameLabel = caret.putLabel();
  if(caret.findChar('.') && caret.getPosition() < caret.getDataSize() - 1) {

    DtoParam result;
    result.name = nameLabel.toString();

    do {

      caret.inc();
      auto label = caret.putLabel();
      caret.findChar('.');
      result.propertyPath.push_back(label.std_str());

    } while (caret.getPosition() < caret.getDataSize());

    return result;

  }

  return {};

}

void Executor::bindParams(sqlite3_stmt* stmt,
                          const StringTemplate& queryTemplate,
                          const std::unordered_map<oatpp::String, oatpp::Void>& params)
{

  auto count = queryTemplate.getTemplateVariables().size();

  for(v_uint32 i = 0; i < count; i ++) {
    const auto& var = queryTemplate.getTemplateVariables()[i];
    auto it = params.find(var.name);

    if(it != params.end()) {
      m_serializer.serialize(stmt, i + 1, it->second);
      continue;
    }

    auto dtoParam = paramNameAsDtoParam(var.name);
    if(dtoParam.name) {
      it = params.find(dtoParam.name);
      if(it != params.end() && it->second.valueType->classId.id == data::mapping::type::__class::AbstractObject::CLASS_ID.id) {
        auto value = m_objectTraverser.findPropertyValue(it->second, dtoParam.propertyPath, {});
        if(value.valueType->classId.id != oatpp::Void::Class::CLASS_ID.id) {
          m_serializer.serialize(stmt, i + 1, value);
          continue;
        }
      }
    }

    throw std::runtime_error("[oatpp::sqlite::Executor::bindParams()]: "
                             "Error. Parameter not found " + var.name->std_str());

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

std::shared_ptr<orm::QueryResult> Executor::exec(const oatpp::String& statement,
                                                 const std::shared_ptr<orm::Connection>& connection)
{

  std::shared_ptr<orm::Connection> conn = connection;
  if(!conn) {
    conn = getConnection();
  }

  auto pgConnection = std::static_pointer_cast<sqlite::Connection>(conn);
  sqlite3_stmt* stmt;
  auto res = sqlite3_prepare_v2(pgConnection->getHandle(), statement->c_str(), -1, &stmt, nullptr);
  return std::make_shared<QueryResult>(stmt, pgConnection, m_connectionProvider, m_resultMapper);

}

std::shared_ptr<orm::QueryResult> Executor::begin(const std::shared_ptr<orm::Connection>& connection) {
  return exec("BEGIN", connection);
}

std::shared_ptr<orm::QueryResult> Executor::commit(const std::shared_ptr<orm::Connection>& connection) {
  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::commit()]: "
                             "Error. Can't COMMIT - NULL connection.");
  }
  return exec("COMMIT", connection);
}

std::shared_ptr<orm::QueryResult> Executor::rollback(const std::shared_ptr<orm::Connection>& connection) {
  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::commit()]: "
                             "Error. Can't ROLLBACK - NULL connection.");
  }
  return exec("ROLLBACK", connection);
}

oatpp::String Executor::getSchemaVersionTableName(const oatpp::String& suffix) {
  data::stream::BufferOutputStream stream;
  stream << "oatpp_schema_version";
  if (suffix && suffix->getSize() > 0) {
    stream << "_" << suffix;
  }
  return stream.toString();
}

std::shared_ptr<orm::QueryResult> Executor::updateSchemaVersion(v_int64 newVersion,
                                                                const oatpp::String& suffix,
                                                                const std::shared_ptr<orm::Connection>& connection)
{
  data::stream::BufferOutputStream stream;
  stream
  << "UPDATE "
  << getSchemaVersionTableName(suffix) << " "
  << "SET version=" << newVersion << ";";
  return exec(stream.toString(), connection);
}

v_int64 Executor::getSchemaVersion(const oatpp::String& suffix,
                                   const std::shared_ptr<orm::Connection>& connection)
{

  std::shared_ptr<orm::QueryResult> result;

  {
    data::stream::BufferOutputStream stream;
    stream << "CREATE TABLE IF NOT EXISTS " << getSchemaVersionTableName(suffix) << " (version BIGINT)";
    result = exec(stream.toString(), connection);
    if(!result->isSuccess()) {
      throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                               "Error. Can't create schema version table. " + result->getErrorMessage()->std_str());
    }
  }

  data::stream::BufferOutputStream stream;
  stream << "SELECT * FROM " << getSchemaVersionTableName(suffix);
  result = exec(stream.toString(), result->getConnection());
  if(!result->isSuccess()) {
    throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                             "Error. Can't get schema version. " + result->getErrorMessage()->std_str());
  }

  auto rows = result->fetch<oatpp::Vector<oatpp::Object<VersionRow>>>();

  if(rows->size() == 0) {

    stream.setCurrentPosition(0);
    stream << "INSERT INTO " << getSchemaVersionTableName(suffix) << " (version) VALUES (0)";
    result = exec(stream.toString(), result->getConnection());

    if(result->isSuccess()) {
      return 0;
    }

    throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                             "Error. Can't init schema version. " + result->getErrorMessage()->std_str());

  } else if(rows->size() == 1) {

    auto row = rows[0];
    if(!row->version) {
      throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                               "Error. The schema version table is corrupted - version is null.");
    }

    return row->version;

  }

  throw std::runtime_error("[oatpp::sqlite::Executor::getSchemaVersion()]: "
                           "Error. The schema version table is corrupted - multiple version rows.");

}

void Executor::migrateSchema(const oatpp::String& script,
                             v_int64 newVersion,
                             const oatpp::String& suffix,
                             const std::shared_ptr<orm::Connection>& connection)
{

  if(!script) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Script is null.");
  }

  if(!connection) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Connection is null.");
  }

  auto currVersion = getSchemaVersion(suffix, connection);
  if(newVersion <= currVersion) {
    return;
  }

  if(newVersion > currVersion + 1) {
    throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. +1 version increment is allowed only.");
  }

  if(script->getSize() == 0) {
    OATPP_LOGW("[oatpp::sqlite::Executor::migrateSchema()]", "Warning. Executing empty script for version %d", newVersion);
  }

  {

    auto nativeConnection = std::static_pointer_cast<sqlite::Connection>(connection);
    orm::Transaction transaction(this, nativeConnection);

    char* errmsg = nullptr;
    sqlite3_exec(nativeConnection->getHandle(), script->c_str(), nullptr, nullptr, &errmsg);

    if(errmsg) {
      OATPP_LOGE("[oatpp::sqlite::Executor::migrateSchema()]", "Error. Migration failed for version %d. %s", newVersion, errmsg);
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. " + std::string((const char*) errmsg));
    }

    std::shared_ptr<orm::QueryResult> result;
    result = updateSchemaVersion(newVersion, suffix, nativeConnection);

    if(!result->isSuccess() || result->hasMoreToFetch() > 0) {
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. Can't set new version.");
    }

    result = transaction.commit();
    if(!result->isSuccess()) {
      throw std::runtime_error("[oatpp::sqlite::Executor::migrateSchema()]: Error. Migration failed. Can't commit.");
    }

  }

}

}}
