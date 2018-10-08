#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsWriterBase;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::codecs::memory
{
class FieldMetaData;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::codecs::memory
{
class FSTTermOutputs;
}
namespace org::apache::lucene::codecs::memory
{
class TermData;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util
{
class IntsRefBuilder;
}
namespace org::apache::lucene::store
{
class RAMOutputStream;
}
namespace org::apache::lucene::codecs
{
class BlockTermState;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

/**
 * FST-based term dict, using metadata as FST output.
 *
 * The FST directly holds the mapping between &lt;term, metadata&gt;.
 *
 * Term metadata consists of three parts:
 * 1. term statistics: docFreq, totalTermFreq;
 * 2. monotonic long[], e.g. the pointer to the postings deque for that term;
 * 3. generic byte[], e.g. other information need by postings reader.
 *
 * <p>
 * File:
 * <ul>
 *   <li><tt>.tst</tt>: <a href="#Termdictionary">Term Dictionary</a></li>
 * </ul>
 * <p>
 *
 * <a name="Termdictionary"></a>
 * <h3>Term Dictionary</h3>
 * <p>
 *  The .tst contains a deque of FSTs, one for each field.
 *  The FST maps a term to its corresponding statistics (e.g. docfreq)
 *  and metadata (e.g. information for postings deque reader like file pointer
 *  to postings deque).
 * </p>
 * <p>
 *  Typically the metadata is separated into two parts:
 *  <ul>
 *   <li>
 *    Monotonical long array: Some metadata will always be ascending in order
 *    with the corresponding term. This part is used by FST to share outputs
 * between arcs.
 *   </li>
 *   <li>
 *    Generic byte array: Used to store non-monotonic metadata.
 *   </li>
 *  </ul>
 *
 * File format:
 * <ul>
 *  <li>TermsDict(.tst) --&gt; Header, <i>PostingsHeader</i>, FieldSummary,
 * DirOffset</li> <li>FieldSummary --&gt; NumFields, &lt;FieldNumber, NumTerms,
 * SumTotalTermFreq?, SumDocFreq, DocCount, LongsSize, TermFST
 * &gt;<sup>NumFields</sup></li> <li>TermFST --&gt; {@link FST
 * FST&lt;TermData&gt;}</li> <li>TermData --&gt; Flag, BytesSize?,
 * LongDelta<sup>LongsSize</sup>?, Byte<sup>BytesSize</sup>?, &lt;
 * DocFreq[Same?], (TotalTermFreq-DocFreq) &gt; ? </li> <li>Header --&gt; {@link
 * CodecUtil#writeIndexHeader IndexHeader}</li> <li>DirOffset --&gt; {@link
 * DataOutput#writeLong Uint64}</li> <li>DocFreq, LongsSize, BytesSize,
 * NumFields, FieldNumber, DocCount --&gt; {@link DataOutput#writeVInt
 * VInt}</li> <li>TotalTermFreq, NumTerms, SumTotalTermFreq, SumDocFreq,
 * LongDelta --&gt;
 *        {@link DataOutput#writeVLong VLong}</li>
 * </ul>
 * <p>Notes:</p>
 * <ul>
 *  <li>
 *   The format of PostingsHeader and generic meta bytes are customized by the
 * specific postings implementation: they contain arbitrary per-file data (such
 * as parameters or versioning information), and per-term data (non-monotonic
 * ones like pulsed postings data).
 *  </li>
 *  <li>
 *   The format of TermData is determined by FST, typically monotonic metadata
 * will be dense around shallow arcs, while in deeper arcs only generic bytes
 * and term statistics exist.
 *  </li>
 *  <li>
 *   The byte Flag is used to indicate which part of metadata exists on current
 * arc. Specially the monotonic part is omitted when it is an array of 0s.
 *  </li>
 *  <li>
 *   Since LongsSize is per-field fixed, it is only written once in field
 * summary.
 *  </li>
 * </ul>
 *
 * @lucene.experimental
 */

class FSTTermsWriter : public FieldsConsumer
{
  GET_CLASS_NAME(FSTTermsWriter)
public:
  static const std::wstring TERMS_EXTENSION;
  static const std::wstring TERMS_CODEC_NAME;
  static constexpr int TERMS_VERSION_START = 2;
  static constexpr int TERMS_VERSION_CURRENT = TERMS_VERSION_START;

  const std::shared_ptr<PostingsWriterBase> postingsWriter;
  const std::shared_ptr<FieldInfos> fieldInfos;
  std::shared_ptr<IndexOutput> out;
  const int maxDoc;
  const std::deque<std::shared_ptr<FieldMetaData>> fields =
      std::deque<std::shared_ptr<FieldMetaData>>();

  FSTTermsWriter(
      std::shared_ptr<SegmentWriteState> state,
      std::shared_ptr<PostingsWriterBase> postingsWriter) ;

private:
  void writeTrailer(std::shared_ptr<IndexOutput> out,
                    int64_t dirStart) ;

public:
  void write(std::shared_ptr<Fields> fields)  override;

  virtual ~FSTTermsWriter();

private:
  class FieldMetaData : public std::enable_shared_from_this<FieldMetaData>
  {
    GET_CLASS_NAME(FieldMetaData)
  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int64_t numTerms;
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;
    const int longsSize;
    const std::shared_ptr<FST<std::shared_ptr<FSTTermOutputs::TermData>>> dict;

    FieldMetaData(
        std::shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
        int64_t sumTotalTermFreq, int64_t sumDocFreq, int docCount,
        int longsSize,
        std::shared_ptr<FST<std::shared_ptr<FSTTermOutputs::TermData>>> fst);
  };

public:
  class TermsWriter final : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    std::shared_ptr<FSTTermsWriter> outerInstance;

    const std::shared_ptr<Builder<std::shared_ptr<FSTTermOutputs::TermData>>>
        builder;
    const std::shared_ptr<FSTTermOutputs> outputs;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int longsSize;
    int64_t numTerms = 0;

    const std::shared_ptr<IntsRefBuilder> scratchTerm =
        std::make_shared<IntsRefBuilder>();
    const std::shared_ptr<RAMOutputStream> metaWriter =
        std::make_shared<RAMOutputStream>();

  public:
    TermsWriter(std::shared_ptr<FSTTermsWriter> outerInstance,
                std::shared_ptr<FieldInfo> fieldInfo);

    void finishTerm(std::shared_ptr<BytesRef> text,
                    std::shared_ptr<BlockTermState> state) ;

    void finish(int64_t sumTotalTermFreq, int64_t sumDocFreq,
                int docCount) ;
  };

protected:
  std::shared_ptr<FSTTermsWriter> shared_from_this()
  {
    return std::static_pointer_cast<FSTTermsWriter>(
        org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
