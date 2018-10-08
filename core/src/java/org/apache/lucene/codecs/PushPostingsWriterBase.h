#pragma once
#include "../index/IndexOptions.h"
#include "PostingsWriterBase.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"

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
using IndexOptions = org::apache::lucene::index::IndexOptions;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * Extension of {@link PostingsWriterBase}, adding a push
 * API for writing each element of the postings.  This API
 * is somewhat analagous to an XML SAX API, while {@link
 * PostingsWriterBase} is more like an XML DOM API.
 *
 * @see PostingsReaderBase
 * @lucene.experimental
 */
// TODO: find a better name; this defines the API that the
// terms dict impls use to talk to a postings impl.
// TermsDict + PostingsReader/WriterBase == PostingsConsumer/Producer
class PushPostingsWriterBase : public PostingsWriterBase
{
  GET_CLASS_NAME(PushPostingsWriterBase)

  // Reused in writeTerm
private:
  std::shared_ptr<PostingsEnum> postingsEnum;
  int enumFlags = 0;

  /** {@link FieldInfo} of current field being written. */
protected:
  std::shared_ptr<FieldInfo> fieldInfo;

  /** {@link IndexOptions} of current field being
      written */
  IndexOptions indexOptions = static_cast<IndexOptions>(0);

  /** True if the current field writes freqs. */
  bool writeFreqs = false;

  /** True if the current field writes positions. */
  bool writePositions = false;

  /** True if the current field writes payloads. */
  bool writePayloads = false;

  /** True if the current field writes offsets. */
  bool writeOffsets = false;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
  PushPostingsWriterBase();

  /** Return a newly created empty TermState */
public:
  virtual std::shared_ptr<BlockTermState> newTermState() = 0;

  /** Start a new term.  Note that a matching call to {@link
   *  #finishTerm(BlockTermState)} is done, only if the term has at least one
   *  document. */
  virtual void startTerm() = 0;

  /** Finishes the current term.  The provided {@link
   *  BlockTermState} contains the term's summary statistics,
   *  and will holds metadata from PBF when returned */
  virtual void finishTerm(std::shared_ptr<BlockTermState> state) = 0;

  /**
   * Sets the current field for writing, and returns the
   * fixed length of long[] metadata (which is fixed per
   * field), called when the writing switches to another field. */
  int setField(std::shared_ptr<FieldInfo> fieldInfo) override;

  std::shared_ptr<BlockTermState> writeTerm(
      std::shared_ptr<BytesRef> term, std::shared_ptr<TermsEnum> termsEnum,
      std::shared_ptr<FixedBitSet> docsSeen)  override final;

  /** Adds a new doc in this term.
   * <code>freq</code> will be -1 when term frequencies are omitted
   * for the field. */
  virtual void startDoc(int docID, int freq) = 0;

  /** Add a new position and payload, and start/end offset.  A
   *  null payload means no payload; a non-null payload with
   *  zero length also means no payload.  Caller may reuse
   *  the {@link BytesRef} for the payload between calls
   *  (method must fully consume the payload). <code>startOffset</code>
   *  and <code>endOffset</code> will be -1 when offsets are not indexed. */
  virtual void addPosition(int position, std::shared_ptr<BytesRef> payload,
                           int startOffset, int endOffset) = 0;

  /** Called when we are done adding positions and payloads
   *  for each doc. */
  virtual void finishDoc() = 0;

protected:
  std::shared_ptr<PushPostingsWriterBase> shared_from_this()
  {
    return std::static_pointer_cast<PushPostingsWriterBase>(
        PostingsWriterBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/
