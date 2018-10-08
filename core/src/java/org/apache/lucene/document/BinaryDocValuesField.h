#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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

/**
 * Field that stores a per-document {@link BytesRef} value.
 * <p>
 * The values are stored directly with no sharing, which is a good fit when
 * the fields don't share (many) values, such as a title field.  If values
 * may be shared and sorted it's better to use {@link SortedDocValuesField}.
 * Here's an example usage:
 *
 * <pre class="prettyprint">
 *   document.add(new BinaryDocValuesField(name, new BytesRef("hello")));
 * </pre>
 *
 * <p>
 * If you also need to store the value, you should add a
 * separate {@link StoredField} instance.
 *
 * @see BinaryDocValues
 * */
class BinaryDocValuesField : public Field
{
  GET_CLASS_NAME(BinaryDocValuesField)

  /**
   * Type for straight bytes DocValues.
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
  static BinaryDocValuesField::StaticConstructor staticConstructor;

  /**
   * Create a new binary DocValues field.
   * @param name field name
   * @param value binary content
   * @throws IllegalArgumentException if the field name is null
   */
public:
  BinaryDocValuesField(const std::wstring &name,
                       std::shared_ptr<BytesRef> value);

protected:
  std::shared_ptr<BinaryDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<BinaryDocValuesField>(
        Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
