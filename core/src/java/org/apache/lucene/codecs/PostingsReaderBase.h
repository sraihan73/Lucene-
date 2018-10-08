#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"

#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

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

using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using DataInput = org::apache::lucene::store::DataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;

/** The core terms dictionaries (BlockTermsReader,
 *  BlockTreeTermsReader) interact with a single instance
 *  of this class to manage creation of {@link
org.apache.lucene.index.PostingsEnum} and
 *  {@link org.apache.lucene.index.PostingsEnum} instances.  It provides an
GET_CLASS_NAME(to)
 *  IndexInput (termsIn) where this class may read any
 *  previously stored data that it had written in its
 *  corresponding {@link PostingsWriterBase} at indexing
 *  time.
 *  @lucene.experimental */

// TODO: maybe move under blocktree?  but it's used by other terms dicts (e.g.
// Block)

// TODO: find a better name; this defines the API that the
// terms dict impls use to talk to a postings impl.
// TermsDict + PostingsReader/WriterBase == PostingsConsumer/Producer
class PostingsReaderBase
    : public std::enable_shared_from_this<PostingsReaderBase>,
      public Accountable
{
  GET_CLASS_NAME(PostingsReaderBase)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  PostingsReaderBase();

  /** Performs any initialization, such as reading and
   *  verifying the header from the provided terms
   *  dictionary {@link IndexInput}. */
public:
  virtual void init(std::shared_ptr<IndexInput> termsIn,
                    std::shared_ptr<SegmentReadState> state) = 0;

  /** Return a newly created empty TermState */
  virtual std::shared_ptr<BlockTermState> newTermState() = 0;

  /** Actually decode metadata for next term
   *  @see PostingsWriterBase#encodeTerm
   */
  virtual void decodeTerm(std::deque<int64_t> &longs,
                          std::shared_ptr<DataInput> in_,
                          std::shared_ptr<FieldInfo> fieldInfo,
                          std::shared_ptr<BlockTermState> state,
                          bool absolute) = 0;

  /** Must fully consume state, since after this call that
   *  TermState may be reused. */
  virtual std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<FieldInfo> fieldInfo,
           std::shared_ptr<BlockTermState> state,
           std::shared_ptr<PostingsEnum> reuse, int flags) = 0;

  /**
   * Checks consistency of this reader.
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
  virtual void checkIntegrity() = 0;

  void close() = 0;
  override
};

} // #include  "core/src/java/org/apache/lucene/codecs/
