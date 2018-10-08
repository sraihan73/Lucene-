#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "FSTOrdTermsWriter.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/memory/TermsReader.h"

#include  "core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include  "core/src/java/org/apache/lucene/index/TermState.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class BytesRefFSTEnum;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class InputOutput;
}
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/codecs/memory/Frame.h"
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"
#include  "core/src/java/org/apache/lucene/util/automaton/ByteRunAutomaton.h"

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
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using ByteRunAutomaton = org::apache::lucene::util::automaton::ByteRunAutomaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using org::apache::lucene::util::fst::BytesRefFSTEnum::InputOutput;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;

/**
 * FST-based terms dictionary reader.
 *
 * The FST index maps each term and its ord, and during seek
 * the ord is used fetch metadata from a single block.
 * The term dictionary is fully memory resident.
 *
 * @lucene.experimental
 */
class FSTOrdTermsReader : public FieldsProducer
{
  GET_CLASS_NAME(FSTOrdTermsReader)
public:
  static constexpr int INTERVAL = FSTOrdTermsWriter::SKIP_INTERVAL;
  const std::map_obj<std::wstring, std::shared_ptr<TermsReader>> fields =
      std::map_obj<std::wstring, std::shared_ptr<TermsReader>>();
  const std::shared_ptr<PostingsReaderBase> postingsReader;
  // static final bool TEST = false;

  FSTOrdTermsReader(
      std::shared_ptr<SegmentReadState> state,
      std::shared_ptr<PostingsReaderBase> postingsReader) ;

private:
  void seekDir(std::shared_ptr<IndexInput> in_) ;
  void checkFieldSummary(
      std::shared_ptr<SegmentInfo> info, std::shared_ptr<IndexInput> indexIn,
      std::shared_ptr<IndexInput> blockIn, std::shared_ptr<TermsReader> field,
      std::shared_ptr<TermsReader> previous) ;

public:
  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

  virtual ~FSTOrdTermsReader();

public:
  class TermsReader final : public Terms, public Accountable
  {
    GET_CLASS_NAME(TermsReader)
  private:
    std::shared_ptr<FSTOrdTermsReader> outerInstance;

  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int64_t numTerms;
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;
    const int longsSize;
    const std::shared_ptr<FST<int64_t>> index;

    const int numSkipInfo;
    std::deque<int64_t> const skipInfo;
    std::deque<char> const statsBlock;
    std::deque<char> const metaLongsBlock;
    std::deque<char> const metaBytesBlock;

    TermsReader(std::shared_ptr<FSTOrdTermsReader> outerInstance,
                std::shared_ptr<FieldInfo> fieldInfo,
                std::shared_ptr<IndexInput> blockIn, int64_t numTerms,
                int64_t sumTotalTermFreq, int64_t sumDocFreq, int docCount,
                int longsSize,
                std::shared_ptr<FST<int64_t>> index) ;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    std::shared_ptr<TermsEnum> iterator()  override;

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> compiled,
              std::shared_ptr<BytesRef> startTerm)  override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();

    // Only wraps common operations for PBF interact
  public:
    class BaseTermsEnum : public TermsEnum
    {
      GET_CLASS_NAME(BaseTermsEnum)
    private:
      std::shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance;

      /* Current term's ord, starts from 0 */
    public:
      // C++ NOTE: Fields cannot have the same name as methods:
      int64_t ord_ = 0;

      /* Current term stats + decoded metadata (customized by PBF) */
      const std::shared_ptr<BlockTermState> state;

      /* Datainput to load stats & metadata */
      const std::shared_ptr<ByteArrayDataInput> statsReader =
          std::make_shared<ByteArrayDataInput>();
      const std::shared_ptr<ByteArrayDataInput> metaLongsReader =
          std::make_shared<ByteArrayDataInput>();
      const std::shared_ptr<ByteArrayDataInput> metaBytesReader =
          std::make_shared<ByteArrayDataInput>();

      /* To which block is buffered */
      int statsBlockOrd = 0;
      int metaBlockOrd = 0;

      /* Current buffered metadata (long[] & byte[]) */
      std::deque<std::deque<int64_t>> longs;
      std::deque<int> bytesStart;
      std::deque<int> bytesLength;

      /* Current buffered stats (df & ttf) */
      // C++ NOTE: Fields cannot have the same name as methods:
      std::deque<int> docFreq_;
      // C++ NOTE: Fields cannot have the same name as methods:
      std::deque<int64_t> totalTermFreq_;

      BaseTermsEnum(std::shared_ptr<FSTOrdTermsReader::TermsReader>
                        outerInstance) ;

      /** Decodes stats data into term state */
      virtual void decodeStats() ;

      /** Let PBF decode metadata */
      virtual void decodeMetaData() ;

      /** Load current stats shard */
      void refillStats() ;

      /** Load current metadata shard */
      void refillMetadata() ;

      std::shared_ptr<TermState> termState()  override;

      int docFreq()  override;

      int64_t totalTermFreq()  override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

      // TODO: this can be achieved by making use of Util.getByOutput()
      //           and should have related tests
      void seekExact(int64_t ord)  override;

      int64_t ord() override;

