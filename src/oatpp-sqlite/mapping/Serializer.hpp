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

#ifndef oatpp_sqlite_mapping_Serializer_hpp
#define oatpp_sqlite_mapping_Serializer_hpp

#include "oatpp/core/Types.hpp"
#include "sqlite/sqlite3.h"

namespace oatpp { namespace sqlite { namespace mapping {

class Serializer {
public:
  typedef void (*SerializerMethod)(sqlite3_stmt*, v_uint32, const oatpp::Void&);
private:
  std::vector<SerializerMethod> m_methods;
public:

  Serializer();

  void setSerializerMethod(const data::mapping::type::ClassId& classId, SerializerMethod method);

  void serialize(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) const;

public:

  static void serializeString(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeInt8(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeUInt8(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeInt16(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeUInt16(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeInt32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeUInt32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeInt64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeUInt64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeFloat32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

  static void serializeFloat64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph);

};

}}}

#endif // oatpp_sqlite_mapping_Serializer_hpp
