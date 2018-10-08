#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::document
{

using BytesRef = org::apache::lucene::util::BytesRef;

/** A field whose value is stored so that {@link
 *  IndexSearcher#doc} and {@link IndexReader#document IndexReader.document()}
 * will return the field and its value. */
class StoredField : public Field
{
  GET_CLASS_NAME(StoredField)

  /**
   * Type for a stored-only field.
   */
public:
  static const std::shared_ptr<FieldType> TYPE;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StoredField::StaticConstructor staticConstructor;

  /**
   * Expert: allows you to customize the {@link
   * FieldType}.
   * @param name field name
   * @param type custom {@link FieldType} for this field
   * @throws IllegalArgumentException if the field name is null.
   */
protected:
  StoredField(const std::wstring &name, std::shared_ptr<FieldType> type);

  /**
   * Expert: allows you to customize the {@link
   * FieldType}.
   * <p>NOTE: the provided byte[] is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param bytes byte array pointing to binary content (not copied)
   * @param type custom {@link FieldType} for this field
   * @throws IllegalArgumentException if the field name is null.
   */
public:
  StoredField(const std::wstring &name, std::shared_ptr<BytesRef> bytes,
              std::shared_ptr<FieldType> type);

  /**
   * Create a stored-only field with the given binary value.
   * <p>NOTE: the provided byte[] is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param value byte array pointing to binary content (not copied)
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, std::deque<char> &value);

  /**
   * Create a stored-only field with the given binary value.
   * <p>NOTE: the provided byte[] is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param value byte array pointing to binary content (not copied)
   * @param offset starting position of the byte array
   * @param length valid length of the byte array
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, std::deque<char> &value, int offset,
              int length);

  /**
   * Create a stored-only field with the given binary value.
   * <p>NOTE: the provided BytesRef is not copied so be sure
   * not to change it until you're done with this field.
   * @param name field name
   * @param value BytesRef pointing to binary content (not copied)
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, std::shared_ptr<BytesRef> value);

  /**
   * Create a stored-only field with the given string value.
   * @param name field name
   * @param value string value
   * @throws IllegalArgumentException if the field name or value is null.
   */
  StoredField(const std::wstring &name, const std::wstring &value);

  /**
   * Expert: allows you to customize the {@link
   * FieldType}.
   * @param name field name
   * @param value string value
   * @param type custom {@link FieldType} for this field
   * @throws IllegalArgumentException if the field name or value is null.
   */
  StoredField(const std::wstring &name, const std::wstring &value,
              std::shared_ptr<FieldType> type);

  // TODO: not great but maybe not a big problem?
  /**
   * Create a stored-only field with the given integer value.
   * @param name field name
   * @param value integer value
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, int value);

  /**
   * Create a stored-only field with the given float value.
   * @param name field name
   * @param value float value
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, float value);

  /**
   * Create a stored-only field with the given long value.
   * @param name field name
   * @param value long value
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, int64_t value);

  /**
   * Create a stored-only field with the given double value.
   * @param name field name
   * @param value double value
   * @throws IllegalArgumentException if the field name is null.
   */
  StoredField(const std::wstring &name, double value);

protected:
  std::shared_ptr<StoredField> shared_from_this()
  {
    return std::static_pointer_cast<StoredField>(Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
