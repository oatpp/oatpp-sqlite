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

  ////

  setSerializerMethod(sqlite::mapping::type::__class::Uuid::CLASS_ID, &Serializer::serializeUuid);

}

void Serializer::setSerializerMethod(const data::mapping::type::ClassId& classId, SerializerMethod method) {
  const v_uint32 id = classId.id;
  if(id < m_methods.size()) {
    m_methods[id] = method;
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::Serializer::setSerializerMethod()]: Error. Unknown classId");
  }
}

void Serializer::serialize(OutputData& outData, const oatpp::Void& polymorph) const {
  auto id = polymorph.valueType->classId.id;
  auto& method = m_methods[id];
  if(method) {
    (*method)(outData, polymorph);
  } else {
    throw std::runtime_error("[oatpp::sqlite::mapping::Serializer::serialize()]: "
                             "Error. No serialize method for type '" + std::string(polymorph.valueType->classId.name) +
                             "'");
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serializer utility functions

void Serializer::serNull(OutputData& outData) {
  outData.dataBuffer.reset();
  outData.data = nullptr;
  outData.dataSize = 0;
  outData.dataFormat = 1;
}

void Serializer::serInt2(OutputData& outData, v_int16 value) {
  outData.dataBuffer.reset(new char[2]);
  outData.data = outData.dataBuffer.get();
  outData.dataSize = 2;
  outData.dataFormat = 1;

  *((p_int16) outData.data) = htons(value);
}

void Serializer::serInt4(OutputData& outData, v_int32 value) {
  outData.dataBuffer.reset(new char[4]);
  outData.data = outData.dataBuffer.get();
  outData.dataSize = 4;
  outData.dataFormat = 1;

  *((p_int32) outData.data) = htonl(value);
}

void Serializer::serInt8(OutputData& outData, v_int64 value) {
  outData.dataBuffer.reset(new char[8]);
  outData.data = outData.dataBuffer.get();
  outData.dataSize = 8;
  outData.dataFormat = 1;

  *((p_int32) (outData.data + 0)) = htonl(value >> 32);
  *((p_int32) (outData.data + 4)) = htonl(value & 0xFFFFFFFF);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Serializer functions

void Serializer::serializeString(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    base::StrBuffer *buff = static_cast<base::StrBuffer *>(polymorph.get());
    outData.data = buff->c_str();
    outData.dataSize = buff->getSize();
    outData.dataFormat = 1;
  } else {
    serNull(outData);
  }
}

void Serializer::serializeInt8(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int8>();
    serInt2(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeUInt8(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt8>();
    serInt2(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeInt16(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int16>();
    serInt2(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeUInt16(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt16>();
    serInt4(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeInt32(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int32>();
    serInt4(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeUInt32(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::UInt32>();
    serInt8(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeInt64(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Int64>();
    serInt8(outData, *v);
  } else {
    serNull(outData);
  }
}

void Serializer::serializeUInt64(OutputData& outData, const oatpp::Void& polymorph) {
  serNull(outData);
}

void Serializer::serializeFloat32(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Float32>();
    serInt4(outData, *((p_int32) v.get()));
  } else{
    serNull(outData);
  }
}

void Serializer::serializeFloat64(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<oatpp::Float64>();
    serInt8(outData, *((p_int64) v.get()));
  } else{
    serNull(outData);
  }
}

void Serializer::serializeUuid(OutputData& outData, const oatpp::Void& polymorph) {
  if(polymorph) {
    auto v = polymorph.staticCast<sqlite::Uuid>();
    outData.data = (const char*) v->getData();
    outData.dataSize = v->getSize();
    outData.dataFormat = 1;
  } else{
    serNull(outData);
  }
}

}}}
