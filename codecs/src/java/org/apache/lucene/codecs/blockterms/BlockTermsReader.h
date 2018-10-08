#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <map_obj>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::codecs
{
class PostingsReaderBase;
}
namespace org::apache::lucene::codecs::blockterms
{
class FieldReader;
}
namespace org::apache::lucene::codecs::blockterms
{
class TermsIndexReaderBase;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::codecs
{
class BlockTermState;
}
namespace org::apache::lucene::codecs::blockterms
{
class FieldIndexEnum;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::util
{
class Accountable;
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
namespace org::apache::lucene::codecs::blockterms
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/** Handles a terms dict, but decouples all details of
 *  doc/freqs/positions reading to an instance of {@link
 *  PostingsReaderBase}.  This class is reusable for
 *  codecs that use a different format for
 *  docs/freqs/positions (though codecs are also free to
 *  make their own terms dict impl).
 *
 * <p>This class also interacts with an instance of {@link
 * TermsIndexReaderBase}, to abstract away the specific
 * implementation of the terms dict index.
 * @lucene.experimental */

class BlockTermsReader : public FieldsProducer
{
  GET_CLASS_NAME(BlockTermsReader)
private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(BlockTermsReader::typeid);
  // Open input to the main terms dict file (_X.tis)
  const std::shared_ptr<IndexInput> in_;

  // Reads the terms dict entries, to gather state to
  // produce DocsEnum on demand
  const std::shared_ptr<PostingsReaderBase> postingsReader;

  const std::map_obj<std::wstring, std::shared_ptr<FieldReader>> fields =
      std::map_obj<std::wstring, std::shared_ptr<FieldReader>>();

  // Reads the terms index
  std::shared_ptr<TermsIndexReaderBase> indexReader;

  // Used as key for the terms cache
private:
  class FieldAndTerm : public std::enable_shared_from_this<FieldAndTerm>,
                       public Cloneable
  {
    GET_CLASS_NAME(FieldAndTerm)
  public:
    std::wstring field;
    std::shared_ptr<BytesRef> term;

    FieldAndTerm();

    FieldAndTerm(std::shared_ptr<FieldAndTerm> other);

    bool equals(std::any _other) override;

    std::shared_ptr<FieldAndTerm> clone() override;

    virtual int hashCode();
  };

public:
  BlockTermsReader(std::shared_ptr<TermsIndexReaderBase> indexReader,
                   std::shared_ptr<PostingsReaderBase> postingsReader,
                   std::shared_ptr<SegmentReadState> state) ;

private:
  void seekDir(std::shared_ptr<IndexInput> input) ;

public:
  virtual ~BlockTermsReader();

  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

private:
  static const int64_t FIELD_READER_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(FieldReader::typeid);

private:
  class FieldReader : public Terms, public Accountable
  {
    GET_CLASS_NAME(FieldReader)
  private:
    std::shared_ptr<BlockTermsReader> outerInstance;

  public:
    const int64_t numTerms;
    const std::shared_ptr<FieldInfo> fieldInfo;
    const int64_t termsStartPointer;
    const int64_t sumTotalTermFreq;
    const int64_t sumDocFreq;
    const int docCount;
    const int longsSize;

    FieldReader(std::shared_ptr<BlockTermsReader> outerInstance,
                std::shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
                int64_t termsStartPointer, int64_t sumTotalTermFreq,
                int64_t sumDocFreq, int docCount, int longsSize);

    int64_t ramBytesUsed() override;

    std::shared_ptr<TermsEnum> iterator()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    // Iterates through terms in this field
  private:
    class SegmentTermsEnum final : public TermsEnum
    {
      GET_CLASS_NAME(SegmentTermsEnum)
    private:
      std::shared_ptr<BlockTermsReader::FieldReader> outerInstance;

