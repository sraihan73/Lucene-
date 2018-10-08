#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/PostingsWriterBase.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/FieldMetaData.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include  "core/src/java/org/apache/lucene/util/fst/PositiveIntOutputs.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::codecs::memory
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;

/**
 * FST-based term dict, using ord as FST output.
 *
 * The FST holds the mapping between &lt;term, ord&gt;, and
 * term's metadata is delta encoded into a single byte block.
 *
 * Typically the byte block consists of four parts:
 * 1. term statistics: docFreq, totalTermFreq;
 * 2. monotonic long[], e.g. the pointer to the postings deque for that term;
 * 3. generic byte[], e.g. other information customized by postings base.
 * 4. single-level skip deque to speed up metadata decoding by ord.
 *
 * <p>
 * Files:
 * <ul>
 *  <li><tt>.tix</tt>: <a href="#Termindex">Term Index</a></li>
 *  <li><tt>.tbk</tt>: <a href="#Termblock">Term Block</a></li>
 * </ul>
 *
 * <a name="Termindex"></a>
 * <h3>Term Index</h3>
 * <p>
 *  The .tix contains a deque of FSTs, one for each field.
 *  The FST maps a term to its corresponding order in current field.
 * </p>
 *
 * <ul>
 *  <li>TermIndex(.tix) --&gt; Header, TermFST<sup>NumFields</sup>, Footer</li>
 *  <li>TermFST --&gt; {@link FST FST&lt;long&gt;}</li>
 *  <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 *  <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 *
 * <p>Notes:</p>
 * <ul>
 *  <li>
 *  Since terms are already sorted before writing to <a href="#Termblock">Term
 * Block</a>, their ords can directly used to seek term metadata from term
 * block.
 *  </li>
 * </ul>
 *
 * <a name="Termblock"></a>
 * <h3>Term Block</h3>
 * <p>
 *  The .tbk contains all the statistics and metadata for terms, along with
 * field summary (e.g. per-field data like number of documents in current
 * field). For each field, there are four blocks: <ul> <li>statistics bytes
 * block: contains term statistics; </li> <li>metadata longs block:
 * delta-encodes monotonic part of metadata; </li> <li>metadata bytes block:
 * encodes other parts of metadata; </li> <li>skip block: contains skip data, to
 * speed up metadata seeking and decoding</li>
 *  </ul>
 *
 * <p>File Format:</p>
 * <ul>
 *  <li>TermBlock(.tbk) --&gt; Header, <i>PostingsHeader</i>, FieldSummary,
 * DirOffset</li> <li>FieldSummary --&gt; NumFields, &lt;FieldNumber, NumTerms,
 * SumTotalTermFreq?, SumDocFreq, DocCount, LongsSize, DataBlock &gt;
 * <sup>NumFields</sup>, Footer</li>
 *
 *  <li>DataBlock --&gt; StatsBlockLength, MetaLongsBlockLength,
 * MetaBytesBlockLength, SkipBlock, StatsBlock, MetaLongsBlock, MetaBytesBlock
 * </li> <li>SkipBlock --&gt; &lt; StatsFPDelta, MetaLongsSkipFPDelta,
 * MetaBytesSkipFPDelta, MetaLongsSkipDelta<sup>LongsSize</sup>
 * &gt;<sup>NumTerms</sup> <li>StatsBlock --&gt; &lt; DocFreq[Same?],
 * (TotalTermFreq-DocFreq) ? &gt; <sup>NumTerms</sup> <li>MetaLongsBlock --&gt;
 * &lt; LongDelta<sup>LongsSize</sup>, BytesSize &gt; <sup>NumTerms</sup>
 *  <li>MetaBytesBlock --&gt; Byte <sup>MetaBytesBlockLength</sup>
 *  <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 *  <li>DirOffset --&gt; {@link DataOutput#writeLong Uint64}</li>
 *  <li>NumFields, FieldNumber, DocCount, DocFreq, LongsSize,
 *        FieldNumber, DocCount --&gt; {@link DataOutput#writeVInt VInt}</li>
 *  <li>NumTerms, SumTotalTermFreq, SumDocFreq, StatsBlockLength,
 * MetaLongsBlockLength, MetaBytesBlockLength, StatsFPDelta,
 * MetaLongsSkipFPDelta, MetaBytesSkipFPDelta, MetaLongsSkipStart,
 * TotalTermFreq, LongDelta,--&gt; {@link DataOutput#writeVLong VLong}</li>
 *  <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * <p>Notes: </p>
 * <ul>
 *  <li>
 *   The format of PostingsHeader and MetaBytes are customized by the specific
 * postings implementation: they contain arbitrary per-file data (such as
 * parameters or versioning information), and per-term data (non-monotonic ones
 * like pulsed postings data).
 *  </li>
 *  <li>
 *   During initialization the reader will load all the blocks into memory.
 * SkipBlock will be decoded, so that during seek term dict can lookup file
 * pointers directly. StatsFPDelta, MetaLongsSkipFPDelta, etc. are file offset
 *   for every SkipInterval's term. MetaLongsSkipDelta is the difference from
 * previous one, which indicates the value of preceding metadata longs for every
 * SkipInterval's term.
 *  </li>
 *  <li>
 *   DocFreq is the count of documents which contain the term. TotalTermFreq is
 * the total number of occurrences of the term. Usually these two values are the
 * same for long tail terms, therefore one bit is stole from DocFreq to check
 * this case, so that encoding of TotalTermFreq may be omitted.
 *  </li>
 * </ul>
 *
 * @lucene.experimental
 */

