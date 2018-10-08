using namespace std;

#include "TestHindiStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/hi/HindiStemFilter.h"

namespace org::apache::lucene::analysis::hi
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestHindiStemmer::testMasculineNouns() 
{
  check(L"लडका", L"लडक");
  check(L"लडके", L"लडक");
  check(L"लडकों", L"लडक");

  check(L"गुरु", L"गुर");
  check(L"गुरुओं", L"गुर");

  check(L"दोस्त", L"दोस्त");
  check(L"दोस्तों", L"दोस्त");
}

void TestHindiStemmer::testFeminineNouns() 
{
  check(L"लडकी", L"लडक");
  check(L"लडकियों", L"लडक");

  check(L"किताब", L"किताब");
  check(L"किताबें", L"किताब");
  check(L"किताबों", L"किताब");

  check(L"आध्यापीका", L"आध्यापीक");
  check(L"आध्यापीकाएं", L"आध्यापीक");
  check(L"आध्यापीकाओं", L"आध्यापीक");
}

void TestHindiStemmer::testVerbs() 
{
  check(L"खाना", L"खा");
  check(L"खाता", L"खा");
  check(L"खाती", L"खा");
  check(L"खा", L"खा");
}

void TestHindiStemmer::testExceptions() 
{
  check(L"कठिनाइयां", L"कठिन");
  check(L"कठिन", L"कठिन");
}

void TestHindiStemmer::check(const wstring &input,
                             const wstring &output) 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(input);
  shared_ptr<TokenFilter> tf = make_shared<HindiStemFilter>(tokenizer);
  assertTokenStreamContents(tf, std::deque<wstring>{output});
}

void TestHindiStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestHindiStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestHindiStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHindiStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HindiStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::hi