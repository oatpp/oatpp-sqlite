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

#include "ResultMapper.hpp"

namespace oatpp { namespace sqlite { namespace mapping {

ResultMapper::ResultData::ResultData(sqlite3_stmt* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver)
  : stmt(pStmt)
  , typeResolver(pTypeResolver)
{

  next();
  rowIndex = 0;

  {
    colCount = sqlite3_column_count(stmt);
    for (v_int32 i = 0; i < colCount; i++) {
      oatpp::String colName = (const char*) sqlite3_column_name(stmt, i);
      colNames.push_back(colName);
      colIndices.insert({colName, i});
    }
  }

}

void ResultMapper::ResultData::next() {

  auto res = sqlite3_step(stmt);

  switch(res) {

    case SQLITE_ROW: {
      hasMore = true;
      isSuccess = true;
      break;
    }

    case SQLITE_DONE: {
      hasMore = false;
      isSuccess = true;
      break;
    };

    default: {
      hasMore = false;
      isSuccess = false;
    }

  }

}

ResultMapper::ResultMapper() {

  {
    m_readOneRowMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    setReadOneRowMethod(data::mapping::type::__class::AbstractObject::CLASS_ID, &ResultMapper::readRowAsObject);

    setReadOneRowMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readRowAsList<oatpp::AbstractVector>);
    setReadOneRowMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readRowAsList<oatpp::AbstractList>);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, &ResultMapper::readRowAsList<oatpp::AbstractUnorderedSet>);

    setReadOneRowMethod(data::mapping::type::__class::AbstractPairList::CLASS_ID, &ResultMapper::readRowAsKeyValue<oatpp::AbstractFields>);
    setReadOneRowMethod(data::mapping::type::__class::AbstractUnorderedMap::CLASS_ID, &ResultMapper::readRowAsKeyValue<oatpp::AbstractUnorderedFields>);
  }

  {
    m_readRowsMethods.resize(data::mapping::type::ClassId::getClassCount(), nullptr);

    setReadRowsMethod(data::mapping::type::__class::AbstractVector::CLASS_ID, &ResultMapper::readRowsAsList<oatpp::AbstractVector>);
    setReadRowsMethod(data::mapping::type::__class::AbstractList::CLASS_ID, &ResultMapper::readRowsAsList<oatpp::AbstractList>);
    setReadRowsMethod(data::mapping::type::__class::AbstractUnorderedSet::CLASS_ID, &ResultMapper::readRowsAsList<oatpp::AbstractUnorderedSet>);

  }

}

void ResultMapper::setReadOneRowMethod(const data::mapping::type::ClassId& classId, ReadOneRowMethod method) {
  const v_uint32 id = classId.id;
  if(id < m_readOneRowMethods.size()) {
    m_readOneRowMethods[id] = method;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::setReadOneRowMethod()]: Error. Unknown classId");
  }
}

void ResultMapper::setReadRowsMethod(const data::mapping::type::ClassId& classId, ReadRowsMethod method) {
  const v_uint32 id = classId.id;
  if(id < m_readRowsMethods.size()) {
    m_readRowsMethods[id] = method;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::setReadRowsMethod()]: Error. Unknown classId");
  }
}

oatpp::Void ResultMapper::readRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type) {

  auto dispatcher = static_cast<const data::mapping::type::__class::AbstractObject::PolymorphicDispatcher*>(type->polymorphicDispatcher);
  auto object = dispatcher->createObject();
  const auto& fieldsMap = dispatcher->getProperties()->getMap();

  for(v_int32 i = 0; i < dbData->colCount; i ++) {

    auto it = fieldsMap.find(dbData->colNames[i]->std_str());

    if(it != fieldsMap.end()) {
      auto field = it->second;
      mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
      field->set(static_cast<oatpp::BaseObject*>(object.get()),
                 _this->m_deserializer.deserialize(inData, field->type));
    } else {
      OATPP_LOGE("[oatpp::sqlite::mapping::ResultMapper::readRowAsObject]",
                 "Error. The object of type '%s' has no field to map column '%s'.",
                 type->nameQualifier, dbData->colNames[i]->c_str());
      throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readRowAsObject]: Error. "
                               "The object of type " + std::string(type->nameQualifier) +
                               " has no field to map column " + dbData->colNames[i]->std_str()  + ".");
    }

  }

  return object;

}

oatpp::Void ResultMapper::readOneRow(ResultData* dbData, const Type* type) {

  auto id = type->classId.id;
  auto& method = m_readOneRowMethods[id];

  if(method) {
    return (*method)(this, dbData, type);
  }

  auto* interpretation = type->findInterpretation(dbData->typeResolver->getEnabledInterpretations());
  if(interpretation) {
    return interpretation->fromInterpretation(readOneRow(dbData, interpretation->getInterpretationType()));
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readOneRow()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are "
                           "oatpp::Vector, "
                           "oatpp::List, "
                           "oatpp::UnorderedSet, "
                           "oatpp::Fields, "
                           "oatpp::UnorderedFields, "
                           "oatpp::Object");

}

oatpp::Void ResultMapper::readRows(ResultData* dbData, const Type* type, v_int64 count) {

  auto id = type->classId.id;
  auto& method = m_readRowsMethods[id];

  if(method) {
    return (*method)(this, dbData, type, count);
  }

  throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readRows()]: "
                           "Error. Invalid result container type. "
                           "Allowed types are oatpp::Vector, oatpp::List, oatpp::UnorderedSet");

}

}}}
