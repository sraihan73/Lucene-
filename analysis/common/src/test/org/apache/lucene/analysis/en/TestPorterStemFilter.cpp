using namespace std;

#include "TestPorterStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/VocabularyAssert.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/en/PorterStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::en
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
//    import static org.apache.lucene.analysis.VocabularyAssert.*;

void TestPorterStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestPorterStemFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPorterStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPorterStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<PorterStemFilter>(t));
}

void TestPorterStemFilter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestPorterStemFilter::testPorterStemFilter() 
{
  VocabularyAssert::assertVocabulary(a, getDataPath(L"porterTestData.zip"),
                                     L"voc.txt", L"output.txt");
}

void TestPorterStemFilter::testWithKeywordAttribute() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(1, true);
  set->add(L"yourselves");
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(L"yourselves yours"));
  shared_ptr<TokenStream> filter = make_shared<PorterStemFilter>(
      make_shared<SetKeywordMarkerFilter>(tokenizer, set));
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"yourselves", L"your"});
}

void TestPorterStemFilter::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestPorterStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestPorterStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(shared_ptr<TestPorterStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPorterStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PorterStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::en