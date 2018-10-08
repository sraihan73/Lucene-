#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexOutput;
}

namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::fst
{
class ByteSequenceOutputs;
}
namespace org::apache::lucene::store
{
class RAMOutputStream;
}
namespace org::apache::lucene::util
{
class IntsRefBuilder;
}
namespace org::apache::lucene::codecs
{
class TermStats;
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
class FieldsConsumer;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class BytesRefFSTEnum;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TermStats = org::apache::lucene::codecs::TermStats;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;

// TODO: would be nice to somehow allow this to act like
// InstantiatedIndex, by never writing to disk; ie you write
// to this Codec in RAM only and then when you open a reader
// it pulls the FST directly from what you wrote w/o going
// to disk.

/** Stores terms and postings (docs, positions, payloads) in
 *  RAM, using an FST.
 *
 * <p>Note that this codec implements advance as a linear
 * scan!  This means if you store large fields in here,
 * queries that rely on advance will (AND BooleanQuery,
 * PhraseQuery) will be relatively slow!
 *
 * @lucene.experimental */

// TODO: Maybe name this 'Cached' or something to reflect
// the reality that it is actually written to disk, but
// loads itself in ram?
class MemoryPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(MemoryPostingsFormat)

public:
  MemoryPostingsFormat();

  /**
   * Create MemoryPostingsFormat, specifying advanced FST options.
   * @param doPackFST true if a packed FST should be built.
   *        NOTE: packed FSTs are limited to ~2.1 GB of postings.
   * @param acceptableOverheadRatio allowable overhead for packed ints
   *        during FST construction.
   */
  MemoryPostingsFormat(bool doPackFST, float acceptableOverheadRatio);

  virtual std::wstring toString();

private:
  class TermsWriter final : public std::enable_shared_from_this<TermsWriter>
  {
    GET_CLASS_NAME(TermsWriter)
  private:
    const std::shared_ptr<IndexOutput> out;
    const std::shared_ptr<FieldInfo> field;
    const std::shared_ptr<Builder<std::shared_ptr<BytesRef>>> builder;
    const std::shared_ptr<ByteSequenceOutputs> outputs =
        ByteSequenceOutputs::getSingleton();
    int termCount = 0;

  public:
    TermsWriter(std::shared_ptr<IndexOutput> out,
                std::shared_ptr<FieldInfo> field);

  private:
    class PostingsWriter : public std::enable_shared_from_this<PostingsWriter>
    {
      GET_CLASS_NAME(PostingsWriter)
    private:
      std::shared_ptr<MemoryPostingsFormat::TermsWriter> outerInstance;

    public:
      PostingsWriter(
          std::shared_ptr<MemoryPostingsFormat::TermsWriter> outerInstance);

    private:
      int lastDocID = 0;
      int lastPos = 0;
      int lastPayloadLen = 0;

      // NOTE: not private so we don't pay access check at runtime:
    public:
      int docCount = 0;
      std::shared_ptr<RAMOutputStream> buffer =
          std::make_shared<RAMOutputStream>();

      int lastOffsetLength = 0;
      int lastOffset = 0;

      virtual void startDoc(int docID, int termDocFreq) ;

      virtual void addPosition(int pos, std::shared_ptr<BytesRef> payload,
                               int startOffset,
                               int endOffset) ;

      virtual std::shared_ptr<PostingsWriter> reset();
    };

  public:
    const std::shared_ptr<PostingsWriter> postingsWriter =
        std::make_shared<PostingsWriter>(shared_from_this());

  private:
    const std::shared_ptr<RAMOutputStream> buffer2 =
        std::make_shared<RAMOutputStream>();
    const std::shared_ptr<BytesRef> spare = std::make_shared<BytesRef>();
    std::deque<char> finalBuffer = std::deque<char>(128);

    const std::shared_ptr<IntsRefBuilder> scratchIntsRef =
        std::make_shared<IntsRefBuilder>();

    void finishTerm(std::shared_ptr<BytesRef> text,
                    std::shared_ptr<TermStats> stats) ;

  public:
    void finish(int64_t sumTotalTermFreq, int64_t sumDocFreq,
                int docCount) ;
  };

private:
  static std::wstring EXTENSION;
  static const std::wstring CODEC_NAME;
  static constexpr int VERSION_START = 1;
  static constexpr int VERSION_CURRENT = VERSION_START;

private:
  class MemoryFieldsConsumer : public FieldsConsumer
  {
    GET_CLASS_NAME(MemoryFieldsConsumer)
  private:
    const std::shared_ptr<SegmentWriteState> state;
    const std::shared_ptr<IndexOutput> out;

    MemoryFieldsConsumer(std::shared_ptr<SegmentWriteState> state) throw(
        IOException);

  public:
    void write(std::shared_ptr<Fields> fields)  override;

