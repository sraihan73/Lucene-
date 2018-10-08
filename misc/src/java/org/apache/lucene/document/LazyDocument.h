#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::document
{
class LazyField;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Defers actually loading a field's value until you ask
 *  for it.  You must not use the returned Field instances
 *  after the provided reader has been closed.
 * @see #getField
 */
class LazyDocument : public std::enable_shared_from_this<LazyDocument>
{
  GET_CLASS_NAME(LazyDocument)
private:
  const std::shared_ptr<IndexReader> reader;
  const int docID;

  // null until first field is loaded
  std::shared_ptr<Document> doc;

  std::unordered_map<int, std::deque<std::shared_ptr<LazyField>>> fields =
      std::unordered_map<int, std::deque<std::shared_ptr<LazyField>>>();
  std::shared_ptr<Set<std::wstring>> fieldNames =
      std::unordered_set<std::wstring>();

public:
  LazyDocument(std::shared_ptr<IndexReader> reader, int docID);

  /**
   * Creates a StorableField whose value will be lazy loaded if and
   * when it is used.
   * <p>
   * <b>NOTE:</b> This method must be called once for each value of the field
   * name specified in sequence that the values exist.  This method may not be
   * used to generate multiple, lazy, StorableField instances refering to
   * the same underlying StorableField instance.
   * </p>
   * <p>
   * The lazy loading of field values from all instances of StorableField
   * objects returned by this method are all backed by a single StoredDocument
   * per LazyDocument instance.
   * </p>
   */
  virtual std::shared_ptr<IndexableField>
  getField(std::shared_ptr<FieldInfo> fieldInfo);

  /**
   * non-private for test only access
   * @lucene.internal
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<Document> getDocument();

  // :TODO: synchronize to prevent redundent copying? (sync per field name?)
private:
  void fetchRealValues(const std::wstring &name, int fieldNum);

  /**
   * @lucene.internal
   */
public:
  class LazyField : public std::enable_shared_from_this<LazyField>,
                    public IndexableField
  {
    GET_CLASS_NAME(LazyField)
  private:
    std::shared_ptr<LazyDocument> outerInstance;

    // C++ NOTE: Fields cannot have the same name as methods:
    std::wstring name_;
    int fieldNum = 0;

  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile org.apache.lucene.index.IndexableField realValue
    // = null;
    std::shared_ptr<IndexableField> realValue = nullptr;

  private:
    LazyField(std::shared_ptr<LazyDocument> outerInstance,
              const std::wstring &name, int fieldNum);

    /**
     * non-private for test only access
     * @lucene.internal
     */
  public:
    virtual bool hasBeenLoaded();

  private:
    std::shared_ptr<IndexableField> getRealValue();

  public:
    std::wstring name() override;

    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<Number> numericValue() override;

    std::shared_ptr<IndexableFieldType> fieldType() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> analyzer,
                std::shared_ptr<TokenStream> reuse) override;
  };
};

} // namespace org::apache::lucene::document
