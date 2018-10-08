#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/TermsHash.h"

#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"
#include  "core/src/java/org/apache/lucene/index/DocState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInvertState.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TermToBytesRefAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TermFrequencyAttribute.h"
#include  "core/src/java/org/apache/lucene/util/IntBlockPool.h"
#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include  "core/src/java/org/apache/lucene/index/ParallelPostingsArray.h"
#include  "core/src/java/org/apache/lucene/util/Counter.h"
#include  "core/src/java/org/apache/lucene/index/ByteSliceReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"

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

using TermFrequencyAttribute =
    org::apache::lucene::analysis::tokenattributes::TermFrequencyAttribute;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesStartArray =
    org::apache::lucene::util::BytesRefHash::BytesStartArray;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;

class TermsHashPerField
    : public std::enable_shared_from_this<TermsHashPerField>,
      public Comparable<std::shared_ptr<TermsHashPerField>>
{
  GET_CLASS_NAME(TermsHashPerField)
private:
  static constexpr int HASH_INIT_SIZE = 4;

public:
  const std::shared_ptr<TermsHash> termsHash;

  const std::shared_ptr<TermsHashPerField> nextPerField;

protected:
  const std::shared_ptr<DocumentsWriterPerThread::DocState> docState;
  const std::shared_ptr<FieldInvertState> fieldState;

public:
  std::shared_ptr<TermToBytesRefAttribute> termAtt;

protected:
  std::shared_ptr<TermFrequencyAttribute> termFreqAtt;

  // Copied from our perThread
public:
  const std::shared_ptr<IntBlockPool> intPool;
  const std::shared_ptr<ByteBlockPool> bytePool;
  const std::shared_ptr<ByteBlockPool> termBytePool;

  const int streamCount;
  const int numPostingInt;

protected:
  const std::shared_ptr<FieldInfo> fieldInfo;

public:
  const std::shared_ptr<BytesRefHash> bytesHash;

  std::shared_ptr<ParallelPostingsArray> postingsArray;

private:
  const std::shared_ptr<Counter> bytesUsed;

  /** streamCount: how many streams this field stores per term.
   * E.g. doc(+freq) is 1 stream, prox+offset is a second. */

public:
  TermsHashPerField(int streamCount,
                    std::shared_ptr<FieldInvertState> fieldState,
                    std::shared_ptr<TermsHash> termsHash,
                    std::shared_ptr<TermsHashPerField> nextPerField,
                    std::shared_ptr<FieldInfo> fieldInfo);

  virtual void reset();

  virtual void initReader(std::shared_ptr<ByteSliceReader> reader, int termID,
                          int stream);

  std::deque<int> sortedTermIDs;

  /** Collapse the hash table and sort in-place; also sets
   * this.sortedTermIDs to the results */
  virtual std::deque<int> sortPostings();

private:
  bool doNextCall = false;

  // Secondary entry point (for 2nd & subsequent TermsHash),
  // because token text has already been "interned" into
  // textStart, so we hash by textStart.  term vectors use
  // this API.
public:
  virtual void add(int textStart) ;

  /** Called once per inverted token.  This is the primary
   *  entry point (for first TermsHash); postings use this
   *  API. */
  virtual void add() ;

  std::deque<int> intUptos;
  int intUptoStart = 0;

  virtual void writeByte(int stream, char b);

  virtual void writeBytes(int stream, std::deque<char> &b, int offset,
                          int len);

  virtual void writeVInt(int stream, int i);

private:
  class PostingsBytesStartArray final : public BytesStartArray
  {
    GET_CLASS_NAME(PostingsBytesStartArray)

  private:
    const std::shared_ptr<TermsHashPerField> perField;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<Counter> bytesUsed_;

    PostingsBytesStartArray(std::shared_ptr<TermsHashPerField> perField,
                            std::shared_ptr<Counter> bytesUsed);

  public:
    std::deque<int> init() override;

    std::deque<int> grow() override;

    std::deque<int> clear() override;

    std::shared_ptr<Counter> bytesUsed() override;

  protected:
    std::shared_ptr<PostingsBytesStartArray> shared_from_this()
    {
      return std::static_pointer_cast<PostingsBytesStartArray>(
          org.apache.lucene.util.BytesRefHash
              .BytesStartArray::shared_from_this());
    }
  };

public:
  int compareTo(std::shared_ptr<TermsHashPerField> other) override;

  /** Finish adding all instances of this field to the
   *  current document. */
  virtual void finish() ;

  /** Start adding a new field instance; first is true if
   *  this is the first time this field name was seen in the
   *  document. */
  virtual bool start(std::shared_ptr<IndexableField> field, bool first);

  /** Called when a term is seen for the first time. */
  virtual void newTerm(int termID) = 0;

  /** Called when a previously seen term is seen again. */
  virtual void addTerm(int termID) = 0;

  /** Called when the postings array is initialized or
   *  resized. */
  virtual void newPostingsArray() = 0;

  /** Creates a new postings array of the specified size. */
  virtual std::shared_ptr<ParallelPostingsArray>
  createPostingsArray(int size) = 0;
};

} // #include  "core/src/java/org/apache/lucene/index/