  private:
    bool closed = false;

  public:
    virtual ~MemoryFieldsConsumer();

  protected:
    std::shared_ptr<MemoryFieldsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<MemoryFieldsConsumer>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

public:
  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

private:
  class FSTDocsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(FSTDocsEnum)
  private:
    const IndexOptions indexOptions;
    const bool storePayloads;
    std::deque<char> buffer = std::deque<char>(16);
    const std::shared_ptr<ByteArrayDataInput> in_ =
        std::make_shared<ByteArrayDataInput>(buffer);

    int docUpto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int accum = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;
    int payloadLen = 0;
    int numDocs = 0;

  public:
    FSTDocsEnum(IndexOptions indexOptions, bool storePayloads);

    bool canReuse(IndexOptions indexOptions, bool storePayloads);

    std::shared_ptr<FSTDocsEnum> reset(std::shared_ptr<BytesRef> bufferIn,
                                       int numDocs);

    int nextDoc() override;

    int docID() override;

    int advance(int target)  override;

    int freq() override;

    int64_t cost() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

  protected:
    std::shared_ptr<FSTDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FSTDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  class FSTPostingsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(FSTPostingsEnum)
  private:
    const bool storePayloads;
    std::deque<char> buffer = std::deque<char>(16);
    const std::shared_ptr<ByteArrayDataInput> in_ =
        std::make_shared<ByteArrayDataInput>(buffer);

    int docUpto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int accum = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;
    int numDocs = 0;
    int posPending = 0;
    int payloadLength = 0;

  public:
    const bool storeOffsets;
    int offsetLength = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;

  private:
    int pos = 0;
    const std::shared_ptr<BytesRef> payload = std::make_shared<BytesRef>();

  public:
    FSTPostingsEnum(bool storePayloads, bool storeOffsets);

    bool canReuse(bool storePayloads, bool storeOffsets);

    std::shared_ptr<FSTPostingsEnum> reset(std::shared_ptr<BytesRef> bufferIn,
                                           int numDocs);

    int nextDoc() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int docID() override;

    int advance(int target)  override;

    int freq() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<FSTPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FSTPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  class FSTTermsEnum final : public TermsEnum
  {
    GET_CLASS_NAME(FSTTermsEnum)
  private:
    const std::shared_ptr<FieldInfo> field;
    const std::shared_ptr<BytesRefFSTEnum<std::shared_ptr<BytesRef>>> fstEnum;
    const std::shared_ptr<ByteArrayDataInput> buffer =
        std::make_shared<ByteArrayDataInput>();
    bool didDecode = false;

    // C++ NOTE: Fields cannot have the same name as methods:
    int docFreq_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t totalTermFreq_ = 0;
    std::shared_ptr<BytesRefFSTEnum::InputOutput<std::shared_ptr<BytesRef>>>
        current;
    std::shared_ptr<BytesRef> postingsSpare = std::make_shared<BytesRef>();

  public:
    FSTTermsEnum(std::shared_ptr<FieldInfo> field,
                 std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst);

  private:
    void decodeMetaData();

  public:
    bool seekExact(std::shared_ptr<BytesRef> text)  override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                           int flags) override;

    std::shared_ptr<BytesRef> term() override;

    std::shared_ptr<BytesRef> next()  override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    void seekExact(int64_t ord) override;

    int64_t ord() override;

  protected:
    std::shared_ptr<FSTTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FSTTermsEnum>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

private:
  class TermsReader final : public Terms, public Accountable
  {
    GET_CLASS_NAME(TermsReader)

  private:
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;
    const int termCount;
    std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst;
    const std::shared_ptr<ByteSequenceOutputs> outputs =
        ByteSequenceOutputs::getSingleton();
    const std::shared_ptr<FieldInfo> field;

  public:
    TermsReader(std::shared_ptr<FieldInfos> fieldInfos,
                std::shared_ptr<IndexInput> in_,
                int termCount) ;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq() override;

    int getDocCount() override;

    int64_t size() override;

    std::shared_ptr<TermsEnum> iterator() override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<TermsReader> shared_from_this()
    {
      return std::static_pointer_cast<TermsReader>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

public:
  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

private:
  class FieldsProducerAnonymousInnerClass : public FieldsProducer
  {
    GET_CLASS_NAME(FieldsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<MemoryPostingsFormat> outerInstance;

    std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<TermsReader>>>
        fields;

  public:
    FieldsProducerAnonymousInnerClass(
        std::shared_ptr<MemoryPostingsFormat> outerInstance,
        std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<TermsReader>>>
            fields);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

    virtual ~FieldsProducerAnonymousInnerClass();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();

    void checkIntegrity()  override;

  protected:
    std::shared_ptr<FieldsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FieldsProducerAnonymousInnerClass>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MemoryPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<MemoryPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