      const std::shared_ptr<IndexInput> in_;
      const std::shared_ptr<BlockTermState> state;
      const bool doOrd;
      const std::shared_ptr<FieldAndTerm> fieldTerm =
          std::make_shared<FieldAndTerm>();
      const std::shared_ptr<TermsIndexReaderBase::FieldIndexEnum> indexEnum;
      // C++ NOTE: Fields cannot have the same name as methods:
      const std::shared_ptr<BytesRefBuilder> term_ =
          std::make_shared<BytesRefBuilder>();

      /* This is true if indexEnum is "still" seek'd to the index term
         for the current term. We set it to true on seeking, and then it
         remains valid until next() is called enough times to load another
         terms block: */
      bool indexIsCurrent = false;

      /* True if we've already called .next() on the indexEnum, to "bracket"
         the current block of terms: */
      bool didIndexNext = false;

      /* Next index term, bracketing the current block of terms; this is
         only valid if didIndexNext is true: */
      std::shared_ptr<BytesRef> nextIndexTerm;

      /* True after seekExact(TermState), do defer seeking.  If the app then
         calls next() (which is not "typical"), then we'll do the real seek */
      bool seekPending = false;

      std::deque<char> termSuffixes;
      std::shared_ptr<ByteArrayDataInput> termSuffixesReader =
          std::make_shared<ByteArrayDataInput>();

      /* Common prefix used for all terms in this block. */
      int termBlockPrefix = 0;

      /* How many terms in current block */
      int blockTermCount = 0;

      std::deque<char> docFreqBytes;
      const std::shared_ptr<ByteArrayDataInput> freqReader =
          std::make_shared<ByteArrayDataInput>();
      int metaDataUpto = 0;

      std::deque<int64_t> longs;
      std::deque<char> bytes;
      std::shared_ptr<ByteArrayDataInput> bytesReader;

    public:
      SegmentTermsEnum(std::shared_ptr<BlockTermsReader::FieldReader>
                           outerInstance) ;

      // TODO: we may want an alternate mode here which is
      // "if you are about to return NOT_FOUND I won't use
      // the terms data from that"; eg FuzzyTermsEnum will
      // (usually) just immediately call seek again if we
      // return NOT_FOUND so it's a waste for us to fill in
      // the term that was actually NOT_FOUND
      SeekStatus
      seekCeil(std::shared_ptr<BytesRef> target)  override;

      std::shared_ptr<BytesRef> next()  override;

      /* Decodes only the term bytes of the next term.  If caller then asks for
         metadata, ie docFreq, totalTermFreq or pulls a D/&PEnum, we then
         (lazily) decode all metadata up to the current term. */
    private:
      std::shared_ptr<BytesRef> _next() ;

    public:
      std::shared_ptr<BytesRef> term() override;

      int docFreq()  override;

      int64_t totalTermFreq()  override;

      std::shared_ptr<PostingsEnum>
      postings(std::shared_ptr<PostingsEnum> reuse,
               int flags)  override;

      void seekExact(std::shared_ptr<BytesRef> target,
                     std::shared_ptr<TermState> otherState) override;

      std::shared_ptr<TermState> termState()  override;

      void seekExact(int64_t ord)  override;

      int64_t ord() override;

      /* Does initial decode of next block of terms; this
         doesn't actually decode the docFreq, totalTermFreq,
         postings details (frq/prx offset, etc.) metadata;
         it just loads them as byte[] blobs which are then
         decoded on-demand if the metadata is ever requested
         for any term in this block.  This enables terms-only
         intensive consumes (eg certain MTQs, respelling) to
         not pay the price of decoding metadata they won't
         use. */
    private:
      bool nextBlock() ;

      void decodeMetaData() ;

    protected:
      std::shared_ptr<SegmentTermsEnum> shared_from_this()
      {
        return std::static_pointer_cast<SegmentTermsEnum>(
            org.apache.lucene.index.TermsEnum::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FieldReader> shared_from_this()
    {
      return std::static_pointer_cast<FieldReader>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

  void checkIntegrity()  override;

protected:
  std::shared_ptr<BlockTermsReader> shared_from_this()
  {
    return std::static_pointer_cast<BlockTermsReader>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::blockterms
