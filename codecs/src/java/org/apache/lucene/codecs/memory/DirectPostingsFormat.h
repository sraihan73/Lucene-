#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}

namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs::memory
{
class DirectField;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util::automaton
{
class RunAutomaton;
}
namespace org::apache::lucene::util::automaton
{
class Transition;
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
using Fields = org::apache::lucene::index::Fields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using RunAutomaton = org::apache::lucene::util::automaton::RunAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;

// TODO:
//   - build depth-N prefix hash?
//   - or: longer dense skip lists than just next byte?

/** Wraps {@link Lucene50PostingsFormat} format for on-disk
 *  storage, but then at read time loads and stores all
 *  terms and postings directly in RAM as byte[], int[].
 *
 *  <p><b>WARNING</b>: This is
 *  exceptionally RAM intensive: it makes no effort to
 *  compress the postings data, storing terms as separate
 *  byte[] and postings as separate int[], but as a result it
 *  gives substantial increase in search performance.
 *
 *  <p>This postings format supports {@link TermsEnum#ord}
 *  and {@link TermsEnum#seekExact(long)}.

 *  <p>Because this holds all term bytes as a single
 *  byte[], you cannot have more than 2.1GB worth of term
 *  bytes in a single segment.
 *
 * @lucene.experimental */

class DirectPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(DirectPostingsFormat)

private:
  const int minSkipCount;
  const int lowFreqCutoff;

  static constexpr int DEFAULT_MIN_SKIP_COUNT = 8;
  static constexpr int DEFAULT_LOW_FREQ_CUTOFF = 32;

  // private static final bool DEBUG = true;

  // TODO: allow passing/wrapping arbitrary postings format?

public:
  DirectPostingsFormat();

  /** minSkipCount is how many terms in a row must have the
   *  same prefix before we put a skip pointer down.  Terms
   *  with docFreq &lt;= lowFreqCutoff will use a single int[]
   *  to hold all docs, freqs, position and offsets; terms
   *  with higher docFreq will use separate arrays. */
  DirectPostingsFormat(int minSkipCount, int lowFreqCutoff);

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

private:
  class DirectFields final : public FieldsProducer
  {
    GET_CLASS_NAME(DirectFields)
  private:
    const std::unordered_map<std::wstring, std::shared_ptr<DirectField>>
        fields = std::map_obj<std::wstring, std::shared_ptr<DirectField>>();

  public:
    DirectFields(std::shared_ptr<SegmentReadState> state,
                 std::shared_ptr<Fields> fields, int minSkipCount,
                 int lowFreqCutoff) ;

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

    virtual ~DirectFields();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<DirectFields> shared_from_this()
    {
      return std::static_pointer_cast<DirectFields>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

private:
  class DirectField final : public Terms, public Accountable
  {
    GET_CLASS_NAME(DirectField)

  private:
    static const int64_t BASE_RAM_BYTES_USED =
        RamUsageEstimator::shallowSizeOfInstance(DirectField::typeid);

  private:
    class TermAndSkip : public std::enable_shared_from_this<TermAndSkip>,
                        public Accountable
    {
      GET_CLASS_NAME(TermAndSkip)
    public:
      std::deque<int> skips;
    };

  private:
    class LowFreqTerm final : public TermAndSkip
    {
      GET_CLASS_NAME(LowFreqTerm)

    private:
      static const int64_t BASE_RAM_BYTES_USED =
          RamUsageEstimator::shallowSizeOfInstance(HighFreqTerm::typeid);

    public:
      std::deque<int> const postings;
      std::deque<char> const payloads;
      const int docFreq;
      const int totalTermFreq;

      LowFreqTerm(std::deque<int> &postings, std::deque<char> &payloads,
                  int docFreq, int totalTermFreq);

      int64_t ramBytesUsed() override;

    protected:
      std::shared_ptr<LowFreqTerm> shared_from_this()
      {
        return std::static_pointer_cast<LowFreqTerm>(
            TermAndSkip::shared_from_this());
      }
    };

    // TODO: maybe specialize into prx/no-prx/no-frq cases?
  private:
    class HighFreqTerm final : public TermAndSkip
    {
      GET_CLASS_NAME(HighFreqTerm)

    private:
      static const int64_t BASE_RAM_BYTES_USED =
          RamUsageEstimator::shallowSizeOfInstance(HighFreqTerm::typeid);

    public:
      const int64_t totalTermFreq;
      std::deque<int> const docIDs;
      std::deque<int> const freqs;
      std::deque<std::deque<int>> const positions;
      std::deque<std::deque<std::deque<char>>> const payloads;

      HighFreqTerm(std::deque<int> &docIDs, std::deque<int> &freqs,
                   std::deque<std::deque<int>> &positions,
                   std::deque<std::deque<std::deque<char>>> &payloads,
                   int64_t totalTermFreq);

      int64_t ramBytesUsed() override;

    protected:
      std::shared_ptr<HighFreqTerm> shared_from_this()
      {
        return std::static_pointer_cast<HighFreqTerm>(
            TermAndSkip::shared_from_this());
      }
    };

  private:
    std::deque<char> const termBytes;
    std::deque<int> const termOffsets;

    std::deque<int> const skips;
    std::deque<int> const skipOffsets;

    std::deque<std::shared_ptr<TermAndSkip>> const terms;
    const bool hasFreq;
    const bool hasPos;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasOffsets_;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasPayloads_;
    const int64_t sumTotalTermFreq;
    const int docCount;
    const int64_t sumDocFreq;
    int skipCount = 0;

    // TODO: maybe make a separate builder?  These are only
    // used during load:
    int count = 0;
    std::deque<int> sameCounts = std::deque<int>(10);
    const int minSkipCount;

  private:
    class IntArrayWriter final
        : public std::enable_shared_from_this<IntArrayWriter>
    {
      GET_CLASS_NAME(IntArrayWriter)
    private:
      std::deque<int> ints = std::deque<int>(10);
      int upto = 0;

    public:
      void add(int value);

      std::deque<int> get();
    };

  public:
    DirectField(std::shared_ptr<SegmentReadState> state,
                const std::wstring &field, std::shared_ptr<Terms> termsIn,
                int minSkipCount, int lowFreqCutoff) ;

    int64_t ramBytesUsed() override;

    virtual std::wstring toString();

    // Compares in unicode (UTF8) order:
    int compare(int ord, std::shared_ptr<BytesRef> other);

  private:
    void setSkips(int termOrd, std::deque<char> &termBytes);

    void finishSkips();

    void saveSkip(int ord, int backCount);

  public:
    std::shared_ptr<TermsEnum> iterator() override;

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> compiled,
              std::shared_ptr<BytesRef> startTerm) override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq() override;

    int getDocCount() override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  private:
    class DirectTermsEnum final : public TermsEnum
    {
      GET_CLASS_NAME(DirectTermsEnum)
    private:
      std::shared_ptr<DirectPostingsFormat::DirectField> outerInstance;

      const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
      int termOrd = 0;

      DirectTermsEnum(
          std::shared_ptr<DirectPostingsFormat::DirectField> outerInstance);

      std::shared_ptr<BytesRef> setTerm();

    public:
      std::shared_ptr<BytesRef> next() override;

      std::shared_ptr<TermState> termState() override;

      // If non-negative, exact match; else, -ord-1, where ord
      // is where you would insert the term.
    private:
      int findTerm(std::shared_ptr<BytesRef> term);

    public:
      SeekStatus seekCeil(std::shared_ptr<BytesRef> term) override;

      bool seekExact(std::shared_ptr<BytesRef> term) override;

      void seekExact(int64_t ord) override;

      void
      seekExact(std::shared_ptr<BytesRef> term,
                std::shared_ptr<TermState> state)  override;

      std::shared_ptr<BytesRef> term() override;

      int64_t ord() override;

      int docFreq() override;

      int64_t totalTermFreq() override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

    protected:
      std::shared_ptr<DirectTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<DirectTermsEnum>(
            org.apache.lucene.index.TermsEnum::shared_from_this());
      }
    };

  private:
    class DirectIntersectTermsEnum final : public TermsEnum
    {
      GET_CLASS_NAME(DirectIntersectTermsEnum)
    private:
      std::shared_ptr<DirectPostingsFormat::DirectField> outerInstance;

      const std::shared_ptr<RunAutomaton> runAutomaton;
      const std::shared_ptr<CompiledAutomaton> compiledAutomaton;
      int termOrd = 0;
      const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();

    private:
      class State final : public std::enable_shared_from_this<State>
      {
        GET_CLASS_NAME(State)
      private:
        std::shared_ptr<
            DirectPostingsFormat::DirectField::DirectIntersectTermsEnum>
            outerInstance;

      public:
        State(std::shared_ptr<
              DirectPostingsFormat::DirectField::DirectIntersectTermsEnum>
                  outerInstance);

        int changeOrd = 0;
        int state = 0;
        int transitionUpto = 0;
        int transitionCount = 0;
        int transitionMax = 0;
        int transitionMin = 0;
        const std::shared_ptr<Transition> transition =
            std::make_shared<Transition>();
      };

    private:
      std::deque<std::shared_ptr<State>> states;
      int stateUpto = 0;

    public:
      DirectIntersectTermsEnum(
          std::shared_ptr<DirectPostingsFormat::DirectField> outerInstance,
          std::shared_ptr<CompiledAutomaton> compiled,
          std::shared_ptr<BytesRef> startTerm);

    private:
      void grow();

    public:
      std::shared_ptr<BytesRef> next() override;

      std::shared_ptr<TermState> termState() override;

      std::shared_ptr<BytesRef> term() override;

      int64_t ord() override;

      int docFreq() override;

      int64_t totalTermFreq() override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse, int flags) override;

      SeekStatus seekCeil(std::shared_ptr<BytesRef> term) override;

      void seekExact(int64_t ord) override;

    protected:
      std::shared_ptr<DirectIntersectTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<DirectIntersectTermsEnum>(
            org.apache.lucene.index.TermsEnum::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DirectField> shared_from_this()
    {
      return std::static_pointer_cast<DirectField>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

  // Docs only:
private:
  class LowFreqDocsEnumNoTF final : public PostingsEnum
  {
    GET_CLASS_NAME(LowFreqDocsEnumNoTF)
  private:
    std::deque<int> postings;
    int upto = 0;

  public:
    std::shared_ptr<PostingsEnum> reset(std::deque<int> &postings);

    // TODO: can do this w/o setting members?

    int nextDoc() override;

    int docID() override;

    int freq() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<LowFreqDocsEnumNoTF> shared_from_this()
    {
      return std::static_pointer_cast<LowFreqDocsEnumNoTF>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // Docs + freqs:
private:
  class LowFreqDocsEnumNoPos final : public PostingsEnum
  {
    GET_CLASS_NAME(LowFreqDocsEnumNoPos)
  private:
    std::deque<int> postings;
    int upto = 0;

  public:
    LowFreqDocsEnumNoPos();

    std::shared_ptr<PostingsEnum> reset(std::deque<int> &postings);

    // TODO: can do this w/o setting members?
    int nextDoc() override;

    int docID() override;

    int freq() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<LowFreqDocsEnumNoPos> shared_from_this()
    {
      return std::static_pointer_cast<LowFreqDocsEnumNoPos>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // Docs + freqs + positions/offets:
private:
  class LowFreqDocsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(LowFreqDocsEnum)
  private:
    std::deque<int> postings;
    const int posMult;
    int upto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;

  public:
    LowFreqDocsEnum(int posMult);

    bool canReuse(int posMult);

    std::shared_ptr<PostingsEnum> reset(std::deque<int> &postings);

    // TODO: can do this w/o setting members?
    int nextDoc() override;

    int docID() override;

    int freq() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<LowFreqDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<LowFreqDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  class LowFreqPostingsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(LowFreqPostingsEnum)
  private:
    std::deque<int> postings;
    const int posMult;
    const bool hasOffsets;
    const bool hasPayloads;
    const std::shared_ptr<BytesRef> payload = std::make_shared<BytesRef>();
    int upto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;
    int skipPositions = 0;
    int pos = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = 0;
    int lastPayloadOffset = 0;
    int payloadOffset = 0;
    int payloadLength = 0;
    std::deque<char> payloadBytes;

  public:
    LowFreqPostingsEnum(bool hasOffsets, bool hasPayloads);

    std::shared_ptr<PostingsEnum> reset(std::deque<int> &postings,
                                        std::deque<char> &payloadBytes);

    int nextDoc() override;

    int docID() override;

    int freq() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    int advance(int target)  override;

    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<LowFreqPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<LowFreqPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // Docs + freqs:
private:
  class HighFreqDocsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(HighFreqDocsEnum)
  private:
    std::deque<int> docIDs;
    std::deque<int> freqs;
    int upto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;

  public:
    HighFreqDocsEnum();

    std::deque<int> getDocIDs();

    std::deque<int> getFreqs();

    std::shared_ptr<PostingsEnum> reset(std::deque<int> &docIDs,
                                        std::deque<int> &freqs);

    int nextDoc() override;

    int docID() override;

    int freq() override;

    int advance(int target) override;

    int64_t cost() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

  protected:
    std::shared_ptr<HighFreqDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<HighFreqDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

  // TODO: specialize offsets and not
private:
  class HighFreqPostingsEnum final : public PostingsEnum
  {
    GET_CLASS_NAME(HighFreqPostingsEnum)
  private:
    std::deque<int> docIDs;
    std::deque<int> freqs;
    std::deque<std::deque<int>> positions;
    std::deque<std::deque<std::deque<char>>> payloads;
    const bool hasOffsets;
    const int posJump;
    int upto = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int posUpto = 0;
    std::deque<int> curPositions;

  public:
    HighFreqPostingsEnum(bool hasOffsets);

    std::deque<int> getDocIDs();

    std::deque<std::deque<int>> getPositions();

    int getPosJump();

    std::shared_ptr<PostingsEnum>
    reset(std::deque<int> &docIDs, std::deque<int> &freqs,
          std::deque<std::deque<int>> &positions,
          std::deque<std::deque<std::deque<char>>> &payloads);

    int nextDoc() override;

    int freq() override;

    int docID() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    int advance(int target) override;

  private:
    const std::shared_ptr<BytesRef> payload = std::make_shared<BytesRef>();

  public:
    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<HighFreqPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<HighFreqPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DirectPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<DirectPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::memory
