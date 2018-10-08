#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Fields.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
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
namespace org::apache::lucene::store
{
class IndexInput;
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
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTVTerms;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
}
namespace org::apache::lucene::codecs::simpletext
{
class SimpleTVPostings;
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
namespace org::apache::lucene::codecs::simpletext
{

using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Fields = org::apache::lucene::index::Fields;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

using namespace org::apache::lucene::codecs::simpletext;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextTermVectorsWriter.*;

/**
 * Reads plain-text term vectors.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextTermVectorsReader : public TermVectorsReader
{
  GET_CLASS_NAME(SimpleTextTermVectorsReader)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          SimpleTextTermVectorsReader::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(BytesRef::typeid) +
      RamUsageEstimator::shallowSizeOfInstance(CharsRef::typeid);

  std::deque<int64_t> offsets; // docid -> offset in .vec file
  std::shared_ptr<IndexInput> in_;
  std::shared_ptr<BytesRefBuilder> scratch =
      std::make_shared<BytesRefBuilder>();
  std::shared_ptr<CharsRefBuilder> scratchUTF16 =
      std::make_shared<CharsRefBuilder>();

public:
  SimpleTextTermVectorsReader(
      std::shared_ptr<Directory> directory, std::shared_ptr<SegmentInfo> si,
      std::shared_ptr<IOContext> context) ;

  // used by clone
  SimpleTextTermVectorsReader(std::deque<int64_t> &offsets,
                              std::shared_ptr<IndexInput> in_);

  // we don't actually write a .tvx-like index, instead we read the
  // vectors file in entirety up-front and save the offsets
  // so we can seek to the data later.
private:
  void readIndex(int maxDoc) ;

public:
  std::shared_ptr<Fields> get(int doc)  override;

  std::shared_ptr<TermVectorsReader> clone() override;

  virtual ~SimpleTextTermVectorsReader();

private:
  void readLine() ;

  int parseIntAt(int offset);

  std::wstring readString(int offset, std::shared_ptr<BytesRefBuilder> scratch);

private:
  class SimpleTVFields : public Fields
  {
    GET_CLASS_NAME(SimpleTVFields)
  private:
    const std::shared_ptr<
        SortedMap<std::wstring, std::shared_ptr<SimpleTVTerms>>>
        fields;

  public:
    SimpleTVFields(
        std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<SimpleTVTerms>>>
            fields);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    int size() override;

  protected:
    std::shared_ptr<SimpleTVFields> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTVFields>(
          org.apache.lucene.index.Fields::shared_from_this());
    }
  };

private:
  class SimpleTVTerms : public Terms
  {
    GET_CLASS_NAME(SimpleTVTerms)
  public:
    const std::shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SimpleTVPostings>>>
        terms;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasOffsets_;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasPositions_;
    // C++ NOTE: Fields cannot have the same name as methods:
    const bool hasPayloads_;

    SimpleTVTerms(bool hasOffsets, bool hasPositions, bool hasPayloads);

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size()  override;

    int64_t getSumTotalTermFreq()  override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<SimpleTVTerms> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTVTerms>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

private:
  class SimpleTVPostings : public std::enable_shared_from_this<SimpleTVPostings>
  {
    GET_CLASS_NAME(SimpleTVPostings)
  private:
    int freq = 0;
    std::deque<int> positions;
    std::deque<int> startOffsets;
    std::deque<int> endOffsets;
    std::deque<std::shared_ptr<BytesRef>> payloads;
  };

private:
  class SimpleTVTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(SimpleTVTermsEnum)
  public:
    std::shared_ptr<
        SortedMap<std::shared_ptr<BytesRef>, std::shared_ptr<SimpleTVPostings>>>
        terms;
    std::shared_ptr<Iterator<std::unordered_map::Entry<
        std::shared_ptr<BytesRef>,
        std::shared_ptr<SimpleTextTermVectorsReader::SimpleTVPostings>>>>
        iterator;
    std::unordered_map::Entry<
        std::shared_ptr<BytesRef>,
        std::shared_ptr<SimpleTextTermVectorsReader::SimpleTVPostings>>
        current;

    SimpleTVTermsEnum(
        std::shared_ptr<SortedMap<std::shared_ptr<BytesRef>,
                                  std::shared_ptr<SimpleTVPostings>>>
            terms);

    SeekStatus
    seekCeil(std::shared_ptr<BytesRef> text)  override;

    void seekExact(int64_t ord)  override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> term()  override;

    int64_t ord()  override;

    int docFreq()  override;

    int64_t totalTermFreq()  override;

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

  protected:
    std::shared_ptr<SimpleTVTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTVTermsEnum>(
          org.apache.lucene.index.TermsEnum::shared_from_this());
    }
  };

  // note: these two enum classes are exactly like the Default impl...
private:
  class SimpleTVDocsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(SimpleTVDocsEnum)
  private:
    bool didNext = false;
    int doc = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;

  public:
    int freq()  override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int docID() override;

    int nextDoc() override;

    int advance(int target)  override;

    virtual void reset(int freq);

    int64_t cost() override;

  protected:
    std::shared_ptr<SimpleTVDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTVDocsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

private:
  class SimpleTVPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(SimpleTVPostingsEnum)
  private:
    bool didNext = false;
    int doc = -1;
    int nextPos = 0;
    std::deque<int> positions;
    std::deque<std::shared_ptr<BytesRef>> payloads;
    std::deque<int> startOffsets;
    std::deque<int> endOffsets;

  public:
    int freq()  override;

    int docID() override;

    int nextDoc() override;

    int advance(int target)  override;

    virtual void reset(std::deque<int> &positions,
                       std::deque<int> &startOffsets,
                       std::deque<int> &endOffsets,
                       std::deque<std::shared_ptr<BytesRef>> &payloads);

    std::shared_ptr<BytesRef> getPayload() override;

    int nextPosition() override;

    int startOffset() override;

    int endOffset() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<SimpleTVPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SimpleTVPostingsEnum>(
          org.apache.lucene.index.PostingsEnum::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<SimpleTextTermVectorsReader> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextTermVectorsReader>(
        org.apache.lucene.codecs.TermVectorsReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
