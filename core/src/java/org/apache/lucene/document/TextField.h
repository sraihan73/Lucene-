#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}

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

using TokenStream = org::apache::lucene::analysis::TokenStream;

/** A field that is indexed and tokenized, without term
 *  vectors.  For example this would be used on a 'body'
 *  field, that contains the bulk of a document's text. */

class TextField final : public Field
{
  GET_CLASS_NAME(TextField)

  /** Indexed, tokenized, not stored. */
public:
  static const std::shared_ptr<FieldType> TYPE_NOT_STORED;

  /** Indexed, tokenized, stored. */
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
  static TextField::StaticConstructor staticConstructor;

  // TODO: add sugar for term vectors...?

  /** Creates a new un-stored TextField with Reader value.
   * @param name field name
   * @param reader reader value
   * @throws IllegalArgumentException if the field name is null
   * @throws NullPointerException if the reader is null
   */
public:
  TextField(const std::wstring &name, std::shared_ptr<Reader> reader);

  /** Creates a new TextField with std::wstring value.
   * @param name field name
   * @param value string value
   * @param store Store.YES if the content should also be stored
   * @throws IllegalArgumentException if the field name or value is null.
   */
  TextField(const std::wstring &name, const std::wstring &value, Store store);

  /** Creates a new un-stored TextField with TokenStream value.
   * @param name field name
   * @param stream TokenStream value
   * @throws IllegalArgumentException if the field name is null.
   * @throws NullPointerException if the tokenStream is null
   */
  TextField(const std::wstring &name, std::shared_ptr<TokenStream> stream);

protected:
  std::shared_ptr<TextField> shared_from_this()
  {
    return std::static_pointer_cast<TextField>(Field::shared_from_this());
  }
};

} // namespace org::apache::lucene::document
