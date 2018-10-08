using namespace std;

#include "TestDaitchMokotoffSoundexFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/phonetic/DaitchMokotoffSoundexFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;

void TestDaitchMokotoffSoundexFilter::testAlgorithms() 
{
  assertAlgorithm(true, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"aaa", L"000000", L"bbb", L"700000",
                                       L"ccc", L"400000", L"450000", L"454000",
                                       L"540000", L"545000", L"500000",
                                       L"easgasg", L"045450"});
  assertAlgorithm(false, L"aaa bbb ccc easgasg",
                  std::deque<wstring>{L"000000", L"700000", L"400000",
                                       L"450000", L"454000", L"540000",
                                       L"545000", L"500000", L"045450"});
}

void TestDaitchMokotoffSoundexFilter::assertAlgorithm(
    bool inject, const wstring &input,
    std::deque<wstring> &expected) 
{
  shared_ptr<Tokenizer> tokenizer = make_shared<WhitespaceTokenizer>();
  tokenizer->setReader(make_shared<StringReader>(input));
  shared_ptr<DaitchMokotoffSoundexFilter> filter =
      make_shared<DaitchMokotoffSoundexFilter>(tokenizer, inject);
  assertTokenStreamContents(filter, expected);
}

void TestDaitchMokotoffSoundexFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;

  shared_ptr<Analyzer> b =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());

  checkRandomData(random(), b, 1000 * RANDOM_MULTIPLIER);
  delete b;
}

TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestDaitchMokotoffSoundexFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DaitchMokotoffSoundexFilter>(tokenizer, false));
}

TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestDaitchMokotoffSoundexFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DaitchMokotoffSoundexFilter>(tokenizer, false));
}

void TestDaitchMokotoffSoundexFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestDaitchMokotoffSoundexFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestDaitchMokotoffSoundexFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<DaitchMokotoffSoundexFilter>(
                     tokenizer, random()->nextBoolean()));
}
} // namespace org::apache::lucene::analysis::phonetic