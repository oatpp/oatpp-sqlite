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

#ifndef oatpp_sqlite_mapping_ResultMapper_hpp
#define oatpp_sqlite_mapping_ResultMapper_hpp

#include "Deserializer.hpp"
#include "oatpp/core/data/mapping/TypeResolver.hpp"
#include "oatpp/core/Types.hpp"

#include "sqlite/sqlite3.h"

namespace oatpp { namespace sqlite { namespace mapping {

class ResultMapper {
public:

  struct ResultData {

    ResultData(sqlite3_stmt* pStmt, const std::shared_ptr<const data::mapping::TypeResolver>& pTypeResolver);

    sqlite3_stmt* stmt;

    std::shared_ptr<const data::mapping::TypeResolver> typeResolver;

    std::vector<oatpp::String> colNames;
    std::unordered_map<data::share::StringKeyLabel, v_int32> colIndices;
    v_int64 colCount;
    v_int64 rowIndex;

    bool hasMore;
    bool isSuccess;

  public:

    void next();

  };

private:
  typedef oatpp::data::mapping::type::Type Type;
  typedef oatpp::Void (*ReadOneRowMethod)(ResultMapper*, ResultData*, const Type*);
  typedef oatpp::Void (*ReadRowsMethod)(ResultMapper*, ResultData*, const Type*, v_int64);
private:

  template<class Collection>
  static oatpp::Void readRowAsList(ResultMapper* _this, ResultData* dbData, const Type* type) {

    auto polymorphicDispatcher = static_cast<const typename Collection::Class::PolymorphicDispatcher*>(type->polymorphicDispatcher);
    auto listWrapper = polymorphicDispatcher->createObject();

    const auto& list = listWrapper.template staticCast<Collection>();

    const Type* itemType = *type->params.begin();

    for(v_int32 i = 0; i < dbData->colCount; i ++) {
      mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
      polymorphicDispatcher->addPolymorphicItem(listWrapper, _this->m_deserializer.deserialize(inData, itemType));
    }

    return listWrapper;

  }

  template<class Collection>
  static oatpp::Void readRowAsKeyValue(ResultMapper* _this, ResultData* dbData, const Type* type) {

    auto polymorphicDispatcher = static_cast<const typename Collection::Class::PolymorphicDispatcher*>(type->polymorphicDispatcher);
    auto mapWrapper = polymorphicDispatcher->createObject();
    const auto& map = mapWrapper.template staticCast<Collection>();

    auto it = type->params.begin();
    const Type* keyType = *it ++;
    if(keyType->classId.id != oatpp::data::mapping::type::__class::String::CLASS_ID.id){
      throw std::runtime_error("[oatpp::sqlite::mapping::ResultMapper::readRowAsKeyValue()]: Invalid map key. Key should be String");
    }

    const Type* valueType = *it;
    for(v_int32 i = 0; i < dbData->colCount; i ++) {
      mapping::Deserializer::InData inData(dbData->stmt, i, dbData->typeResolver);
      polymorphicDispatcher->addPolymorphicItem(mapWrapper, dbData->colNames[i], _this->m_deserializer.deserialize(inData, valueType));
    }

    return mapWrapper;
  }

  static oatpp::Void readRowAsObject(ResultMapper* _this, ResultData* dbData, const Type* type);

  template<class Collection>
  static oatpp::Void readRowsAsList(ResultMapper* _this, ResultData* dbData, const Type* type, v_int64 count) {

    auto polymorphicDispatcher = static_cast<const typename Collection::Class::PolymorphicDispatcher*>(type->polymorphicDispatcher);
    auto listWrapper = polymorphicDispatcher->createObject();
    const auto& list = listWrapper.template staticCast<Collection>();

    if(count != 0) {

      const Type *itemType = *type->params.begin();

      v_int64 counter = 0;
      while (dbData->hasMore) {
        polymorphicDispatcher->addPolymorphicItem(listWrapper, _this->readOneRow(dbData, itemType));
        ++dbData->rowIndex;
        dbData->next();
        if (count > 0) {
          ++counter;
          if (counter == count) {
            break;
          }
        }
      }

    }

    return listWrapper;

  }

private:
  Deserializer m_deserializer;
  std::vector<ReadOneRowMethod> m_readOneRowMethods;
  std::vector<ReadRowsMethod> m_readRowsMethods;
public:

  ResultMapper();

  void setReadOneRowMethod(const data::mapping::type::ClassId& classId, ReadOneRowMethod method);
  void setReadRowsMethod(const data::mapping::type::ClassId& classId, ReadRowsMethod method);

  oatpp::Void readOneRow(ResultData* dbData, const Type* type);
  oatpp::Void readRows(ResultData* dbData, const Type* type, v_int64 count);

};

}}}

#endif //oatpp_sqlite_mapping_ResultMapper_hpp
