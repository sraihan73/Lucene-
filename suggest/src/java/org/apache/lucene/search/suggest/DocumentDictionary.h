#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
namespace org::apache::lucene::search::suggest
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexableField = org::apache::lucene::index::IndexableField;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * <p>
 * Dictionary with terms, weights, payload (optional) and contexts (optional)
 * information taken from stored/indexed fields in a Lucene index.
 * </p>
 * <b>NOTE:</b>
 *  <ul>
 *    <li>
 *      The term field has to be stored; if it is missing, the document is
 * skipped.
 *    </li>
 *    <li>
 *      The payload and contexts field are optional and are not required to be
 * stored.
 *    </li>
 *    <li>
 *      The weight field can be stored or can be a {@link NumericDocValues}.
 *      If the weight field is not defined, the value of the weight is
 * <code>0</code>
 *    </li>
 *  </ul>
 */
class DocumentDictionary
    : public std::enable_shared_from_this<DocumentDictionary>,
      public Dictionary
{
  GET_CLASS_NAME(DocumentDictionary)

  /** {@link IndexReader} to load documents from */
protected:
  const std::shared_ptr<IndexReader> reader;

  /** Field to read payload from */
  const std::wstring payloadField;
  /** Field to read contexts from */
  const std::wstring contextsField;

private:
  const std::wstring field;
  const std::wstring weightField;

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms and <code>weightField</code> for the
   * weights that will be used for the corresponding terms.
   */
public:
  DocumentDictionary(std::shared_ptr<IndexReader> reader,
                     const std::wstring &field,
                     const std::wstring &weightField);

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms, <code>weightField</code> for the weights
   * that will be used for the the corresponding terms and
   * <code>payloadField</code> for the corresponding payloads for the entry.
   */
  DocumentDictionary(std::shared_ptr<IndexReader> reader,
                     const std::wstring &field, const std::wstring &weightField,
                     const std::wstring &payloadField);

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms, <code>weightField</code> for the weights
   * that will be used for the the corresponding terms,
   * <code>payloadField</code> for the corresponding payloads for the entry and
   * <code>contextsField</code> for associated contexts.
   */
  DocumentDictionary(std::shared_ptr<IndexReader> reader,
                     const std::wstring &field, const std::wstring &weightField,
                     const std::wstring &payloadField,
                     const std::wstring &contextsField);

  std::shared_ptr<InputIterator> getEntryIterator()  override;

  /** Implements {@link InputIterator} from stored fields. */
protected:
  class DocumentInputIterator
      : public std::enable_shared_from_this<DocumentInputIterator>,
        public InputIterator
  {
    GET_CLASS_NAME(DocumentInputIterator)
  private:
    std::shared_ptr<DocumentDictionary> outerInstance;

    const int docCount;
    const std::shared_ptr<Set<std::wstring>> relevantFields;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasPayloads_;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasContexts_;
    const std::shared_ptr<Bits> liveDocs;
    int currentDocId = -1;
    int64_t currentWeight = 0;
    std::shared_ptr<BytesRef> currentPayload = nullptr;
    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> currentContexts;
    const std::shared_ptr<NumericDocValues> weightValues;

  public:
    std::deque<std::shared_ptr<IndexableField>> currentDocFields =
        std::deque<std::shared_ptr<IndexableField>>(0);
    int nextFieldsPosition = 0;

    /**
     * Creates an iterator over term, weight and payload fields from the lucene
     * index. setting <code>withPayload</code> to false, implies an iterator
     * over only term and weight.
     */
    DocumentInputIterator(std::shared_ptr<DocumentDictionary> outerInstance,
                          bool hasPayloads,
                          bool hasContexts) ;

    int64_t weight() override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

    /**
     * Returns the value of the <code>weightField</code> for the current
     * document. Retrieves the value for the <code>weightField</code> if it's
     * stored (using <code>doc</code>) or if it's indexed as {@link
     * NumericDocValues} (using <code>docId</code>) for the document. If no
     * value is found, then the weight is 0.
     */
  protected:
    virtual int64_t getWeight(std::shared_ptr<Document> doc,
                                int docId) ;

  private:
    std::shared_ptr<Set<std::wstring>>
    getRelevantFields(std::deque<std::wstring> &fields);

  public:
    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
