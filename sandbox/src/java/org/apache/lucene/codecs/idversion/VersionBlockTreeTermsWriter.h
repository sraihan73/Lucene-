#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
#include  "core/src/java/org/apache/lucene/codecs/blocktree/BlockTreeTermsWriter.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktree/FieldMetaData.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/codecs/blocktree/PendingBlock.h"
#include  "core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class Builder;
}
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktree/PendingEntry.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktree/PendingTerm.h"
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
namespace org::apache::lucene::codecs::idversion
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

/*
  TODO:

    - Currently there is a one-to-one mapping of indexed
      term to term block, but we could decouple the two, ie,
      put more terms into the index than there are blocks.
      The index would take up more RAM but then it'd be able
      to avoid seeking more often and could make PK/FuzzyQ
      faster if the additional indexed terms could store
      the offset into the terms block.

    - The blocks are not written in true depth-first
      order, meaning if you just next() the file pointer will
      sometimes jump backwards.  For example, block foo* will
      be written before block f* because it finished before.
      This could possibly hurt performance if the terms dict is
      not hot, since OSs anticipate sequential file access.  We
      could fix the writer to re-order the blocks as a 2nd
      pass.

    - Each block encodes the term suffixes packed
      sequentially using a separate vInt per term, which is
      1) wasteful and 2) slow (must linear scan to find a
      particular suffix).  We should instead 1) make
      random-access array so we can directly access the Nth
      suffix, and 2) bulk-encode this array using bulk int[]
      codecs; then at search time we can binary search when
      we seek a particular term.
*/

/**
 * This is just like {@link BlockTreeTermsWriter}, except it also stores a
 * version per term, and adds a method to its TermsEnum implementation to
 * seekExact only if the version is &gt;= the specified version.  The version is
 * added to the terms index to avoid seeking if no term in the block has a high
 * enough version.  The term blocks file is .tiv and the terms index extension
 * is .tipv.
 *
 * @lucene.experimental
 */

class VersionBlockTreeTermsWriter final : public FieldsConsumer
{
  GET_CLASS_NAME(VersionBlockTreeTermsWriter)

public:
  static const std::shared_ptr<
      PairOutputs<std::shared_ptr<BytesRef>, int64_t>>
      FST_OUTPUTS;

  static const std::shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>>
      NO_OUTPUT;

  /** Suggested default value for the {@code
   *  minItemsInBlock} parameter to {@link
   *  #VersionBlockTreeTermsWriter(SegmentWriteState,PostingsWriterBase,int,int)}.
   */
  static constexpr int DEFAULT_MIN_BLOCK_SIZE = 25;

  /** Suggested default value for the {@code
   *  maxItemsInBlock} parameter to {@link
   *  #VersionBlockTreeTermsWriter(SegmentWriteState,PostingsWriterBase,int,int)}.
   */
  static constexpr int DEFAULT_MAX_BLOCK_SIZE = 48;

  // public final static bool DEBUG = false;
  // private final static bool SAVE_DOT_FILES = false;

  static constexpr int OUTPUT_FLAGS_NUM_BITS = 2;
  static constexpr int OUTPUT_FLAGS_MASK = 0x3;
  static constexpr int OUTPUT_FLAG_IS_FLOOR = 0x1;
  static constexpr int OUTPUT_FLAG_HAS_TERMS = 0x2;

  /** Extension of terms file */
  static const std::wstring TERMS_EXTENSION;
  static const std::wstring TERMS_CODEC_NAME;

  /** Initial terms format. */
  static constexpr int VERSION_START = 1;

  /** Current terms format. */
  static constexpr int VERSION_CURRENT = VERSION_START;

  /** Extension of terms index file */
  static const std::wstring TERMS_INDEX_EXTENSION;
  static const std::wstring TERMS_INDEX_CODEC_NAME;

private:
  const std::shared_ptr<IndexOutput> out;
  const std::shared_ptr<IndexOutput> indexOut;

public:
  const int maxDoc;
  const int minItemsInBlock;
  const int maxItemsInBlock;

  const std::shared_ptr<PostingsWriterBase> postingsWriter;
  const std::shared_ptr<FieldInfos> fieldInfos;

private:
  class FieldMetaData : public std::enable_shared_from_this<FieldMetaData>
  {
    GET_CLASS_NAME(FieldMetaData)
  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    const std::shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>> rootCode;
    const int64_t numTerms;
    const int64_t indexStartFP;

