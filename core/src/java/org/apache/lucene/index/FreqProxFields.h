#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FreqProxTermsWriterPerField.h"

#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/FreqProxPostingsArray.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/TermState.h"
#include  "core/src/java/org/apache/lucene/index/ByteSliceReader.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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
namespace org::apache::lucene::index
{

using FreqProxPostingsArray = org::apache::lucene::index::
    FreqProxTermsWriterPerField::FreqProxPostingsArray;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/** Implements limited (iterators only, no stats) {@link
 *  Fields} interface over the in-RAM buffered
 *  fields/terms/postings, to flush postings through the
 *  PostingsFormat. */

class FreqProxFields : public Fields
{
  GET_CLASS_NAME(FreqProxFields)
public:
  const std::unordered_map<std::wstring,
                           std::shared_ptr<FreqProxTermsWriterPerField>>
      fields = std::make_shared<LinkedHashMap<
          std::wstring, std::shared_ptr<FreqProxTermsWriterPerField>>>();

  FreqProxFields(
      std::deque<std::shared_ptr<FreqProxTermsWriterPerField>> &fieldList);

  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

private:
  class FreqProxTerms : public Terms
  {
    GET_CLASS_NAME(FreqProxTerms)
  public:
    const std::shared_ptr<FreqProxTermsWriterPerField> terms;

    FreqProxTerms(std::shared_ptr<FreqProxTermsWriterPerField> terms);

    std::shared_ptr<TermsEnum> iterator() override;

    int64_t size() override;

    int64_t getSumTotalTermFreq() override;

    int64_t getSumDocFreq() override;

    int getDocCount() override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<FreqProxTerms> shared_from_this()
    {
      return std::static_pointer_cast<FreqProxTerms>(Terms::shared_from_this());
    }
  };

private:
  class FreqProxTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(FreqProxTermsEnum)
  public:
    const std::shared_ptr<FreqProxTermsWriterPerField> terms;
    std::deque<int> const sortedTermIDs;
    const std::shared_ptr<FreqProxPostingsArray> postingsArray;
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
    const int numTerms;
    // C++ NOTE: Fields cannot have the same name as methods:
    int ord_ = 0;

    FreqProxTermsEnum(std::shared_ptr<FreqProxTermsWriterPerField> terms);

    virtual void reset();

    SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

    void seekExact(int64_t ord) override;

    std::shared_ptr<BytesRef> next() override;

    std::shared_ptr<BytesRef> term() override;

    int64_t ord() override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                           int flags) override;

    /**
     * Expert: Returns the TermsEnums internal state to position the TermsEnum
     * without re-seeking the term dictionary.
     * <p>
     * NOTE: A seek by {@link TermState} might not capture the
     * {@link AttributeSource}'s state. Callers must maintain the
     * {@link AttributeSource} states separately
     *
     * @see TermState
     * @see #seekExact(BytesRef, TermState)
     */
    std::shared_ptr<TermState> termState()  override;

  private:
    class TermStateAnonymousInnerClass : public TermState
    {
      GET_CLASS_NAME(TermStateAnonymousInnerClass)
    private:
      std::shared_ptr<FreqProxTermsEnum> outerInstance;

    public:
      TermStateAnonymousInnerClass(
          std::shared_ptr<FreqProxTermsEnum> outerInstance);

      void copyFrom(std::shared_ptr<TermState> other) override;

    protected:
      std::shared_ptr<TermStateAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TermStateAnonymousInnerClass>(
            TermState::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FreqProxTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FreqProxTermsEnum>(
          TermsEnum::shared_from_this());
    }
  };

private:
  class FreqProxDocsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(FreqProxDocsEnum)

  public:
    const std::shared_ptr<FreqProxTermsWriterPerField> terms;
    const std::shared_ptr<FreqProxPostingsArray> postingsArray;
    const std::shared_ptr<ByteSliceReader> reader =
        std::make_shared<ByteSliceReader>();
    const bool readTermFreq;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;
    bool ended = false;
    int termID = 0;

    FreqProxDocsEnum(std::shared_ptr<FreqProxTermsWriterPerField> terms,
                     std::shared_ptr<FreqProxPostingsArray> postingsArray);

    virtual void reset(int termID);

    int docID() override;

    int freq() override;

    int nextPosition()  override;

    int startOffset()  override;

    int endOffset()  override;

    std::shared_ptr<BytesRef> getPayload()  override;

    int nextDoc()  override;

    int advance(int target) override;

    int64_t cost() override;

  protected:
    std::shared_ptr<FreqProxDocsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FreqProxDocsEnum>(
          PostingsEnum::shared_from_this());
    }
  };

private:
  class FreqProxPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(FreqProxPostingsEnum)

  public:
    const std::shared_ptr<FreqProxTermsWriterPerField> terms;
    const std::shared_ptr<FreqProxPostingsArray> postingsArray;
    const std::shared_ptr<ByteSliceReader> reader =
        std::make_shared<ByteSliceReader>();
    const std::shared_ptr<ByteSliceReader> posReader =
        std::make_shared<ByteSliceReader>();
    const bool readOffsets;
    // C++ NOTE: Fields cannot have the same name as methods:
    int docID_ = -1;
    // C++ NOTE: Fields cannot have the same name as methods:
    int freq_ = 0;
    int pos = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int startOffset_ = 0;
    // C++ NOTE: Fields cannot have the same name as methods:
    int endOffset_ = 0;
    int posLeft = 0;
    int termID = 0;
    bool ended = false;
    bool hasPayload = false;
    std::shared_ptr<BytesRefBuilder> payload =
        std::make_shared<BytesRefBuilder>();

    FreqProxPostingsEnum(std::shared_ptr<FreqProxTermsWriterPerField> terms,
                         std::shared_ptr<FreqProxPostingsArray> postingsArray);

    virtual void reset(int termID);

    int docID() override;

    int freq() override;

    int nextDoc()  override;

    int advance(int target) override;

    int64_t cost() override;

    int nextPosition()  override;

    int startOffset() override;

    int endOffset() override;

    std::shared_ptr<BytesRef> getPayload() override;

  protected:
    std::shared_ptr<FreqProxPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<FreqProxPostingsEnum>(
          PostingsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FreqProxFields> shared_from_this()
  {
    return std::static_pointer_cast<FreqProxFields>(Fields::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
