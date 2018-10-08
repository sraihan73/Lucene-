#pragma once
#include "../../index/TermsEnum.h"
#include "../../util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktree/SegmentTermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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
namespace org::apache::lucene::codecs::blocktree
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using FST = org::apache::lucene::util::fst::FST;

class SegmentTermsEnumFrame final
    : public std::enable_shared_from_this<SegmentTermsEnumFrame>
{
  GET_CLASS_NAME(SegmentTermsEnumFrame)
  // Our index in stack[]:
public:
  const int ord;

  bool hasTerms = false;
  bool hasTermsOrig = false;
  bool isFloor = false;

  std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc;

  // static bool DEBUG = BlockTreeTermsWriter.DEBUG;

  // File pointer where this block was loaded from
  int64_t fp = 0;
  int64_t fpOrig = 0;
  int64_t fpEnd = 0;

  std::deque<char> suffixBytes = std::deque<char>(128);
  const std::shared_ptr<ByteArrayDataInput> suffixesReader =
      std::make_shared<ByteArrayDataInput>();

  std::deque<char> statBytes = std::deque<char>(64);
  const std::shared_ptr<ByteArrayDataInput> statsReader =
      std::make_shared<ByteArrayDataInput>();

  std::deque<char> floorData = std::deque<char>(32);
  const std::shared_ptr<ByteArrayDataInput> floorDataReader =
      std::make_shared<ByteArrayDataInput>();

  // Length of prefix shared by all terms in this block
  int prefix = 0;

  // Number of entries (term or sub-block) in this block
  int entCount = 0;

  // Which term we will next read, or -1 if the block
  // isn't loaded yet
  int nextEnt = 0;

  // True if this block is either not a floor block,
  // or, it's the last sub-block of a floor block
  bool isLastInFloor = false;

  // True if all entries are terms
  bool isLeafBlock = false;

  int64_t lastSubFP = 0;

  int nextFloorLabel = 0;
  int numFollowFloorBlocks = 0;

  // Next term to decode metaData; we decode metaData
  // lazily so that scanning to find the matching term is
  // fast and only if you find a match and app wants the
  // stats or docs/positions enums, will we decode the
  // metaData
  int metaDataUpto = 0;

  const std::shared_ptr<BlockTermState> state;

  // metadata buffer, holding monotonic values
  std::deque<int64_t> const longs;
  // metadata buffer, holding general values
  std::deque<char> bytes = std::deque<char>(32);
  const std::shared_ptr<ByteArrayDataInput> bytesReader =
      std::make_shared<ByteArrayDataInput>();

private:
  const std::shared_ptr<SegmentTermsEnum> ste;

public:
  SegmentTermsEnumFrame(std::shared_ptr<SegmentTermsEnum> ste,
                        int ord) ;

  void setFloorData(std::shared_ptr<ByteArrayDataInput> in_,
                    std::shared_ptr<BytesRef> source);

  int getTermBlockOrd();

  void loadNextFloorBlock() ;

  /* Does initial decode of next block of terms; this
     doesn't actually decode the docFreq, totalTermFreq,
     postings details (frq/prx offset, etc.) metadata;
     it just loads them as byte[] blobs which are then
     decoded on-demand if the metadata is ever requested
     for any term in this block.  This enables terms-only
     intensive consumes (eg certain MTQs, respelling) to
     not pay the price of decoding metadata they won't
     use. */
  void loadBlock() ;

  void rewind();

  // Decodes next entry; returns true if it's a sub-block
  bool next() ;

  void nextLeaf();

  bool nextNonLeaf() ;

  // TODO: make this array'd so we can do bin search?
  // likely not worth it?  need to measure how many
  // floor blocks we "typically" get
  void scanToFloorFrame(std::shared_ptr<BytesRef> target);

  void decodeMetaData() ;

  // Used only by assert
private:
  bool prefixMatches(std::shared_ptr<BytesRef> target);

  // Scans to sub-block that has this target fp; only
  // called by next(); NOTE: does not set
  // startBytePos/suffix as a side effect
public:
  void scanToSubBlock(int64_t subFP);

  // NOTE: sets startBytePos/suffix as a side effect
  SeekStatus scanToTerm(std::shared_ptr<BytesRef> target,
                        bool exactOnly) ;

private:
  int startBytePos = 0;
  int suffix = 0;
  int64_t subCode = 0;

  // for debugging
  /*
  @SuppressWarnings("unused")
  static std::wstring brToString(BytesRef b) {
    try {
      return b.utf8ToString() + " " + b;
    } catch (Throwable t) {
      // If BytesRef isn't actually UTF8, or it's eg a
      // prefix of UTF8 that ends mid-unicode-char, we
      // fallback to hex:
      return b.toString();
    }
  }
  */

  // Target's prefix matches this block's prefix; we
  // scan the entries check if the suffix matches.
public:
  SeekStatus scanToTermLeaf(std::shared_ptr<BytesRef> target,
                            bool exactOnly) ;

  // Target's prefix matches this block's prefix; we
  // scan the entries check if the suffix matches.
  SeekStatus scanToTermNonLeaf(std::shared_ptr<BytesRef> target,
                               bool exactOnly) ;

private:
  void fillTerm();
};

} // #include  "core/src/java/org/apache/lucene/codecs/blocktree/