  private:
    const int longsSize;

  public:
    const std::shared_ptr<BytesRef> minTerm;
    const std::shared_ptr<BytesRef> maxTerm;

    FieldMetaData(
        std::shared_ptr<FieldInfo> fieldInfo,
        std::shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>> rootCode,
        int64_t numTerms, int64_t indexStartFP, int longsSize,
        std::shared_ptr<BytesRef> minTerm, std::shared_ptr<BytesRef> maxTerm);
  };

private:
  const std::deque<std::shared_ptr<BlockTreeTermsWriter::FieldMetaData>>
      fields =
          std::deque<std::shared_ptr<BlockTreeTermsWriter::FieldMetaData>>();
  const std::wstring segment;

  /** Create a new writer.  The number of items (terms or
   *  sub-blocks) per block will aim to be between
   *  minItemsPerBlock and maxItemsPerBlock, though in some
   *  cases the blocks may be smaller than the min. */
public:
  VersionBlockTreeTermsWriter(
      std::shared_ptr<SegmentWriteState> state,
      std::shared_ptr<PostingsWriterBase> postingsWriter, int minItemsInBlock,
      int maxItemsInBlock) ;

  /** Writes the terms file trailer. */
private:
  void writeTrailer(std::shared_ptr<IndexOutput> out,
                    int64_t dirStart) ;

  /** Writes the index file trailer. */
  void writeIndexTrailer(std::shared_ptr<IndexOutput> indexOut,
                         int64_t dirStart) ;

public:
  void write(std::shared_ptr<Fields> fields)  override;

  static int64_t encodeOutput(int64_t fp, bool hasTerms, bool isFloor);

private:
  class PendingEntry : public std::enable_shared_from_this<PendingEntry>
  {
    GET_CLASS_NAME(PendingEntry)
  public:
    const bool isTerm;

  protected:
    PendingEntry(bool isTerm);
  };

private:
  class PendingTerm final : public PendingEntry
  {
    GET_CLASS_NAME(PendingTerm)
  public:
    std::deque<char> const termBytes;
    // stats + metadata
    const std::shared_ptr<BlockTermState> state;

    PendingTerm(std::shared_ptr<BytesRef> term,
                std::shared_ptr<BlockTermState> state);

    virtual std::wstring toString();

  protected:
    std::shared_ptr<PendingTerm> shared_from_this()
    {
      return std::static_pointer_cast<PendingTerm>(
          PendingEntry::shared_from_this());
    }
  };

  // for debugging
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring
  // brToString(org.apache.lucene.util.BytesRef b)
  static std::wstring brToString(std::shared_ptr<BytesRef> b);

  // for debugging
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") static std::wstring brToString(byte[]
  // b)
  static std::wstring brToString(std::deque<char> &b);

private:
  class PendingBlock final : public PendingEntry
  {
    GET_CLASS_NAME(PendingBlock)
  public:
    const std::shared_ptr<BytesRef> prefix;
    const int64_t fp;
    std::shared_ptr<FST<Pair<std::shared_ptr<BytesRef>, int64_t>>> index;
    std::deque<FST<Pair<std::shared_ptr<BytesRef>, int64_t>>> subIndices;
    const bool hasTerms;
    const bool isFloor;
    const int floorLeadByte;
    /** Max version for all terms in this block. */
  private:
    const int64_t maxVersion;

  public:
    PendingBlock(std::shared_ptr<BytesRef> prefix, int64_t maxVersion,
                 int64_t fp, bool hasTerms, bool isFloor, int floorLeadByte,
                 std::deque<FST<Pair<std::shared_ptr<BytesRef>, int64_t>>>
                     &subIndices);

    virtual std::wstring toString();

    void compileIndex(
        std::deque<std::shared_ptr<BlockTreeTermsWriter::PendingBlock>>
            &blocks,
        std::shared_ptr<RAMOutputStream> scratchBytes,
        std::shared_ptr<IntsRefBuilder> scratchIntsRef) ;

