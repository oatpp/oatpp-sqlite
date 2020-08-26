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

#include "Serializer.hpp"

#include "oatpp-sqlite/Types.hpp"

#if defined(WIN32) || defined(_WIN32)
  #include <WinSock2.h>
#else
  #include <arpa/inet.h>
#endif

namespace oatpp { namespace sqlite { namespace mapping {

Serializer::Serializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setSerializerMethod(data::mapping::type::__class::String::CLASS_ID, &Serializer::serializeString);
  setSerializerMethod(data::mapping::type::__class::Any::CLASS_ID, nullptr);

  setSerializerMethod(data::mapping::type::__class::Int8::CLASS_ID, &Serializer::serializeInt8);
  setSerializerMethod(data::mapping::type::__class::UInt8::CLASS_ID, &Serializer::serializeUInt8);

  setSerializerMethod(data::mapping::type::__class::Int16::CLASS_ID, &Serializer::serializeInt16);
  setSerializerMethod(data::mapping::type::__class::UInt16::CLASS_ID, &Serializer::serializeUInt16);

  setSerializerMethod(data::mapping::type::__class::Int32::CLASS_ID, &Serializer::serializeInt32);
  setSerializerMethod(data::mapping::type::__class::UInt32::CLASS_ID, &Serializer::serializeUInt32);

  setSerializerMethod(data::mapping::type::__class::Int64::CLASS_ID, &Serializer::serializeInt64);
  setSerializerMethod(data::mapping::type::__class::UInt64::CLASS_ID, &Serializer::serializeUInt64);

  setSerializerMethod(data::mapping::type::__class::Float32::CLASS_ID, &Serializer::serializeFloat32);
  setSerializerMethod(data::mapping::type::__class::Float64::CLASS_ID, &Serializer::serializeFloat64);
  setSerializerMethod(data::mapping::type::__class::Boolean::CLASS_ID, nullptr);

  setSerializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractEnum::CLASS_ID, nullptr);

  setSerializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractList::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, nullptr);

  setSerializerMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, nullptr);
  setSerializerMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, nullptr);

}

void Serializer::setSerializerMethod(const data::mapping::type::ClassId& classId, SerializerMethod method) {
  const v_uint32 id = classId.id;
  if(id < m_methods.size()) {
    m_methods[id] = method;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::Serializer::setSerializerMethod()]: Error. Unknown classId");
  }
}

void Serializer::serialize(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) const {
  auto id = polymorph.valueType->classId.id;
  auto& method = m_methods[id];
  if(method) {
    (*method)(stmt, paramIndex, polymorph);
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::Serializer::serialize()]: "
                             "Error. No serialize method for type '" + std::string(polymorph.valueType->classId.name) +
                             "'");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serializer functions

void Serializer::serializeString(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    base::StrBuffer *buff = static_cast<base::StrBuffer *>(polymorph.get());
    sqlite3_bind_text(stmt, paramIndex, buff->c_str(), buff->getSize(), nullptr);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeInt8(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int8>();
    sqlite3_bind_int(stmt, paramIndex, (int) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeUInt8(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt8>();
    sqlite3_bind_int(stmt, paramIndex, (int) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeInt16(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int16>();
    sqlite3_bind_int(stmt, paramIndex, (int) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeUInt16(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt16>();
    sqlite3_bind_int(stmt, paramIndex, (int) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeInt32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int32>();
    sqlite3_bind_int(stmt, paramIndex, (int) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeUInt32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt32>();
    sqlite3_bind_int64(stmt, paramIndex, (sqlite3_int64) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeInt64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int64>();
    sqlite3_bind_int64(stmt, paramIndex, (sqlite3_int64) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeUInt64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt64>();
    sqlite3_bind_int64(stmt, paramIndex, (sqlite3_int64) *v);
  } else {
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeFloat32(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Float32>();
    sqlite3_bind_double(stmt, paramIndex, *v);
  } else{
    sqlite3_bind_null(stmt, paramIndex);
  }
}

void Serializer::serializeFloat64(sqlite3_stmt* stmt, v_uint32 paramIndex, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Float64>();
    sqlite3_bind_double(stmt, paramIndex, *v);
  } else{
    sqlite3_bind_null(stmt, paramIndex);
  }
}

}}}
