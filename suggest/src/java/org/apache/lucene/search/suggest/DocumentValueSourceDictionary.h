#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class LongValuesSource;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::suggest
{
class InputIterator;
}
namespace org::apache::lucene::search
{
class LongValues;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::search::suggest
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;

/**
 * <p>
 * Dictionary with terms and optionally payload and
 * optionally contexts information
 * taken from stored fields in a Lucene index. Similar to
 * {@link DocumentDictionary}, except it obtains the weight
 * of the terms in a document based on a {@link LongValuesSource}.
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
 *  </ul>
 *  <p>
 *  In practice the {@link LongValuesSource} will likely be obtained
 *  using the lucene expression module. The following example shows
 *  how to create a {@link LongValuesSource} from a simple addition of two
 *  fields:
 *  <code>
 *    Expression expression = JavascriptCompiler.compile("f1 + f2");
 *    SimpleBindings bindings = new SimpleBindings();
 *    bindings.add(new SortField("f1", SortField.Type.LONG));
 *    bindings.add(new SortField("f2", SortField.Type.LONG));
 *    LongValuesSource valueSource =
 * expression.getDoubleValuesSource(bindings).toLongValuesSource();
 *  </code>
 *  </p>
 *
 */
class DocumentValueSourceDictionary : public DocumentDictionary
{
  GET_CLASS_NAME(DocumentValueSourceDictionary)

private:
  const std::shared_ptr<LongValuesSource> weightsValueSource;

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms, <code>payload</code> for the
   * corresponding payloads, <code>contexts</code> for the associated contexts
   * and uses the <code>weightsValueSource</code> supplied to determine the
   * score.
   */
public:
  DocumentValueSourceDictionary(
      std::shared_ptr<IndexReader> reader, const std::wstring &field,
      std::shared_ptr<LongValuesSource> weightsValueSource,
      const std::wstring &payload, const std::wstring &contexts);

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms, <code>payloadField</code> for the
   * corresponding payloads and uses the <code>weightsValueSource</code>
   * supplied to determine the score.
   */
  DocumentValueSourceDictionary(
      std::shared_ptr<IndexReader> reader, const std::wstring &field,
      std::shared_ptr<LongValuesSource> weightsValueSource,
      const std::wstring &payload);

  /**
   * Creates a new dictionary with the contents of the fields named
   * <code>field</code> for the terms and uses the
   * <code>weightsValueSource</code> supplied to determine the score.
   */
  DocumentValueSourceDictionary(
      std::shared_ptr<IndexReader> reader, const std::wstring &field,
      std::shared_ptr<LongValuesSource> weightsValueSource);

  std::shared_ptr<InputIterator> getEntryIterator()  override;

public:
  class DocumentValueSourceInputIterator final
      : public DocumentDictionary::DocumentInputIterator
  {
    GET_CLASS_NAME(DocumentValueSourceInputIterator)
  private:
    std::shared_ptr<DocumentValueSourceDictionary> outerInstance;

    std::shared_ptr<LongValues> currentWeightValues;
    /** leaves of the reader */
    const std::deque<std::shared_ptr<LeafReaderContext>> leaves;
    /** starting docIds of all the leaves */
    std::deque<int> const starts;
    /** current leave index */
    int currentLeafIndex = 0;

  public:
    DocumentValueSourceInputIterator(
        std::shared_ptr<DocumentValueSourceDictionary> outerInstance,
        bool hasPayloads, bool hasContexts) ;

    /**
     * Returns the weight for the current <code>docId</code> as computed
     * by the <code>weightsValueSource</code>
     * */
  protected:
    int64_t getWeight(std::shared_ptr<Document> doc,
                        int docId)  override;

  protected:
    std::shared_ptr<DocumentValueSourceInputIterator> shared_from_this()
    {
      return std::static_pointer_cast<DocumentValueSourceInputIterator>(
          DocumentDictionary.DocumentInputIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DocumentValueSourceDictionary> shared_from_this()
  {
    return std::static_pointer_cast<DocumentValueSourceDictionary>(
        DocumentDictionary::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest
