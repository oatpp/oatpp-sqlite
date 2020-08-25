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

#include "Uuid.hpp"
#include "oatpp/encoding/Hex.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

namespace oatpp { namespace sqlite { namespace mapping { namespace type {

UuidObject::UuidObject(v_char8 data[DATA_SIZE]) {
  std::memcpy(m_data, data, DATA_SIZE);
}

UuidObject::UuidObject(const oatpp::String& text) {
  data::stream::BufferOutputStream stream(16);
  encoding::Hex::decode(&stream, text->getData(), text->getSize(), true);
  if(stream.getCurrentPosition() != 16) {
    throw std::runtime_error("[oatpp::sqlite::mapping::type::UuidObject::UuidObject()]: Error. Invalid string.");
  }
  std::memcpy(m_data, stream.getData(), DATA_SIZE);
}

const p_char8 UuidObject::getData() const {
  return (const p_char8) m_data;
}

v_buff_size UuidObject::getSize() const {
  return DATA_SIZE;
}

oatpp::String UuidObject::toString() const {
  auto alphabet = encoding::Hex::ALPHABET_LOWER;
  data::stream::BufferOutputStream stream(36);
  encoding::Hex::encode(&stream, &m_data[0], 4, alphabet);
  stream.writeCharSimple('-');
  encoding::Hex::encode(&stream, &m_data[4], 2, alphabet);
  stream.writeCharSimple('-');
  encoding::Hex::encode(&stream, &m_data[6], 2, alphabet);
  stream.writeCharSimple('-');
  encoding::Hex::encode(&stream, &m_data[8], 2, alphabet);
  stream.writeCharSimple('-');
  encoding::Hex::encode(&stream, &m_data[10], 6, alphabet);
  return stream.toString();
}

bool UuidObject::operator==(const UuidObject &other) const {
  return std::memcmp(m_data, other.m_data, DATA_SIZE) == 0;
}

bool UuidObject::operator!=(const UuidObject &other) const {
  return !operator==(other);
}

namespace __class {

  const oatpp::ClassId Uuid::CLASS_ID("oatpp::sqlite::Uuid");

  oatpp::Type* Uuid::getType() {
    static Type type(
      CLASS_ID, nullptr, nullptr, nullptr, nullptr,
      {
        {"sqlite", new Inter()}
      }
    );
    return &type;
  }

}

}}}}
