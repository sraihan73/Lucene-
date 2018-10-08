#pragma once
#include "../index/DocIDMerger.h"
#include "../index/IndexableField.h"
#include "../index/StoredFieldVisitor.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfo;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
}
namespace org::apache::lucene::codecs
{
class MergeVisitor;
}
namespace org::apache::lucene::index
{
class MergeState;
}
namespace org::apache::lucene::index
{
class DocMap;
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
namespace org::apache::lucene::codecs
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using MergeState = org::apache::lucene::index::MergeState;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Codec API for writing stored fields:
 * <ol>
 *   <li>For every document, {@link #startDocument()} is called,
 *       informing the Codec that a new document has started.
 *   <li>{@link #writeField(FieldInfo, IndexableField)} is called for
 *       each field in the document.
 *   <li>After all documents have been written, {@link #finish(FieldInfos, int)}
 *       is called for verification/sanity-checks.
 *   <li>Finally the writer is closed ({@link #close()})
 * </ol>
 *
 * @lucene.experimental
 */
class StoredFieldsWriter
    : public std::enable_shared_from_this<StoredFieldsWriter>
{
  GET_CLASS_NAME(StoredFieldsWriter)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  StoredFieldsWriter();

  /** Called before writing the stored fields of the document.
   *  {@link #writeField(FieldInfo, IndexableField)} will be called
   *  for each stored field. Note that this is
   *  called even if the document has no stored fields. */
public:
  virtual void startDocument() = 0;

  /** Called when a document and all its fields have been added. */
  virtual void finishDocument() ;

  /** Writes a single stored field. */
  virtual void writeField(std::shared_ptr<FieldInfo> info,
                          std::shared_ptr<IndexableField> field) = 0;

  /** Called before {@link #close()}, passing in the number
   *  of documents that were written. Note that this is
   *  intentionally redundant (equivalent to the number of
   *  calls to {@link #startDocument()}, but a Codec should
   *  check that this is the case to detect the JRE bug described
   *  in LUCENE-1282. */
  virtual void finish(std::shared_ptr<FieldInfos> fis, int numDocs) = 0;

private:
  class StoredFieldsMergeSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(StoredFieldsMergeSub)
  private:
    const std::shared_ptr<StoredFieldsReader> reader;
    const int maxDoc;
    const std::shared_ptr<MergeVisitor> visitor;

  public:
    int docID = -1;

    StoredFieldsMergeSub(std::shared_ptr<MergeVisitor> visitor,
                         std::shared_ptr<MergeState::DocMap> docMap,
                         std::shared_ptr<StoredFieldsReader> reader,
                         int maxDoc);

    int nextDoc() override;

  protected:
    std::shared_ptr<StoredFieldsMergeSub> shared_from_this()
    {
      return std::static_pointer_cast<StoredFieldsMergeSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /** Merges in the stored fields from the readers in
   *  <code>mergeState</code>. The default implementation skips
   *  over deleted documents, and uses {@link #startDocument()},
   *  {@link #writeField(FieldInfo, IndexableField)}, and {@link
   * #finish(FieldInfos, int)}, returning the number of documents that were
   * written. Implementations can override this method for more sophisticated
   *  merging (bulk-byte copying, etc). */
public:
  virtual int merge(std::shared_ptr<MergeState> mergeState) ;

  /**
   * A visitor that adds every field it sees.
   * <p>
   * Use like this:
   * <pre>
   * MergeVisitor visitor = new MergeVisitor(mergeState, readerIndex);
   * for (...) {
   *   startDocument();
   *   storedFieldsReader.visitDocument(docID, visitor);
   *   finishDocument();
   * }
   * </pre>
   */
protected:
  class MergeVisitor : public StoredFieldVisitor, public IndexableField
  {
    GET_CLASS_NAME(MergeVisitor)
  private:
    std::shared_ptr<StoredFieldsWriter> outerInstance;

  public:
    // C++ NOTE: Fields cannot have the same name as methods:
    std::shared_ptr<BytesRef> binaryValue_;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::wstring stringValue_;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::shared_ptr<Number> numericValue_;
    std::shared_ptr<FieldInfo> currentField;
    std::shared_ptr<FieldInfos> remapper;

    /**
     * Create new merge visitor.
     */
    MergeVisitor(std::shared_ptr<StoredFieldsWriter> outerInstance,
                 std::shared_ptr<MergeState> mergeState, int readerIndex);

    void binaryField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void stringField(std::shared_ptr<FieldInfo> fieldInfo,
                     std::deque<char> &value)  override;

    void intField(std::shared_ptr<FieldInfo> fieldInfo,
                  int value)  override;

    void longField(std::shared_ptr<FieldInfo> fieldInfo,
                   int64_t value)  override;

    void floatField(std::shared_ptr<FieldInfo> fieldInfo,
                    float value)  override;

    void doubleField(std::shared_ptr<FieldInfo> fieldInfo,
                     double value)  override;

    Status needsField(std::shared_ptr<FieldInfo> fieldInfo) throw(
        IOException) override;

    std::wstring name() override;

    std::shared_ptr<IndexableFieldType> fieldType() override;

    std::shared_ptr<BytesRef> binaryValue() override;

    std::wstring stringValue() override;

    std::shared_ptr<Number> numericValue() override;

    std::shared_ptr<Reader> readerValue() override;

    std::shared_ptr<TokenStream>
    tokenStream(std::shared_ptr<Analyzer> analyzer,
                std::shared_ptr<TokenStream> reuse) override;

    virtual void reset(std::shared_ptr<FieldInfo> field);

    virtual void write() ;

  protected:
    std::shared_ptr<MergeVisitor> shared_from_this()
    {
      return std::static_pointer_cast<MergeVisitor>(
          org.apache.lucene.index.StoredFieldVisitor::shared_from_this());
    }
  };

public:
  void close() = 0;
  override
};

} // namespace org::apache::lucene::codecs