class FSTOrdTermsWriter : public FieldsConsumer
{
  GET_CLASS_NAME(FSTOrdTermsWriter)
public:
  static const std::wstring TERMS_INDEX_EXTENSION;
  static const std::wstring TERMS_BLOCK_EXTENSION;
  static const std::wstring TERMS_CODEC_NAME;
  static const std::wstring TERMS_INDEX_CODEC_NAME;

  static constexpr int VERSION_START = 2;
  static constexpr int VERSION_CURRENT = VERSION_START;
  static constexpr int SKIP_INTERVAL = 8;

  const std::shared_ptr<PostingsWriterBase> postingsWriter;
  const std::shared_ptr<FieldInfos> fieldInfos;
  const int maxDoc;
  const std::deque<std::shared_ptr<FieldMetaData>> fields =
      std::deque<std::shared_ptr<FieldMetaData>>();
  std::shared_ptr<IndexOutput> blockOut = nullptr;
  std::shared_ptr<IndexOutput> indexOut = nullptr;

  FSTOrdTermsWriter(
      std::shared_ptr<SegmentWriteState> state,
      std::shared_ptr<PostingsWriterBase> postingsWriter) ;

  void write(std::shared_ptr<Fields> fields)  override;

  virtual ~FSTOrdTermsWriter();

private:
  void writeTrailer(std::shared_ptr<IndexOutput> out,
                    int64_t dirStart) ;

private:
  class FieldMetaData : public std::enable_shared_from_this<FieldMetaData>
  {
    GET_CLASS_NAME(FieldMetaData)
  public:
    std::shared_ptr<FieldInfo> fieldInfo;
    int64_t numTerms = 0;
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    int docCount = 0;
    int longsSize = 0;
    std::shared_ptr<FST<int64_t>> dict;

    // TODO: block encode each part

    // vint encode next skip point (fully decoded when reading)
    std::shared_ptr<RAMOutputStream> skipOut;
    // vint encode df, (ttf-df)
    std::shared_ptr<RAMOutputStream> statsOut;
    // vint encode monotonic long[] and length for corresponding byte[]
    std::shared_ptr<RAMOutputStream> metaLongsOut;
    // generic byte[]
    std::shared_ptr<RAMOutputStream> metaBytesOut;
  };

public:
  class TermsWriter final : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    std::shared_ptr<FSTOrdTermsWriter> outerInstance;

    const std::shared_ptr<Builder<int64_t>> builder;
    const std::shared_ptr<PositiveIntOutputs> outputs;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int longsSize;
    int64_t numTerms = 0;

    const std::shared_ptr<IntsRefBuilder> scratchTerm =
        std::make_shared<IntsRefBuilder>();
    const std::shared_ptr<RAMOutputStream> statsOut =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> metaLongsOut =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<RAMOutputStream> metaBytesOut =
        std::make_shared<RAMOutputStream>();

    const std::shared_ptr<RAMOutputStream> skipOut =
        std::make_shared<RAMOutputStream>();
    int64_t lastBlockStatsFP = 0;
    int64_t lastBlockMetaLongsFP = 0;
    int64_t lastBlockMetaBytesFP = 0;
    std::deque<int64_t> lastBlockLongs;

    std::deque<int64_t> lastLongs;
    int64_t lastMetaBytesFP = 0;

  public:
    TermsWriter(std::shared_ptr<FSTOrdTermsWriter> outerInstance,
                std::shared_ptr<FieldInfo> fieldInfo);

    void finishTerm(std::shared_ptr<BytesRef> text,
                    std::shared_ptr<BlockTermState> state) ;

    void finish(int64_t sumTotalTermFreq, int64_t sumDocFreq,
                int docCount) ;

  private:
    void bufferSkip() ;
  };

protected:
  std::shared_ptr<FSTOrdTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<FSTOrdTermsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/memory/
