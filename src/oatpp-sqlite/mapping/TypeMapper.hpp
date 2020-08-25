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

#ifndef oatpp_sqlite_mapping_TypeMapper_hpp
#define oatpp_sqlite_mapping_TypeMapper_hpp

#include "oatpp/core/Types.hpp"

namespace oatpp { namespace sqlite { namespace mapping {

class TypeMapper {
private:
  std::vector<int> m_oids;
  std::unordered_map<int, const data::mapping::type::Type*> m_types;
public:

  TypeMapper();

  void setTypeOid(const data::mapping::type::ClassId& classId, int oid);
  void setOidType(int oid, const data::mapping::type::Type* type);

  int getTypeOid(const data::mapping::type::Type* type) const;
  const data::mapping::type::Type* getOidType(int oid) const;

};

}}}

#endif // oatpp_sqlite_mapping_TypeMapper_hpp
