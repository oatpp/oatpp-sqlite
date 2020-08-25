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

#include "TypeMapper.hpp"

#include "oatpp-sqlite/Types.hpp"

#include "sqlite/sqlite3.h"

namespace oatpp { namespace sqlite { namespace mapping {

TypeMapper::TypeMapper() {

  {
    m_oids.resize(data::mapping::type::ClassId::getClassCount(), 0);

    setTypeOid(data::mapping::type::__class::String::CLASS_ID, SQLITE3_TEXT);
    setTypeOid(data::mapping::type::__class::Any::CLASS_ID, 0);

    setTypeOid(data::mapping::type::__class::Int8::CLASS_ID, SQLITE_INTEGER);
    setTypeOid(data::mapping::type::__class::UInt8::CLASS_ID, SQLITE_INTEGER);

    setTypeOid(data::mapping::type::__class::Int16::CLASS_ID, SQLITE_INTEGER);
    setTypeOid(data::mapping::type::__class::UInt16::CLASS_ID, SQLITE_INTEGER);

    setTypeOid(data::mapping::type::__class::Int32::CLASS_ID, SQLITE_INTEGER);
    setTypeOid(data::mapping::type::__class::UInt32::CLASS_ID, SQLITE_INTEGER);

    setTypeOid(data::mapping::type::__class::Int64::CLASS_ID, SQLITE_INTEGER);
    setTypeOid(data::mapping::type::__class::UInt64::CLASS_ID, SQLITE_INTEGER);

    setTypeOid(data::mapping::type::__class::Float32::CLASS_ID, SQLITE_FLOAT);
    setTypeOid(data::mapping::type::__class::Float64::CLASS_ID, SQLITE_FLOAT);
    setTypeOid(data::mapping::type::__class::Boolean::CLASS_ID, SQLITE_INTEGER);

    setTypeOid(data::mapping::type::__class::AbstractObject::CLASS_ID, 0);
    setTypeOid(data::mapping::type::__class::AbstractEnum::CLASS_ID, 0);

    setTypeOid(data::mapping::type::__class::AbstractVector::CLASS_ID, 0);
    setTypeOid(data::mapping::type::__class::AbstractList::CLASS_ID, 0);
    setTypeOid(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, 0);

    setTypeOid(data::mapping::type::__class::AbstractPairList::CLASS_ID, 0);
    setTypeOid(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, 0);

  }

  {
    setOidType(SQLITE3_TEXT, oatpp::String::Class::getType());
    setOidType(SQLITE_INTEGER, oatpp::Int64::Class::getType());
    setOidType(SQLITE_FLOAT, oatpp::Float64::Class::getType());
  }

}

void TypeMapper::setTypeOid(const data::mapping::type::ClassId& classId, int oid) {
  const v_uint32 id = classId.id;
  if(id < m_oids.size()) {
    m_oids[id] = oid;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::TypeMapper::setTypeOid()]: Error. Unknown classId");
  }
}

void TypeMapper::setOidType(int oid, const data::mapping::type::Type* type) {
  m_types[oid] = type;
}

int TypeMapper::getTypeOid(const data::mapping::type::Type* type) const {
  const v_uint32 id = type->classId.id;
  if(id < m_oids.size()) {
    return m_oids[id];
  }
  throw std::runtime_error("[oatpp::sqlite::mapping::TypeMapper::getTypeOid()]: Error. Unknown classId");
}

const data::mapping::type::Type* TypeMapper::getOidType(int oid) const {
  auto it = m_types.find(oid);
  if(it != m_types.end()) {
    return it->second;
  }
  return nullptr;
}

}}}
