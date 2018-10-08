#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class RandomPostingsTester;
}

namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::codecs
{
class Codec;
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
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::index
{

using Document = org::apache::lucene::document::Document;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.ALL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.FREQS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.NONE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.OFFSETS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.PAYLOADS;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.index.PostingsEnum.POSITIONS;

/**
 * Abstract class to do basic tests for a postings format.
 * NOTE: This test focuses on the postings
 * (docs/freqs/positions/payloads/offsets) impl, not the
 * terms dict.  The [stretch] goal is for this test to be
 * so thorough in testing a new PostingsFormat that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given PostingsFormat that this
 * test fails to catch then this test needs to be improved! */

// TODO can we make it easy for testing to pair up a "random terms dict impl"
// with your postings base format...

// TODO test when you reuse after skipping a term or two, eg the block reuse
// case

/* TODO
  - threads
  - assert doc=-1 before any nextDoc
  - if a PF passes this test but fails other tests then this
    test has a bug!!
  - test tricky reuse cases, eg across fields
  - verify you get null if you pass needFreq/needOffset but
    they weren't indexed
*/

class BasePostingsFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BasePostingsFormatTestCase)

public:
  static std::shared_ptr<RandomPostingsTester> postingsTester;

  // TODO maybe instead of @BeforeClass just make a single test run: build
  // postings & index & test it?

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void createPostings() throws
  // java.io.IOException
  static void createPostings() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
  static void afterClass() ;

  virtual void testDocsOnly() ;

  virtual void testDocsAndFreqs() ;

  virtual void testDocsAndFreqsAndPositions() ;

  virtual void
  testDocsAndFreqsAndPositionsAndPayloads() ;

  virtual void
  testDocsAndFreqsAndPositionsAndOffsets() ;

  virtual void
  testDocsAndFreqsAndPositionsAndOffsetsAndPayloads() ;

  virtual void testRandom() ;

protected:
  virtual bool isPostingsEnumReuseImplemented();

public:
  virtual void testPostingsEnumReuse() ;

protected:
  static void checkReuse(std::shared_ptr<TermsEnum> termsEnum, int firstFlags,
                         int secondFlags, bool shouldReuse) ;

public:
  virtual void testJustEmptyField() ;

  virtual void testEmptyFieldAndEmptyTerm() ;

  virtual void testDidntWantFreqsButAskedAnyway() ;

  virtual void testAskForPositionsWhenNotThere() ;

  // tests that ghost fields still work
  // TODO: can this be improved?
  virtual void testGhosts() ;

  // tests that level 2 ghost fields still work
  virtual void testLevel2Ghosts() ;

private:
  class TermFreqs : public std::enable_shared_from_this<TermFreqs>
  {
    GET_CLASS_NAME(TermFreqs)
  public:
    int64_t totalTermFreq = 0;
    int docFreq = 0;
  };

  // LUCENE-5123: make sure we can visit postings twice
  // during flush/merge
public:
  virtual void testInvertedWrite() ;

private:
  class FilterCodecAnonymousInnerClass : public FilterCodec
  {
    GET_CLASS_NAME(FilterCodecAnonymousInnerClass)
  private:
    std::shared_ptr<BasePostingsFormatTestCase> outerInstance;

    std::unordered_map<std::wstring, std::shared_ptr<TermFreqs>> termFreqs;
    std::shared_ptr<AtomicLong> sumDocFreq;
    std::shared_ptr<AtomicLong> sumTotalTermFreq;

  public:
    FilterCodecAnonymousInnerClass(
        std::shared_ptr<BasePostingsFormatTestCase> outerInstance,
        const std::wstring &getName,
        std::shared_ptr<org::apache::lucene::codecs::Codec> getCodec,
        std::unordered_map<std::wstring, std::shared_ptr<TermFreqs>> &termFreqs,
        std::shared_ptr<AtomicLong> sumDocFreq,
        std::shared_ptr<AtomicLong> sumTotalTermFreq);

    std::shared_ptr<PostingsFormat> postingsFormat() override;

  private:
    class PostingsFormatAnonymousInnerClass : public PostingsFormat
    {
      GET_CLASS_NAME(PostingsFormatAnonymousInnerClass)
    private:
      std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance;

      std::shared_ptr<PostingsFormat> defaultPostingsFormat;
      std::shared_ptr<Thread> mainThread;

    public:
      PostingsFormatAnonymousInnerClass(
          std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance,
          const std::wstring &getName,
          std::shared_ptr<PostingsFormat> defaultPostingsFormat,
          std::shared_ptr<Thread> mainThread);

      std::shared_ptr<FieldsConsumer> fieldsConsumer(
          std::shared_ptr<SegmentWriteState> state)  override;

    private:
      class FieldsConsumerAnonymousInnerClass : public FieldsConsumer
      {
        GET_CLASS_NAME(FieldsConsumerAnonymousInnerClass)
      private:
        std::shared_ptr<PostingsFormatAnonymousInnerClass> outerInstance;

        std::shared_ptr<org::apache::lucene::index::SegmentWriteState> state;
        std::shared_ptr<FieldsConsumer> fieldsConsumer;

      public:
        FieldsConsumerAnonymousInnerClass(
            std::shared_ptr<PostingsFormatAnonymousInnerClass> outerInstance,
            std::shared_ptr<org::apache::lucene::index::SegmentWriteState>
                state,
            std::shared_ptr<FieldsConsumer> fieldsConsumer);

        void write(std::shared_ptr<Fields> fields)  override;

        virtual ~FieldsConsumerAnonymousInnerClass();

      protected:
        std::shared_ptr<FieldsConsumerAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<FieldsConsumerAnonymousInnerClass>(
              org.apache.lucene.codecs.FieldsConsumer::shared_from_this());
        }
      };

    public:
      std::shared_ptr<FieldsProducer> fieldsProducer(
          std::shared_ptr<SegmentReadState> state)  override;

    protected:
      std::shared_ptr<PostingsFormatAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PostingsFormatAnonymousInnerClass>(
            org.apache.lucene.codecs.PostingsFormat::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

protected:
  virtual void assertReused(const std::wstring &field,
                            std::shared_ptr<PostingsEnum> p1,
                            std::shared_ptr<PostingsEnum> p2);

public:
  virtual void testPostingsEnumDocsOnly() ;

  virtual void testPostingsEnumFreqs() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<BasePostingsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<BasePostingsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumPositions() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<BasePostingsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<BasePostingsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumOffsets() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<BasePostingsFormatTestCase> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<BasePostingsFormatTestCase> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testPostingsEnumPayloads() ;

  virtual void testPostingsEnumAll() ;

protected:
  void addRandomFields(std::shared_ptr<Document> doc) override;

protected:
  std::shared_ptr<BasePostingsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BasePostingsFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
