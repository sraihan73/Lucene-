#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/index/TermData.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/Builder.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/Fields.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO: test multiple codecs here?

// TODO
//   - test across fields
//   - fix this test to run once for all codecs
//   - make more docs per term, to test > 1 level skipping
//   - test all combinations of payloads/not and omitTF/not
//   - test w/ different indexDivisor
//   - test field where payload length rarely changes
//   - 0-term fields
//   - seek/skip to same term/doc i'm already on
//   - mix in deleted docs
//   - seek, skip beyond end -- assert returns false
//   - seek, skip to things that don't exist -- ensure it
//     goes to 1 before next one known to exist
//   - skipTo(term)
//   - skipTo(doc)

class TestCodecs : public LuceneTestCase
{
  GET_CLASS_NAME(TestCodecs)
private:
  static std::deque<std::wstring> fieldNames;

  static int NUM_TEST_ITER;
  static constexpr int NUM_TEST_THREADS = 3;
  static constexpr int NUM_FIELDS = 4;
  static constexpr int NUM_TERMS_RAND = 50; // must be > 16 to test skipping
  static constexpr int DOC_FREQ_RAND = 500; // must be > 16 to test skipping
  static constexpr int TERM_DOC_FREQ_RAND = 20;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void beforeClass()
  static void beforeClass();

public:
  class FieldData : public std::enable_shared_from_this<FieldData>,
                    public Comparable<std::shared_ptr<FieldData>>
  {
    GET_CLASS_NAME(FieldData)
  public:
    const std::shared_ptr<FieldInfo> fieldInfo;
    std::deque<std::shared_ptr<TermData>> const terms;
    const bool omitTF;
    const bool storePayloads;

    FieldData(const std::wstring &name,
              std::shared_ptr<FieldInfos::Builder> fieldInfos,
              std::deque<std::shared_ptr<TermData>> &terms, bool const omitTF,
              bool const storePayloads);

    int compareTo(std::shared_ptr<FieldData> other) override;
  };

public:
  class PositionData : public std::enable_shared_from_this<PositionData>
  {
    GET_CLASS_NAME(PositionData)
  public:
    int pos = 0;
    std::shared_ptr<BytesRef> payload;

    PositionData(int const pos, std::shared_ptr<BytesRef> payload);
  };

public:
  class TermData : public std::enable_shared_from_this<TermData>,
                   public Comparable<std::shared_ptr<TermData>>
  {
    GET_CLASS_NAME(TermData)
  public:
    std::wstring text2;
    const std::shared_ptr<BytesRef> text;
    std::deque<int> docs;
    std::deque<std::deque<std::shared_ptr<PositionData>>> positions;
    std::shared_ptr<FieldData> field;

    TermData(
        const std::wstring &text, std::deque<int> &docs,
        std::deque<std::deque<std::shared_ptr<PositionData>>> &positions);

    int compareTo(std::shared_ptr<TermData> o) override;
  };

private:
  static const std::wstring SEGMENT;

public:
  virtual std::deque<std::shared_ptr<TermData>>
  makeRandomTerms(bool const omitTF, bool const storePayloads);

  virtual void testFixedPostings() ;

  virtual void testRandomPostings() ;

private:
  class Verify : public Thread
  {
    GET_CLASS_NAME(Verify)
  public:
    const std::shared_ptr<Fields> termsDict;
    std::deque<std::shared_ptr<FieldData>> const fields;
    const std::shared_ptr<SegmentInfo> si;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool failed;
    bool failed = false;

    Verify(std::shared_ptr<SegmentInfo> si,
           std::deque<std::shared_ptr<FieldData>> &fields,
           std::shared_ptr<Fields> termsDict);

    void run() override;

  private:
    void verifyDocs(
        std::deque<int> &docs,
        std::deque<std::deque<std::shared_ptr<PositionData>>> &positions,
        std::shared_ptr<PostingsEnum> postingsEnum,
        bool const doPos) ;

  public:
    std::deque<char> data = std::deque<char>(10);

  private:
    void verifyPositions(
        std::deque<std::shared_ptr<PositionData>> &positions,
        std::shared_ptr<PostingsEnum> posEnum) ;

  public:
    virtual void _run() ;

  protected:
    std::shared_ptr<Verify> shared_from_this()
    {
      return std::static_pointer_cast<Verify>(Thread::shared_from_this());
    }
  };

private:
  class DataFields : public Fields
  {
    GET_CLASS_NAME(DataFields)
  private:
    std::deque<std::shared_ptr<FieldData>> const fields;

  public:
    DataFields(std::deque<std::shared_ptr<FieldData>> &fields);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

  private:
    class IteratorAnonymousInnerClass
        : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
          public Iterator<std::wstring>
    {
      GET_CLASS_NAME(IteratorAnonymousInnerClass)
    private:
      std::shared_ptr<DataFields> outerInstance;

    public:
      IteratorAnonymousInnerClass(std::shared_ptr<DataFields> outerInstance);

      int upto = 0;

      bool hasNext();

      std::wstring next();

      void remove();
    };

  public:
    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

  protected:
    std::shared_ptr<DataFields> shared_from_this()
    {
      return std::static_pointer_cast<DataFields>(Fields::shared_from_this());
    }
  };

private:
  class DataTerms : public Terms
  {
    GET_CLASS_NAME(DataTerms)
  public:
    const std::shared_ptr<FieldData> fieldData;

    DataTerms(std::shared_ptr<FieldData> fieldData);

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
    std::shared_ptr<DataTerms> shared_from_this()
    {
      return std::static_pointer_cast<DataTerms>(Terms::shared_from_this());
    }
  };

private:
  class DataTermsEnum : public TermsEnum
  {
    GET_CLASS_NAME(DataTermsEnum)
  public:
    const std::shared_ptr<FieldData> fieldData;

  private:
    int upto = -1;

  public:
    DataTermsEnum(std::shared_ptr<FieldData> fieldData);

    std::shared_ptr<BytesRef> next() override;

    std::shared_ptr<BytesRef> term() override;

    SeekStatus seekCeil(std::shared_ptr<BytesRef> text) override;

    void seekExact(int64_t ord) override;

    int64_t ord() override;

    int docFreq() override;

    int64_t totalTermFreq() override;

    std::shared_ptr<PostingsEnum> postings(std::shared_ptr<PostingsEnum> reuse,
                                           int flags) override;

  protected:
    std::shared_ptr<DataTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<DataTermsEnum>(
          TermsEnum::shared_from_this());
    }
  };

private:
  class DataPostingsEnum : public PostingsEnum
  {
    GET_CLASS_NAME(DataPostingsEnum)
  public:
    const std::shared_ptr<TermData> termData;
    int docUpto = -1;
    int posUpto = 0;

    DataPostingsEnum(std::shared_ptr<TermData> termData);

    int64_t cost() override;

    int nextDoc() override;

    int docID() override;

    int advance(int target) override;

    int freq() override;

    int nextPosition() override;

    std::shared_ptr<BytesRef> getPayload() override;

    int startOffset() override;

    int endOffset() override;

  protected:
    std::shared_ptr<DataPostingsEnum> shared_from_this()
    {
      return std::static_pointer_cast<DataPostingsEnum>(
          PostingsEnum::shared_from_this());
    }
  };

private:
  void write(std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<FieldInfos> fieldInfos,
             std::shared_ptr<Directory> dir,
             std::deque<std::shared_ptr<FieldData>>
                 &fields) ;

public:
  virtual void testDocsOnlyFreq() ;

protected:
  std::shared_ptr<TestCodecs> shared_from_this()
  {
    return std::static_pointer_cast<TestCodecs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
