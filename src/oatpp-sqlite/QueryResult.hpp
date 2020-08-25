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

#ifndef oatpp_sqlite_QueryResult_hpp
#define oatpp_sqlite_QueryResult_hpp

#include "ConnectionProvider.hpp"
#include "mapping/Deserializer.hpp"
#include "mapping/ResultMapper.hpp"
#include "oatpp/orm/QueryResult.hpp"

namespace oatpp { namespace sqlite {

class QueryResult : public orm::QueryResult {
private:
  static constexpr v_int32 TYPE_ERROR = 0;
  static constexpr v_int32 TYPE_COMMAND = 1;
  static constexpr v_int32 TYPE_TUPLES = 2;
private:
  sqlite3_stmt* m_stmt;
  std::shared_ptr<Connection> m_connection;
  std::shared_ptr<provider::Provider<Connection>> m_connectionProvider;
  std::shared_ptr<mapping::ResultMapper> m_resultMapper;
  mapping::ResultMapper::ResultData m_resultData;
private:
  mapping::Deserializer m_deserializer;
public:

  QueryResult(sqlite3_stmt* stmt,
              const std::shared_ptr<Connection>& connection,
              const std::shared_ptr<provider::Provider<Connection>>& connectionProvider,
              const std::shared_ptr<mapping::ResultMapper>& resultMapper);

  ~QueryResult();

  std::shared_ptr<orm::Connection> getConnection() const override;

  bool isSuccess() const override;

  oatpp::String getErrorMessage() const override;

  v_int64 getPosition() const override;

  v_int64 getKnownCount() const override;

  bool hasMoreToFetch() const override;

  oatpp::Void fetch(const oatpp::Type* const type, v_int64 count) override;

};

}}

#endif //oatpp_sqlite_QueryResult_hpp
