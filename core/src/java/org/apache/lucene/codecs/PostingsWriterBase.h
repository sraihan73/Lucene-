#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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

using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * Class that plugs into term dictionaries, such as {@link
 * BlockTreeTermsWriter}, and handles writing postings.
 *
 * @see PostingsReaderBase
 * @lucene.experimental
 */
// TODO: find a better name; this defines the API that the
// terms dict impls use to talk to a postings impl.
// TermsDict + PostingsReader/WriterBase == FieldsProducer/Consumer
class PostingsWriterBase
    : public std::enable_shared_from_this<PostingsWriterBase>
{
  GET_CLASS_NAME(PostingsWriterBase)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  PostingsWriterBase();

  /** Called once after startup, before any terms have been
   *  added.  Implementations typically write a header to
   *  the provided {@code termsOut}. */
public:
  virtual void init(std::shared_ptr<IndexOutput> termsOut,
                    std::shared_ptr<SegmentWriteState> state) = 0;

  /** Write all postings for one term; use the provided
   *  {@link TermsEnum} to pull a {@link org.apache.lucene.index.PostingsEnum}.
   *  This method should not
   *  re-position the {@code TermsEnum}!  It is already
   *  positioned on the term that should be written.  This
   *  method must set the bit in the provided {@link
   *  FixedBitSet} for every docID written.  If no docs
   *  were written, this method should return null, and the
   *  terms dict will skip the term. */
  virtual std::shared_ptr<BlockTermState>
  writeTerm(std::shared_ptr<BytesRef> term,
            std::shared_ptr<TermsEnum> termsEnum,
            std::shared_ptr<FixedBitSet> docsSeen) = 0;

  /**
   * Encode metadata as long[] and byte[]. {@code absolute} controls whether
   * current term is delta encoded according to latest term.
   * Usually elements in {@code longs} are file pointers, so each one always
   * increases when a new term is consumed. {@code out} is used to write generic
   * bytes, which are not monotonic.
   *
   * NOTE: sometimes long[] might contain "don't care" values that are unused,
   * e.g. the pointer to postings deque may not be defined for some terms but is
   * defined for others, if it is designed to inline  some postings data in term
   * dictionary. In this case, the postings writer should always use the last
   * value, so that each element in metadata long[] remains monotonic.
   */
  virtual void encodeTerm(std::deque<int64_t> &longs,
                          std::shared_ptr<DataOutput> out,
                          std::shared_ptr<FieldInfo> fieldInfo,
                          std::shared_ptr<BlockTermState> state,
                          bool absolute) = 0;

  /**
   * Sets the current field for writing, and returns the
   * fixed length of long[] metadata (which is fixed per
   * field), called when the writing switches to another field. */
  // TODO: better name?
  virtual int setField(std::shared_ptr<FieldInfo> fieldInfo) = 0;

  void close() = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/codecs/
