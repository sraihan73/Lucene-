#pragma once
#include "../../util/fst/FST.h"
#include "../FieldsConsumer.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::codecs
{
class PostingsWriterBase;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
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
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::codecs
{
class BlockTermState;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::store
{
class RAMOutputStream;
}
namespace org::apache::lucene::util
{
class IntsRefBuilder;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Builder;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::codecs::blocktree
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
using FST = org::apache::lucene::util::fst::FST;

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
 * Block-based terms index and dictionary writer.
 * <p>
 * Writes terms dict and index, block-encoding (column
 * stride) each term's metadata for each set of terms
 * between two index terms.
 * <p>
 *
 * Files:
 * <ul>
 *   <li><tt>.tim</tt>: <a href="#Termdictionary">Term Dictionary</a></li>
 *   <li><tt>.tip</tt>: <a href="#Termindex">Term Index</a></li>
 * </ul>
 * <p>
 * <a name="Termdictionary"></a>
 * <h3>Term Dictionary</h3>
 *
 * <p>The .tim file contains the deque of terms in each
 * field along with per-term statistics (such as docfreq)
 * and per-term metadata (typically pointers to the postings deque
 * for that term in the inverted index).
 * </p>
 *
 * <p>The .tim is arranged in blocks: with blocks containing
 * a variable number of entries (by default 25-48), where
 * each entry is either a term or a reference to a
 * sub-block.</p>
 *
 * <p>NOTE: The term dictionary can plug into different postings
 * implementations: the postings writer/reader are actually responsible for
 * encoding and decoding the Postings Metadata and Term Metadata sections.</p>
 *
 * <ul>
 *    <li>TermsDict (.tim) --&gt; Header, <i>PostingsHeader</i>,
 * NodeBlock<sup>NumBlocks</sup>, FieldSummary, DirOffset, Footer</li>
 *    <li>NodeBlock --&gt; (OuterNode | InnerNode)</li>
 *    <li>OuterNode --&gt; EntryCount, SuffixLength,
 * Byte<sup>SuffixLength</sup>, StatsLength, &lt; TermStats
 * &gt;<sup>EntryCount</sup>, MetaLength,
 * &lt;<i>TermMetadata</i>&gt;<sup>EntryCount</sup></li> <li>InnerNode --&gt;
 * EntryCount, SuffixLength[,Sub?], Byte<sup>SuffixLength</sup>, StatsLength,
 * &lt; TermStats ? &gt;<sup>EntryCount</sup>, MetaLength, &lt;<i>TermMetadata ?
 * </i>&gt;<sup>EntryCount</sup></li> <li>TermStats --&gt; DocFreq,
 * TotalTermFreq </li> <li>FieldSummary --&gt; NumFields, &lt;FieldNumber,
 * NumTerms, RootCodeLength, Byte<sup>RootCodeLength</sup>, SumTotalTermFreq?,
 * SumDocFreq, DocCount, LongsSize, MinTerm,
 * MaxTerm&gt;<sup>NumFields</sup></li> <li>Header --&gt; {@link
 * CodecUtil#writeHeader CodecHeader}</li> <li>DirOffset --&gt; {@link
 * DataOutput#writeLong Uint64}</li> <li>MinTerm,MaxTerm --&gt; {@link
 * DataOutput#writeVInt VInt} length followed by the byte[]</li>
 *    <li>EntryCount,SuffixLength,StatsLength,DocFreq,MetaLength,NumFields,
 *        FieldNumber,RootCodeLength,DocCount,LongsSize --&gt; {@link
 * DataOutput#writeVInt VInt}</li>
 *    <li>TotalTermFreq,NumTerms,SumTotalTermFreq,SumDocFreq --&gt;
 *        {@link DataOutput#writeVLong VLong}</li>
 *    <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *    <li>Header is a {@link CodecUtil#writeHeader CodecHeader} storing the
 * version information for the BlockTree implementation.</li> <li>DirOffset is a
 * pointer to the FieldSummary section.</li> <li>DocFreq is the count of
 * documents which contain the term.</li> <li>TotalTermFreq is the total number
 * of occurrences of the term. This is encoded as the difference between the
 * total number of occurrences and the DocFreq.</li> <li>FieldNumber is the
 * fields number from {@link FieldInfos}. (.fnm)</li> <li>NumTerms is the number
 * of unique terms for the field.</li> <li>RootCode points to the root block for
 * the field.</li> <li>SumDocFreq is the total number of postings, the number of
 * term-document pairs across the entire field.</li> <li>DocCount is the number
 * of documents that have at least one posting for this field.</li>
 *    <li>LongsSize records how many long values the postings writer/reader
 * record per term (e.g., to hold freq/prox/doc file offsets). <li>MinTerm,
 * MaxTerm are the lowest and highest term in this field.</li>
 *    <li>PostingsHeader and TermMetadata are plugged into by the specific
 * postings implementation: these contain arbitrary per-file data (such as
 * parameters or versioning information) and per-term data (such as pointers to
 * inverted files).</li> <li>For inner nodes of the tree, every entry will steal
 * one bit to mark whether it points to child nodes(sub-block). If so, the
 * corresponding TermStats and TermMetaData are omitted </li>
 * </ul>
 * <a name="Termindex"></a>
 * <h3>Term Index</h3>
 * <p>The .tip file contains an index into the term dictionary, so that it can
 * be accessed randomly.  The index is also used to determine when a given term
 * cannot exist on disk (in the .tim file), saving a disk seek.</p> <ul>
 *   <li>TermsIndex (.tip) --&gt; Header, FSTIndex<sup>NumFields</sup>
 *                                &lt;IndexStartFP&gt;<sup>NumFields</sup>,
 * DirOffset, Footer</li> <li>Header --&gt; {@link CodecUtil#writeHeader
 * CodecHeader}</li> <li>DirOffset --&gt; {@link DataOutput#writeLong
 * Uint64}</li> <li>IndexStartFP --&gt; {@link DataOutput#writeVLong VLong}</li>
 *   <!-- TODO: better describe FST output here -->
 *   <li>FSTIndex --&gt; {@link FST FST&lt;byte[]&gt;}</li>
 *   <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *   <li>The .tip file contains a separate FST for each
 *       field.  The FST maps a term prefix to the on-disk
 *       block that holds all terms starting with that
 *       prefix.  Each field's IndexStartFP points to its
 *       FST.</li>
 *   <li>DirOffset is a pointer to the start of the IndexStartFPs
 *       for all fields</li>
 *   <li>It's possible that an on-disk block would contain
 *       too many terms (more than the allowed maximum
 *       (default: 48)).  When this happens, the block is
 *       sub-divided into new blocks (called "floor
 *       blocks"), and then the output in the FST for the
 *       block's prefix encodes the leading byte of each
 *       sub-block, and its file pointer.
 * </ul>
 *
 * @see BlockTreeTermsReader
 * @lucene.experimental
 */
class BlockTreeTermsWriter final : public FieldsConsumer
{
  GET_CLASS_NAME(BlockTreeTermsWriter)

  /** Suggested default value for the {@code
   *  minItemsInBlock} parameter to {@link
   *  #BlockTreeTermsWriter(SegmentWriteState,PostingsWriterBase,int,int)}. */
public:
  static constexpr int DEFAULT_MIN_BLOCK_SIZE = 25;

  /** Suggested default value for the {@code
   *  maxItemsInBlock} parameter to {@link
   *  #BlockTreeTermsWriter(SegmentWriteState,PostingsWriterBase,int,int)}. */
  static constexpr int DEFAULT_MAX_BLOCK_SIZE = 48;

  // public static bool DEBUG = false;
  // public static bool DEBUG2 = false;

  // private final static bool SAVE_DOT_FILES = false;

private:
  const std::shared_ptr<IndexOutput> termsOut;
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
    const std::shared_ptr<BytesRef> rootCode;
    const int64_t numTerms;
    const int64_t indexStartFP;
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;

  private:
    const int longsSize;

  public:
    const std::shared_ptr<BytesRef> minTerm;
    const std::shared_ptr<BytesRef> maxTerm;

    FieldMetaData(std::shared_ptr<FieldInfo> fieldInfo,
                  std::shared_ptr<BytesRef> rootCode, int64_t numTerms,
                  int64_t indexStartFP, int64_t sumTotalTermFreq,
                  int64_t sumDocFreq, int docCount, int longsSize,
                  std::shared_ptr<BytesRef> minTerm,
                  std::shared_ptr<BytesRef> maxTerm);
  };

private:
  const std::deque<std::shared_ptr<FieldMetaData>> fields =
      std::deque<std::shared_ptr<FieldMetaData>>();

  /** Create a new writer.  The number of items (terms or
   *  sub-blocks) per block will aim to be between
   *  minItemsPerBlock and maxItemsPerBlock, though in some
   *  cases the blocks may be smaller than the min. */
public:
  BlockTreeTermsWriter(std::shared_ptr<SegmentWriteState> state,
                       std::shared_ptr<PostingsWriterBase> postingsWriter,
                       int minItemsInBlock,
                       int maxItemsInBlock) ;

  /** Writes the terms file trailer. */
private:
  void writeTrailer(std::shared_ptr<IndexOutput> out,
                    int64_t dirStart) ;

  /** Writes the index file trailer. */
  void writeIndexTrailer(std::shared_ptr<IndexOutput> indexOut,
                         int64_t dirStart) ;

  /** Throws {@code IllegalArgumentException} if any of these settings
   *  is invalid. */
public:
  static void validateSettings(int minItemsInBlock, int maxItemsInBlock);

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
    std::shared_ptr<FST<std::shared_ptr<BytesRef>>> index;
    std::deque<FST<std::shared_ptr<BytesRef>>> subIndices;
    const bool hasTerms;
    const bool isFloor;
    const int floorLeadByte;

    PendingBlock(std::shared_ptr<BytesRef> prefix, int64_t fp, bool hasTerms,
                 bool isFloor, int floorLeadByte,
                 std::deque<FST<std::shared_ptr<BytesRef>>> &subIndices);

    virtual std::wstring toString();

    void compileIndex(
        std::deque<std::shared_ptr<PendingBlock>> &blocks,
        std::shared_ptr<RAMOutputStream> scratchBytes,
        std::shared_ptr<IntsRefBuilder> scratchIntsRef) ;

    // TODO: maybe we could add bulk-add method to
    // Builder?  Takes FST and unions it w/ current
    // FST.
  private:
    void
    append(std::shared_ptr<Builder<std::shared_ptr<BytesRef>>> builder,
           std::shared_ptr<FST<std::shared_ptr<BytesRef>>> subIndex,
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
  static const std::shared_ptr<BytesRef> EMPTY_BYTES_REF;

public:
  class TermsWriter : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    std::shared_ptr<BlockTreeTermsWriter> outerInstance;

    const std::shared_ptr<FieldInfo> fieldInfo;
    const int longsSize;
    int64_t numTerms = 0;

  public:
    const std::shared_ptr<FixedBitSet> docsSeen;
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
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
    const std::deque<std::shared_ptr<PendingEntry>> pending =
        std::deque<std::shared_ptr<PendingEntry>>();

    // Reused in writeBlocks:
    const std::deque<std::shared_ptr<PendingBlock>> newBlocks =
        std::deque<std::shared_ptr<PendingBlock>>();

    std::shared_ptr<PendingTerm> firstPendingTerm;
    std::shared_ptr<PendingTerm> lastPendingTerm;

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
    std::shared_ptr<PendingBlock>
    writeBlock(int prefixLength, bool isFloor, int floorLeadLabel, int start,
               int end, bool hasTerms, bool hasSubBlocks) ;

  public:
    TermsWriter(std::shared_ptr<BlockTreeTermsWriter> outerInstance,
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
    const std::shared_ptr<RAMOutputStream> statsWriter =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> metaWriter =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> bytesWriter =
        std::make_shared<RAMOutputStream>();
  };

private:
  bool closed = false;

public:
  virtual ~BlockTreeTermsWriter();

private:
  static void writeBytesRef(std::shared_ptr<IndexOutput> out,
                            std::shared_ptr<BytesRef> bytes) ;

protected:
  std::shared_ptr<BlockTreeTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<BlockTreeTermsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blocktree
