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

#include "Deserializer.hpp"
#include "oatpp-sqlite/Types.hpp"

namespace oatpp { namespace sqlite { namespace mapping {

Deserializer::InData::InData(sqlite3_stmt* pStmt, int pCol) {
  stmt = pStmt;
  col = pCol;
  oid = sqlite3_column_type(stmt, col);
  isNull = (oid == SQLITE_NULL);
}

Deserializer::Deserializer() {

  m_methods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

  setDeserializerMethod(data::mapping::type::__class::String::CLASS_ID, &Deserializer::deserializeString);
  setDeserializerMethod(data::mapping::type::__class::Any::CLASS_ID, &Deserializer::deserializeAny);

  setDeserializerMethod(data::mapping::type::__class::Int8::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int8>);
  setDeserializerMethod(data::mapping::type::__class::UInt8::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt8>);

  setDeserializerMethod(data::mapping::type::__class::Int16::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int16>);
  setDeserializerMethod(data::mapping::type::__class::UInt16::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt16>);

  setDeserializerMethod(data::mapping::type::__class::Int32::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int32>);
  setDeserializerMethod(data::mapping::type::__class::UInt32::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt32>);

  setDeserializerMethod(data::mapping::type::__class::Int64::CLASS_ID, &Deserializer::deserializeInt<oatpp::Int64>);
  setDeserializerMethod(data::mapping::type::__class::UInt64::CLASS_ID, &Deserializer::deserializeInt<oatpp::UInt64>);

  setDeserializerMethod(data::mapping::type::__class::Float32::CLASS_ID, &Deserializer::deserializeFloat32);
  setDeserializerMethod(data::mapping::type::__class::Float64::CLASS_ID, &Deserializer::deserializeFloat64);
  setDeserializerMethod(data::mapping::type::__class::Boolean::CLASS_ID, nullptr);

  setDeserializerMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractEnum::CLASS_ID, nullptr);

  setDeserializerMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, nullptr);

  setDeserializerMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, nullptr);
  setDeserializerMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, nullptr);

}

void Deserializer::setDeserializerMethod(const data::mapping::type::ClassId& classId, DeserializerMethod method) {
  const v_uint32 id = classId.id;
  if(id < m_methods.size()) {
    m_methods[id] = method;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::setDeserializerMethod()]: Error. Unknown classId");
  }
}

oatpp::Void Deserializer::deserialize(const InData& data, const Type* type) const {

  auto id = type->classId.id;
  auto& method = m_methods[id];

  if(method) {
    return (*method)(this, data, type);
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserialize()]: "
                           "Error. No deserialize method for type '" + std::string(type->classId.name) + "'");

}

v_int64 Deserializer::deInt(const InData& data) {
  switch(data.oid) {
    case SQLITE_INTEGER: return sqlite3_column_int64(data.stmt, data.col);
  }
  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deInt()]: Error. Unknown OID.");
}

oatpp::Void Deserializer::deserializeString(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::String();
  }

  switch(data.oid) {
    case SQLITE3_TEXT: {
      auto ptr = (const char*) sqlite3_column_text(data.stmt, data.col);
      auto size = sqlite3_column_bytes(data.stmt, data.col);
      return oatpp::String(ptr, size, true);
    }
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeString()]: Error. Unknown OID.");

}

oatpp::Void Deserializer::deserializeFloat32(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float32();
  }

  return oatpp::Float32(sqlite3_column_double(data.stmt, data.col));

}

oatpp::Void Deserializer::deserializeFloat64(const Deserializer* _this, const InData& data, const Type* type) {

  (void) _this;
  (void) type;

  if(data.isNull) {
    return oatpp::Float64();
  }

  return oatpp::Float64(sqlite3_column_double(data.stmt, data.col));

}

oatpp::Void Deserializer::deserializeAny(const Deserializer* _this, const InData& data, const Type* type) {
  (void) type;
  const Type* valueType = _this->m_typeMapper.getOidType(data.oid);
  if(valueType == nullptr) {
    throw std::runtime_error("[oatpp::sqlite::mapping::Deserializer::deserializeAny()]: Error. Unknown OID.");
  }
  auto value = _this->deserialize(data, valueType);
  auto anyHandle = std::make_shared<data::mapping::type::AnyHandle>(value.getPtr(), value.valueType);
  return oatpp::Void(anyHandle, Any::Class::getType());
}

}}}