    // TODO: maybe we could add bulk-add method to
    // Builder?  Takes FST and unions it w/ current
    // FST.
  private:
    void
    append(std::shared_ptr<Builder<Pair<std::shared_ptr<BytesRef>, int64_t>>>
               builder,
           std::shared_ptr<FST<Pair<std::shared_ptr<BytesRef>, int64_t>>>
               subIndex,
           std::shared_ptr<IntsRefBuilder> scratchIntsRef) ;

  protected:
    std::shared_ptr<PendingBlock> shared_from_this()
    {
      return std::static_pointer_cast<PendingBlock>(
          PendingEntry::shared_from_this());
    }
  };

private:
  const std::shared_ptr<RAMOutputStream> scratchBytes =
      std::make_shared<RAMOutputStream>();
  const std::shared_ptr<IntsRefBuilder> scratchIntsRef =
      std::make_shared<IntsRefBuilder>();

public:
  class TermsWriter : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    std::shared_ptr<VersionBlockTreeTermsWriter> outerInstance;

    const std::shared_ptr<FieldInfo> fieldInfo;
    const int longsSize;
    int64_t numTerms = 0;

  public:
    const std::shared_ptr<FixedBitSet> docsSeen;
    int64_t indexStartFP = 0;

    // Records index into pending where the current prefix at that
    // length "started"; for example, if current term starts with 't',
    // startsByPrefix[0] is the index into pending for the first
    // term/sub-block starting with 't'.  We use this to figure out when
    // to write a new block:
  private:
    const std::shared_ptr<BytesRefBuilder> lastTerm =
        std::make_shared<BytesRefBuilder>();
    std::deque<int> prefixStarts = std::deque<int>(8);

    std::deque<int64_t> const longs;

    // Pending stack of terms and blocks.  As terms arrive (in sorted order)
    // we append to this stack, and once the top of the stack has enough
    // terms starting with a common prefix, we write a new block with
    // those terms and replace those terms in the stack with a new block:
    const std::deque<std::shared_ptr<BlockTreeTermsWriter::PendingEntry>>
        pending =
            std::deque<std::shared_ptr<BlockTreeTermsWriter::PendingEntry>>();

    // Reused in writeBlocks:
    const std::deque<std::shared_ptr<BlockTreeTermsWriter::PendingBlock>>
        newBlocks =
            std::deque<std::shared_ptr<BlockTreeTermsWriter::PendingBlock>>();

    std::shared_ptr<BlockTreeTermsWriter::PendingTerm> firstPendingTerm;
    std::shared_ptr<BlockTreeTermsWriter::PendingTerm> lastPendingTerm;

    /** Writes the top count entries in pending, using prevTerm to compute the
     * prefix. */
  public:
    virtual void writeBlocks(int prefixLength, int count) ;

    /** Writes the specified slice (start is inclusive, end is exclusive)
     *  from pending stack as a new block.  If isFloor is true, there
     *  were too many (more than maxItemsInBlock) entries sharing the
     *  same prefix, and so we broke it into multiple floor blocks where
     *  we record the starting label of the suffix of each floor block. */
  private:
    std::shared_ptr<BlockTreeTermsWriter::PendingBlock>
    writeBlock(int prefixLength, bool isFloor, int floorLeadLabel, int start,
               int end, bool hasTerms, bool hasSubBlocks) ;

  public:
    TermsWriter(std::shared_ptr<VersionBlockTreeTermsWriter> outerInstance,
                std::shared_ptr<FieldInfo> fieldInfo);

    /** Writes one term's worth of postings. */
    virtual void write(std::shared_ptr<BytesRef> text,
                       std::shared_ptr<TermsEnum> termsEnum) ;

    /** Pushes the new term to the top of the stack, and writes new blocks. */
  private:
    void pushTerm(std::shared_ptr<BytesRef> text) ;

    // Finishes all terms in this field
  public:
    virtual void finish() ;

  private:
    const std::shared_ptr<RAMOutputStream> suffixWriter =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> metaWriter =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> bytesWriter =
        std::make_shared<RAMOutputStream>();
  };

private:
  bool closed = false;

public:
  virtual ~VersionBlockTreeTermsWriter();

private:
  static void writeBytesRef(std::shared_ptr<IndexOutput> out,
                            std::shared_ptr<BytesRef> bytes) ;

protected:
  std::shared_ptr<VersionBlockTreeTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<VersionBlockTreeTermsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/idversion/
