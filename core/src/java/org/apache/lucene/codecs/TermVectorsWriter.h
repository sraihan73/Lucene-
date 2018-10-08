#pragma once
#include "../index/DocIDMerger.h"
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
namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
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
class Fields;
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

using DocIDMerger = org::apache::lucene::index::DocIDMerger;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using MergeState = org::apache::lucene::index::MergeState;
using DataInput = org::apache::lucene::store::DataInput;
using BytesRef = org::apache::lucene::util::BytesRef;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Codec API for writing term vectors:
 * <ol>
 *   <li>For every document, {@link #startDocument(int)} is called,
 *       informing the Codec how many fields will be written.
 *   <li>{@link #startField(FieldInfo, int, bool, bool, bool)} is
 * called for each field in the document, informing the codec how many terms
 *       will be written for that field, and whether or not positions,
 *       offsets, or payloads are enabled.
 *   <li>Within each field, {@link #startTerm(BytesRef, int)} is called
 *       for each term.
 *   <li>If offsets and/or positions are enabled, then
 *       {@link #addPosition(int, int, int, BytesRef)} will be called for each
 * term occurrence. <li>After all documents have been written, {@link
 * #finish(FieldInfos, int)} is called for verification/sanity-checks.
 *   <li>Finally the writer is closed ({@link #close()})
 * </ol>
 *
 * @lucene.experimental
 */
class TermVectorsWriter : public std::enable_shared_from_this<TermVectorsWriter>
{
  GET_CLASS_NAME(TermVectorsWriter)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  TermVectorsWriter();

  /** Called before writing the term vectors of the document.
   *  {@link #startField(FieldInfo, int, bool, bool, bool)} will
   *  be called <code>numVectorFields</code> times. Note that if term
   *  vectors are enabled, this is called even if the document
   *  has no deque fields, in this case <code>numVectorFields</code>
   *  will be zero. */
public:
  virtual void startDocument(int numVectorFields) = 0;

  /** Called after a doc and all its fields have been added. */
  virtual void finishDocument() ;

  /** Called before writing the terms of the field.
   *  {@link #startTerm(BytesRef, int)} will be called <code>numTerms</code>
   * times. */
  virtual void startField(std::shared_ptr<FieldInfo> info, int numTerms,
                          bool positions, bool offsets, bool payloads) = 0;

  /** Called after a field and all its terms have been added. */
  virtual void finishField() ;

  /** Adds a term and its term frequency <code>freq</code>.
   * If this field has positions and/or offsets enabled, then
   * {@link #addPosition(int, int, int, BytesRef)} will be called
   * <code>freq</code> times respectively.
   */
  virtual void startTerm(std::shared_ptr<BytesRef> term, int freq) = 0;

  /** Called after a term and all its positions have been added. */
  virtual void finishTerm() ;

  /** Adds a term position and offsets */
  virtual void addPosition(int position, int startOffset, int endOffset,
                           std::shared_ptr<BytesRef> payload) = 0;

  /** Called before {@link #close()}, passing in the number
   *  of documents that were written. Note that this is
   *  intentionally redundant (equivalent to the number of
   *  calls to {@link #startDocument(int)}, but a Codec should
   *  check that this is the case to detect the JRE bug described
   *  in LUCENE-1282. */
  virtual void finish(std::shared_ptr<FieldInfos> fis, int numDocs) = 0;

  /**
   * Called by IndexWriter when writing new segments.
   * <p>
   * This is an expert API that allows the codec to consume
   * positions and offsets directly from the indexer.
   * <p>
   * The default implementation calls {@link #addPosition(int, int, int,
   * BytesRef)}, but subclasses can override this if they want to efficiently
   * write all the positions, then all the offsets, for example. <p> NOTE: This
   * API is extremely expert and subject to change or removal!!!
   * @lucene.internal
   */
  // TODO: we should probably nuke this and make a more efficient 4.x format
  // PreFlex-RW could then be slow and buffer (it's only used in tests...)
  virtual void addProx(int numProx, std::shared_ptr<DataInput> positions,
                       std::shared_ptr<DataInput> offsets) ;

private:
  class TermVectorsMergeSub : public DocIDMerger::Sub
  {
    GET_CLASS_NAME(TermVectorsMergeSub)
  private:
    const std::shared_ptr<TermVectorsReader> reader;
    const int maxDoc;

  public:
    int docID = -1;

    TermVectorsMergeSub(std::shared_ptr<MergeState::DocMap> docMap,
                        std::shared_ptr<TermVectorsReader> reader, int maxDoc);

    int nextDoc() override;

  protected:
    std::shared_ptr<TermVectorsMergeSub> shared_from_this()
    {
      return std::static_pointer_cast<TermVectorsMergeSub>(
          org.apache.lucene.index.DocIDMerger.Sub::shared_from_this());
    }
  };

  /** Merges in the term vectors from the readers in
   *  <code>mergeState</code>. The default implementation skips
   *  over deleted documents, and uses {@link #startDocument(int)},
   *  {@link #startField(FieldInfo, int, bool, bool, bool)},
   *  {@link #startTerm(BytesRef, int)}, {@link #addPosition(int, int, int,
   * BytesRef)}, and {@link #finish(FieldInfos, int)}, returning the number of
   * documents that were written. Implementations can override this method for
   * more sophisticated merging (bulk-byte copying, etc). */
public:
  virtual int merge(std::shared_ptr<MergeState> mergeState) ;

  /** Safe (but, slowish) default method to write every
   *  deque field in the document. */
protected:
  void
  addAllDocVectors(std::shared_ptr<Fields> vectors,
                   std::shared_ptr<MergeState> mergeState) ;

public:
  void close() = 0;
  override
};

} // namespace org::apache::lucene::codecs
