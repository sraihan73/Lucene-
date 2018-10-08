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

/** A field that is indexed but not tokenized: the entire
 *  std::wstring value is indexed as a single token.  For example
 *  this might be used for a 'country' field or an 'id'
 *  field.  If you also need to sort on this field, separately
 *  add a {@link SortedDocValuesField} to your document. */

class StringField final : public Field
{
  GET_CLASS_NAME(StringField)

  /** Indexed, not tokenized, omits norms, indexes
   *  DOCS_ONLY, not stored. */
public:
  static const std::shared_ptr<FieldType> TYPE_NOT_STORED;

  /** Indexed, not tokenized, omits norms, indexes
   *  DOCS_ONLY, stored */
  static const std::shared_ptr<FieldType> TYPE_STORED;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static StringField::StaticConstructor staticConstructor;

  /** Creates a new textual StringField, indexing the provided std::wstring value
   *  as a single token.
   *
   *  @param name field name
   *  @param value std::wstring value
   *  @param stored Store.YES if the content should also be stored
   *  @throws IllegalArgumentException if the field name or value is null.
   */
public:
  StringField(const std::wstring &name, const std::wstring &value,
              Store stored);

  /** Creates a new binary StringField, indexing the provided binary (BytesRef)
   *  value as a single token.
   *
   *  @param name field name
   *  @param value BytesRef value.  The provided value is not cloned so
   *         you must not change it until the document(s) holding it
   *         have been indexed.
   *  @param stored Store.YES if the content should also be stored
   *  @throws IllegalArgumentException if the field name or value is null.
   */
  StringField(const std::wstring &name, std::shared_ptr<BytesRef> value,
              Store stored);

protected:
  std::shared_ptr<StringField> shared_from_this()
  {
    return std::static_pointer_cast<StringField>(Field::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
