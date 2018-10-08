#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/BytesRefFSTEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class PairOutputs;
}
namespace org::apache::lucene::util::fst
{
template <typename Atypename B>
class Pair;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class Terms;
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
namespace org::apache::lucene::codecs::simpletext
{

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using BytesRefFSTEnum = org::apache::lucene::util::fst::BytesRefFSTEnum;
using FST = org::apache::lucene::util::fst::FST;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.DOC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.END;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.END_OFFSET;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.FIELD;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.FREQ;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.PAYLOAD;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.POS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.START_OFFSET;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextFieldsWriter.TERM;

class SimpleTextFieldsReader : public FieldsProducer
{
  GET_CLASS_NAME(SimpleTextFieldsReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(SimpleTextFieldsReader::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(std::map_obj::typeid);

  const std::map_obj<std::wstring, int64_t> fields;
  const std::shared_ptr<IndexInput> in_;
  const std::shared_ptr<FieldInfos> fieldInfos;
  const int maxDoc;

public:
  SimpleTextFieldsReader(std::shared_ptr<SegmentReadState> state) throw(
      IOException);

private:
  std::map_obj<std::wstring, int64_t>
  readFields(std::shared_ptr<IndexInput> in_) ;

private:
  class SimpleTextTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(SimpleTextTermsEnum)
  private:
    std::shared_ptr<SimpleTextFieldsReader> outerInstance;

    const IndexOptions indexOptions;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docFreq_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int64_t totalTermFreq_ = 0;
    int64_t docsStart = 0;
    bool ended = false;
    const std::shared_ptr<BytesRefFSTEnum<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
        fstEnum;

  public:
    SimpleTextTermsEnum(
        std::shared_ptr<SimpleTextFieldsReader> outerInstance,
        std::shared_ptr<FST<PairOutputs::Pair<
            int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
            fst,
        IndexOptions indexOptions);

    bool seekExact(std::shared_ptr<BytesRef> text)  override;

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> term() override;

    int64_t ord()  override;

    void seekExact(int64_t ord) override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

  protected:
    std::shared_ptr<SimpleTextTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextTermsEnum>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

private:
  class SimpleTextDocsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(SimpleTextDocsEnum)
  private:
    std::shared_ptr<SimpleTextFieldsReader> outerInstance;

    const std::shared_ptr<IndexInput> inStart;
    const std::shared_ptr<IndexInput> in_;
    bool omitTF = false;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int tf = 0;
    const std::shared_ptr<BytesRefBuilder> scratch =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<CharsRefBuilder> scratchUTF16 =
        std::make_shared<CharsRefBuilder>();
    // C++ NOTE: Fields cannot have the same name as methods:
    int cost_ = 0;

  public:
    SimpleTextDocsEnum(std::shared_ptr<SimpleTextFieldsReader> outerInstance);

    virtual bool canReuse(std::shared_ptr<IndexInput> in_);

    virtual std::shared_ptr<SimpleTextDocsEnum>
    reset(int64_t fp, bool omitTF, int docFreq) ;

    int docID() override;

    int freq()  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SimpleTextDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  class SimpleTextPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(SimpleTextPostingsEnum)
  private:
    std::shared_ptr<SimpleTextFieldsReader> outerInstance;

    const std::shared_ptr<IndexInput> inStart;
    const std::shared_ptr<IndexInput> in_;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    int tf = 0;
    const std::shared_ptr<BytesRefBuilder> scratch =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<BytesRefBuilder> scratch2 =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<CharsRefBuilder> scratchUTF16 =
        std::make_shared<CharsRefBuilder>();
    const std::shared_ptr<CharsRefBuilder> scratchUTF16_2 =
        std::make_shared<CharsRefBuilder>();
    int pos = 0;
    std::shared_ptr<BytesRef> payload;
    int64_t nextDocStart = 0;
    bool readOffsets = false;
    bool readPositions = false;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int cost_ = 0;

  public:
    SimpleTextPostingsEnum(
        std::shared_ptr<SimpleTextFieldsReader> outerInstance);

    virtual bool canReuse(std::shared_ptr<IndexInput> in_);

    virtual std::shared_ptr<SimpleTextPostingsEnum>
    reset(int64_t fp, IndexOptions indexOptions, int docFreq);

    int docID() override;

    int freq()  override;

    int nextDoc()  override;

    int advance(int target)  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SimpleTextPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

public:
  class TermData : public std::enable_shared_from_this<TermData>
  {
    GET_CLASS_NAME(TermData)
  public:
    int64_t docsStart = 0;
    int docFreq = 0;

    TermData(int64_t docsStart, int docFreq);
  };

private:
  static const int64_t TERMS_BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(SimpleTextTerms::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(BytesRef::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(CharsRef::typeid);

private:
  class SimpleTextTerms : public Terms, public Accountable
  {
    GET_CLASS_NAME(SimpleTextTerms)
  private:
    std::shared_ptr<SimpleTextFieldsReader> outerInstance;

    const int64_t termsStart;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int maxDoc;
    int64_t sumTotalTermFreq = 0;
    int64_t sumDocFreq = 0;
    int docCount = 0;
    std::shared_ptr<FST<
        PairOutputs::Pair<int64_t, PairOutputs::Pair<int64_t, int64_t>>>>
        fst;
    int termCount = 0;
    const std::shared_ptr<BytesRefBuilder> scratch =
        std::make_shared<BytesRefBuilder>();
    const std::shared_ptr<CharsRefBuilder> scratchUTF16 =
        std::make_shared<CharsRefBuilder>();

  public:
    SimpleTextTerms(std::shared_ptr<SimpleTextFieldsReader> outerInstance,
                    const std::wstring &field, int64_t termsStart,
                    int maxDoc) ;

  private:
    void loadTerms() ;

  public:
    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    virtual std::wstring toString();

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<SimpleTextTerms> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTextTerms>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

public:
  std::shared_ptr<Iterator<std::wstring>> iterator() override;

private:
  const std::unordered_map<std::wstring, std::shared_ptr<SimpleTextTerms>>
      termsCache =
          std::unordered_map<std::wstring, std::shared_ptr<SimpleTextTerms>>();

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

  virtual ~SimpleTextFieldsReader();

  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<SimpleTextFieldsReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextFieldsReader>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
