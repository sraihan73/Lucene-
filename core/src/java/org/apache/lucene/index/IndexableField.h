#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexableFieldType;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::index
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesRef = org::apache::lucene::util::BytesRef;

// TODO: how to handle versioning here...?

/** Represents a single field for indexing.  IndexWriter
 *  consumes Iterable&lt;IndexableField&gt; as a document.
 *
 *  @lucene.experimental */

class IndexableField
{
  GET_CLASS_NAME(IndexableField)

  /** Field name */
public:
  virtual std::wstring name() = 0;

  /** {@link IndexableFieldType} describing the properties
   * of this field. */
  virtual std::shared_ptr<IndexableFieldType> fieldType() = 0;

  /**
   * Creates the TokenStream used for indexing this field.  If appropriate,
   * implementations should use the given Analyzer to create the TokenStreams.
   *
   * @param analyzer Analyzer that should be used to create the TokenStreams
   * from
   * @param reuse TokenStream for a previous instance of this field <b>name</b>.
   * This allows custom field types (like StringField and NumericField) that do
   * not use the analyzer to still have good performance. Note: the passed-in
   * type may be inappropriate, for example if you mix up different types of
   * Fields for the same field name. So it's the responsibility of the
   * implementation to check.
   * @return TokenStream value for indexing the document.  Should always return
   *         a non-null value if the field is to be indexed
   */
  virtual std::shared_ptr<TokenStream>
  tokenStream(std::shared_ptr<Analyzer> analyzer,
              std::shared_ptr<TokenStream> reuse) = 0;

  /** Non-null if this field has a binary value */
  virtual std::shared_ptr<BytesRef> binaryValue() = 0;

  /** Non-null if this field has a string value */
  virtual std::wstring stringValue() = 0;

  /** Non-null if this field has a Reader value */
  virtual std::shared_ptr<Reader> readerValue() = 0;

  /** Non-null if this field has a numeric value */
  virtual std::shared_ptr<Number> numericValue() = 0;
};

} // namespace org::apache::lucene::index
