#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::search::suggest::document
{
class Entry;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search::suggest::document
{
class SuggestIndexSearcher;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::search::suggest::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.BaseTokenStreamTestCase.assertTokenStreamContents;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.search.suggest.document.TopSuggestDocs.SuggestScoreDoc;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.hamcrest.core.IsEqual.equalTo;

class TestSuggestField : public LuceneTestCase
{
  GET_CLASS_NAME(TestSuggestField)

public:
  std::shared_ptr<Directory> dir;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void before() throws Exception
  virtual void before() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void after() throws Exception
  virtual void after() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmptySuggestion() throws Exception
  virtual void testEmptySuggestion() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNegativeWeight() throws Exception
  virtual void testNegativeWeight() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReservedChars() throws Exception
  virtual void testReservedChars() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEmpty() throws Exception
  virtual void testEmpty() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTokenStream() throws Exception
  virtual void testTokenStream() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testDupSuggestFieldValues() throws
  // Exception
  virtual void testDupSuggestFieldValues() ;

  virtual void testDeduplication() ;

  virtual void testExtremeDeduplication() ;

private:
  static std::wstring randomSimpleString(int numDigits, int maxLen);

public:
  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<Entry>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestSuggestField> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestSuggestField> outerInstance);

    int compare(std::shared_ptr<Entry> a, std::shared_ptr<Entry> b);
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNRTDeletedDocFiltering() throws
  // Exception
  virtual void testNRTDeletedDocFiltering() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSuggestOnAllFilteredDocuments() throws
  // Exception
  virtual void testSuggestOnAllFilteredDocuments() ;

private:
  class BitsProducerAnonymousInnerClass : public BitsProducer
  {
    GET_CLASS_NAME(BitsProducerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSuggestField> outerInstance;

  public:
    BitsProducerAnonymousInnerClass(
        std::shared_ptr<TestSuggestField> outerInstance);

    std::shared_ptr<Bits> getBits(
        std::shared_ptr<LeafReaderContext> context)  override;

  protected:
    std::shared_ptr<BitsProducerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<BitsProducerAnonymousInnerClass>(
          org.apache.lucene.search.suggest.BitsProducer::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSuggestOnAllDeletedDocuments() throws
  // Exception
  virtual void testSuggestOnAllDeletedDocuments() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSuggestOnMostlyDeletedDocuments()
  // throws Exception
  virtual void testSuggestOnMostlyDeletedDocuments() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultipleSuggestFieldsPerDoc() throws
  // Exception
  virtual void testMultipleSuggestFieldsPerDoc() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testEarlyTermination() throws Exception
  virtual void testEarlyTermination() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMultipleSegments() throws Exception
  virtual void testMultipleSegments() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testReturnedDocID() throws Exception
  virtual void testReturnedDocID() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testScoring() throws Exception
  virtual void testScoring() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRealisticKeys() throws Exception
  virtual void testRealisticKeys() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testThreads() throws Exception
  virtual void testThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSuggestField> outerInstance;

    std::shared_ptr<Analyzer> analyzer;
    int num = 0;
    std::wstring prefix1;
    std::wstring prefix2;
    std::wstring prefix3;
    std::deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                                    TestSuggestField::Entry>>
        entries1;
    std::deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                                    TestSuggestField::Entry>>
        entries2;
    std::deque<std::shared_ptr<org::apache::lucene::search::suggest::document::
                                    TestSuggestField::Entry>>
        entries3;
    std::shared_ptr<CyclicBarrier> startingGun;
    std::shared_ptr<CopyOnWriteArrayList<std::runtime_error>> errors;
    std::shared_ptr<
        org::apache::lucene::search::suggest::document::SuggestIndexSearcher>
        indexSearcher;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSuggestField> outerInstance,
        std::shared_ptr<Analyzer> analyzer, int num,
        const std::wstring &prefix1, const std::wstring &prefix2,
        const std::wstring &prefix3,
        std::deque<std::shared_ptr<org::apache::lucene::search::suggest::
                                        document::TestSuggestField::Entry>>
            &entries1,
        std::deque<std::shared_ptr<org::apache::lucene::search::suggest::
                                        document::TestSuggestField::Entry>>
            &entries2,
        std::deque<std::shared_ptr<org::apache::lucene::search::suggest::
                                        document::TestSuggestField::Entry>>
            &entries3,
        std::shared_ptr<CyclicBarrier> startingGun,
        std::shared_ptr<CopyOnWriteArrayList<std::runtime_error>> errors,
        std::shared_ptr<org::apache::lucene::search::suggest::document::
                            SuggestIndexSearcher>
            indexSearcher);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  class Entry : public std::enable_shared_from_this<Entry>
  {
    GET_CLASS_NAME(Entry)
  public:
    const std::wstring output;
    const float value;
    const std::wstring context;
    const int id;

    Entry(const std::wstring &output, float value);

    Entry(const std::wstring &output, const std::wstring &context, float value);

    Entry(const std::wstring &output, const std::wstring &context, float value,
          int id);

    virtual std::wstring toString();
  };

public:
  static void assertSuggestions(std::shared_ptr<TopDocs> actual,
                                std::deque<Entry> &expected);

private:
  static std::wstring toString(std::shared_ptr<Entry> expected);

  static std::wstring toString(std::shared_ptr<SuggestScoreDoc> actual);

public:
  static std::shared_ptr<IndexWriterConfig>
  iwcWithSuggestField(std::shared_ptr<Analyzer> analyzer,
                      std::deque<std::wstring> &suggestFields);

  static std::shared_ptr<IndexWriterConfig>
  iwcWithSuggestField(std::shared_ptr<Analyzer> analyzer,
                      std::shared_ptr<Set<std::wstring>> suggestFields);

private:
  class Lucene70CodecAnonymousInnerClass : public Lucene70Codec
  {
    GET_CLASS_NAME(Lucene70CodecAnonymousInnerClass)
  private:
    std::shared_ptr<Set<std::wstring>> suggestFields;

  public:
    Lucene70CodecAnonymousInnerClass(
        std::shared_ptr<Set<std::wstring>> suggestFields);

    std::shared_ptr<PostingsFormat> postingsFormat =
        std::make_shared<Completion50PostingsFormat>();

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<Lucene70CodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<Lucene70CodecAnonymousInnerClass>(
          org.apache.lucene.codecs.lucene70.Lucene70Codec::shared_from_this());
    }
  };

public:
  class PayloadAttrToTypeAttrFilter final : public TokenFilter
  {
    GET_CLASS_NAME(PayloadAttrToTypeAttrFilter)
  private:
    std::shared_ptr<PayloadAttribute> payload =
        addAttribute(PayloadAttribute::typeid);
    std::shared_ptr<TypeAttribute> type = addAttribute(TypeAttribute::typeid);

  protected:
    PayloadAttrToTypeAttrFilter(std::shared_ptr<TokenStream> input);

  public:
    bool incrementToken()  override;

  protected:
    std::shared_ptr<PayloadAttrToTypeAttrFilter> shared_from_this()
    {
      return std::static_pointer_cast<PayloadAttrToTypeAttrFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSuggestField> shared_from_this()
  {
    return std::static_pointer_cast<TestSuggestField>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::document