    protected:
      std::shared_ptr<BaseTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<BaseTermsEnum>(
            org.apache.lucene.index.TermsEnum::shared_from_this());
      }
    };

    // Iterates through all terms in this field
  private:
    class SegmentTermsEnum final : public BaseTermsEnum
    {
      GET_CLASS_NAME(SegmentTermsEnum)
    private:
      std::shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance;

    public:
      const std::shared_ptr<BytesRefFSTEnum<int64_t>> fstEnum;
      /* Current term, null when enum ends or unpositioned */
      // C++ NOTE: Fields cannot have the same name as methods:
      std::shared_ptr<BytesRef> term_;

      /* True when current term's metadata is decoded */
      bool decoded = false;

      /* True when current enum is 'positioned' by seekExact(TermState) */
      bool seekPending = false;

      SegmentTermsEnum(std::shared_ptr<FSTOrdTermsReader::TermsReader>
                           outerInstance) ;

      std::shared_ptr<BytesRef> term()  override;

      void decodeMetaData()  override;

      // Update current enum according to FSTEnum
      void updateEnum(std::shared_ptr<BytesRefFSTEnum::InputOutput<int64_t>>
                          pair) ;

      std::shared_ptr<BytesRef> next()  override;

      bool
      seekExact(std::shared_ptr<BytesRef> target)  override;

      SeekStatus
      seekCeil(std::shared_ptr<BytesRef> target)  override;

      void seekExact(std::shared_ptr<BytesRef> target,
                     std::shared_ptr<TermState> otherState) override;

    protected:
      std::shared_ptr<SegmentTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<SegmentTermsEnum>(
            BaseTermsEnum::shared_from_this());
      }
    };

    // Iterates intersect result with automaton (cannot seek!)
  private:
    class IntersectTermsEnum final : public BaseTermsEnum
    {
      GET_CLASS_NAME(IntersectTermsEnum)
    private:
      std::shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance;

      /* Current term, null when enum ends or unpositioned */
    public:
      // C++ NOTE: Fields cannot have the same name as methods:
      std::shared_ptr<BytesRefBuilder> term_;

      /* True when current term's metadata is decoded */
      bool decoded = false;

      /* True when there is pending term when calling next() */
      bool pending = false;

      /* stack to record how current term is constructed,
       * used to accumulate metadata or rewind term:
       *   level == term.length + 1,
       *         == 0 when term is null */
      std::deque<std::shared_ptr<Frame>> stack;
      int level = 0;

      /* term dict fst */
      const std::shared_ptr<FST<int64_t>> fst;
      const std::shared_ptr<FST::BytesReader> fstReader;
      const std::shared_ptr<Outputs<int64_t>> fstOutputs;

      /* query automaton to intersect with */
      const std::shared_ptr<ByteRunAutomaton> fsa;

    private:
      class Frame final : public std::enable_shared_from_this<Frame>
      {
        GET_CLASS_NAME(Frame)
      private:
        std::shared_ptr<FSTOrdTermsReader::TermsReader::IntersectTermsEnum>
            outerInstance;

        /* fst stats */
      public:
        std::shared_ptr<FST::Arc<int64_t>> arc;

        /* automaton stats */
        int state = 0;

        Frame(
            std::shared_ptr<FSTOrdTermsReader::TermsReader::IntersectTermsEnum>
                outerInstance);

        std::wstring toString();
      };

    public:
      IntersectTermsEnum(
          std::shared_ptr<FSTOrdTermsReader::TermsReader> outerInstance,
          std::shared_ptr<CompiledAutomaton> compiled,
          std::shared_ptr<BytesRef> startTerm) ;

      std::shared_ptr<BytesRef> term()  override;

      void decodeMetaData()  override;

      void decodeStats()  override;

      SeekStatus
      seekCeil(std::shared_ptr<BytesRef> target)  override;

      std::shared_ptr<BytesRef> next()  override;

      std::shared_ptr<BytesRef>
      doSeekCeil(std::shared_ptr<BytesRef> target) ;

      /** Virtual frame, never pop */
      std::shared_ptr<Frame>
      loadVirtualFrame(std::shared_ptr<Frame> frame) ;

      /** Load frame for start arc(node) on fst */
      std::shared_ptr<Frame>
      loadFirstFrame(std::shared_ptr<Frame> frame) ;

      /** Load frame for target arc(node) on fst */
      std::shared_ptr<Frame>
      loadExpandFrame(std::shared_ptr<Frame> top,
                      std::shared_ptr<Frame> frame) ;

      /** Load frame for sibling arc(node) on fst */
      std::shared_ptr<Frame>
      loadNextFrame(std::shared_ptr<Frame> top,
                    std::shared_ptr<Frame> frame) ;

      /** Load frame for target arc(node) on fst, so that
       *  arc.label &gt;= label and !fsa.reject(arc.label) */
      std::shared_ptr<Frame>
      loadCeilFrame(int label, std::shared_ptr<Frame> top,
                    std::shared_ptr<Frame> frame) ;

      bool isAccept(std::shared_ptr<Frame> frame);
      bool isValid(std::shared_ptr<Frame> frame);
      bool canGrow(std::shared_ptr<Frame> frame);
      bool canRewind(std::shared_ptr<Frame> frame);

      void pushFrame(std::shared_ptr<Frame> frame);

      std::shared_ptr<Frame> popFrame();

      std::shared_ptr<Frame> newFrame();

      std::shared_ptr<Frame> topFrame();

      std::shared_ptr<BytesRefBuilder> grow(int label);

      std::shared_ptr<BytesRefBuilder> shrink();

    protected:
      std::shared_ptr<IntersectTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<IntersectTermsEnum>(
            BaseTermsEnum::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<TermsReader> shared_from_this()
    {
      return std::static_pointer_cast<TermsReader>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

public:
  template <typename T>
  static void walk(std::shared_ptr<FST<T>> fst) ;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<FSTOrdTermsReader> shared_from_this()
  {
    return std::static_pointer_cast<FSTOrdTermsReader>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/memory/
