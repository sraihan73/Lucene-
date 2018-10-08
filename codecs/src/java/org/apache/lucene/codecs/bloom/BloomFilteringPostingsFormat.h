#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsConsumer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "DefaultBloomFilterFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::bloom
{
class BloomFilterFactory;
}

namespace org::apache::lucene::codecs
{
class PostingsFormat;
}
namespace org::apache::lucene::codecs
{
class FieldsConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::codecs::bloom
{
class FuzzySet;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::store
{
class IndexOutput;
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
namespace org::apache::lucene::codecs::bloom
{

using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * <p>
 * A {@link PostingsFormat} useful for low doc-frequency fields such as primary
 * keys. Bloom filters are maintained in a ".blm" file which offers "fast-fail"
 * for reads in segments known to have no record of the key. A choice of
 * delegate PostingsFormat is used to record all other Postings data.
 * </p>
 * <p>
 * A choice of {@link BloomFilterFactory} can be passed to tailor Bloom Filter
 * settings on a per-field basis. The default configuration is
 * {@link DefaultBloomFilterFactory} which allocates a ~8mb bitset and hashes
 * values using {@link MurmurHash2}. This should be suitable for most purposes.
 * </p>
 * <p>
 * The format of the blm file is as follows:
 * </p>
 * <ul>
 * <li>BloomFilter (.blm) --&gt; Header, DelegatePostingsFormatName,
 * NumFilteredFields, Filter<sup>NumFilteredFields</sup>, Footer</li>
 * <li>Filter --&gt; FieldNumber, FuzzySet</li>
 * <li>FuzzySet --&gt;See {@link FuzzySet#serialize(DataOutput)}</li>
 * <li>Header --&gt; {@link CodecUtil#writeIndexHeader IndexHeader}</li>
 * <li>DelegatePostingsFormatName --&gt; {@link DataOutput#writeString(std::wstring)
 * std::wstring} The name of a ServiceProvider registered {@link PostingsFormat}</li>
 * <li>NumFilteredFields --&gt; {@link DataOutput#writeInt Uint32}</li>
 * <li>FieldNumber --&gt; {@link DataOutput#writeInt Uint32} The number of the
 * field in this segment</li>
 * <li>Footer --&gt; {@link CodecUtil#writeFooter CodecFooter}</li>
 * </ul>
 * @lucene.experimental
 */
class BloomFilteringPostingsFormat final : public PostingsFormat
{
  GET_CLASS_NAME(BloomFilteringPostingsFormat)

public:
  static const std::wstring BLOOM_CODEC_NAME;
  static constexpr int VERSION_START = 3;
  static constexpr int VERSION_CURRENT = VERSION_START;

  /** Extension of Bloom Filters file */
  static const std::wstring BLOOM_EXTENSION;

  std::shared_ptr<BloomFilterFactory> bloomFilterFactory =
      std::make_shared<DefaultBloomFilterFactory>();

private:
  std::shared_ptr<PostingsFormat> delegatePostingsFormat;

  /**
   * Creates Bloom filters for a selection of fields created in the index. This
   * is recorded as a set of Bitsets held as a segment summary in an additional
   * "blm" file. This PostingsFormat delegates to a choice of delegate
   * PostingsFormat for encoding all other postings data.
   *
   * @param delegatePostingsFormat
   *          The PostingsFormat that records all the non-bloom filter data i.e.
   *          postings info.
   * @param bloomFilterFactory
   *          The {@link BloomFilterFactory} responsible for sizing BloomFilters
   *          appropriately
   */
public:
  BloomFilteringPostingsFormat(
      std::shared_ptr<PostingsFormat> delegatePostingsFormat,
      std::shared_ptr<BloomFilterFactory> bloomFilterFactory);

  /**
   * Creates Bloom filters for a selection of fields created in the index. This
   * is recorded as a set of Bitsets held as a segment summary in an additional
   * "blm" file. This PostingsFormat delegates to a choice of delegate
   * PostingsFormat for encoding all other postings data. This choice of
   * constructor defaults to the {@link DefaultBloomFilterFactory} for
   * configuring per-field BloomFilters.
   *
   * @param delegatePostingsFormat
   *          The PostingsFormat that records all the non-bloom filter data i.e.
   *          postings info.
   */
  BloomFilteringPostingsFormat(
      std::shared_ptr<PostingsFormat> delegatePostingsFormat);

  // Used only by core Lucene at read-time via Service Provider instantiation -
  // do not use at Write-time in application code.
  BloomFilteringPostingsFormat();

  std::shared_ptr<FieldsConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<FieldsProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class BloomFilteredFieldsProducer : public FieldsProducer
  {
    GET_CLASS_NAME(BloomFilteredFieldsProducer)
  private:
    std::shared_ptr<FieldsProducer> delegateFieldsProducer;

  public:
    std::unordered_map<std::wstring, std::shared_ptr<FuzzySet>>
        bloomsByFieldName =
            std::unordered_map<std::wstring, std::shared_ptr<FuzzySet>>();

    BloomFilteredFieldsProducer(std::shared_ptr<SegmentReadState> state) throw(
        IOException);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    virtual ~BloomFilteredFieldsProducer();

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

  public:
    class BloomFilteredTerms : public Terms
    {
      GET_CLASS_NAME(BloomFilteredTerms)
    private:
      std::shared_ptr<Terms> delegateTerms;
      std::shared_ptr<FuzzySet> filter;

    public:
      BloomFilteredTerms(std::shared_ptr<Terms> terms,
                         std::shared_ptr<FuzzySet> filter);

      std::shared_ptr<TermsEnum> intersect(
          std::shared_ptr<CompiledAutomaton> compiled,
          std::shared_ptr<BytesRef> startTerm)  override;

      std::shared_ptr<TermsEnum> iterator()  override;

      int64_t size()  override;

      int64_t getSumTotalTermFreq()  override;

      int64_t getSumDocFreq()  override;

      int getDocCount()  override;

      bool hasFreqs() override;

      bool hasOffsets() override;

      bool hasPositions() override;

      bool hasPayloads() override;

      std::shared_ptr<BytesRef> getMin()  override;

      std::shared_ptr<BytesRef> getMax()  override;

    protected:
      std::shared_ptr<BloomFilteredTerms> shared_from_this()
      {
        return std::static_pointer_cast<BloomFilteredTerms>(
            org.apache.lucene.index.Terms::shared_from_this());
      }
    };

  public:
    class BloomFilteredTermsEnum final : public TermsEnum
    {
      GET_CLASS_NAME(BloomFilteredTermsEnum)
    private:
      std::shared_ptr<Terms> delegateTerms;
      std::shared_ptr<TermsEnum> delegateTermsEnum;
      const std::shared_ptr<FuzzySet> filter;

    public:
      BloomFilteredTermsEnum(
          std::shared_ptr<Terms> delegateTerms,
          std::shared_ptr<FuzzySet> filter) ;

      void reset(std::shared_ptr<Terms> delegateTerms) ;

    private:
      std::shared_ptr<TermsEnum> delegate_() ;

    public:
      std::shared_ptr<BytesRef> next()  override;

      bool
      seekExact(std::shared_ptr<BytesRef> text)  override;

      SeekStatus
      seekCeil(std::shared_ptr<BytesRef> text)  override;

      void seekExact(int64_t ord)  override;

      std::shared_ptr<BytesRef> term()  override;

      int64_t ord()  override;

      int docFreq()  override;

      int64_t totalTermFreq()  override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

    protected:
      std::shared_ptr<BloomFilteredTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<BloomFilteredTermsEnum>(
            org.apache.lucene.index.TermsEnum::shared_from_this());
      }
    };

  public:
    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<BloomFilteredFieldsProducer> shared_from_this()
    {
      return std::static_pointer_cast<BloomFilteredFieldsProducer>(
          org.apache.lucene.codecs.FieldsProducer::shared_from_this());
    }
  };

public:
  class BloomFilteredFieldsConsumer : public FieldsConsumer
  {
    GET_CLASS_NAME(BloomFilteredFieldsConsumer)
  private:
    std::shared_ptr<BloomFilteringPostingsFormat> outerInstance;

    std::shared_ptr<FieldsConsumer> delegateFieldsConsumer;
    std::unordered_map<std::shared_ptr<FieldInfo>, std::shared_ptr<FuzzySet>>
        bloomFilters = std::unordered_map<std::shared_ptr<FieldInfo>,
                                          std::shared_ptr<FuzzySet>>();
    std::shared_ptr<SegmentWriteState> state;

  public:
    BloomFilteredFieldsConsumer(
        std::shared_ptr<BloomFilteringPostingsFormat> outerInstance,
        std::shared_ptr<FieldsConsumer> fieldsConsumer,
        std::shared_ptr<SegmentWriteState> state);

    void write(std::shared_ptr<Fields> fields)  override;

  private:
    bool closed = false;

  public:
    virtual ~BloomFilteredFieldsConsumer();

  private:
    void saveAppropriatelySizedBloomFilter(
        std::shared_ptr<IndexOutput> bloomOutput,
        std::shared_ptr<FuzzySet> bloomFilter,
        std::shared_ptr<FieldInfo> fieldInfo) ;

  protected:
    std::shared_ptr<BloomFilteredFieldsConsumer> shared_from_this()
    {
      return std::static_pointer_cast<BloomFilteredFieldsConsumer>(
          org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<BloomFilteringPostingsFormat> shared_from_this()
  {
    return std::static_pointer_cast<BloomFilteringPostingsFormat>(
        org.apache.lucene.codecs.PostingsFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::bloom
