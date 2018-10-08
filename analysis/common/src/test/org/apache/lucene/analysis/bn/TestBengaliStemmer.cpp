using namespace std;

#include "TestBengaliStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/bn/BengaliStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"

namespace org::apache::lucene::analysis::bn
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestBengaliStemmer::testVerbsInShadhuForm() 
{
  check(L"করেছিলাম", L"কর");
  check(L"করিতেছিলে", L"কর");
  check(L"খাইতাম", L"খাই");
  check(L"যাইবে", L"যা");
}

void TestBengaliStemmer::testVerbsInCholitoForm() 
{
  check(L"করছিলাম", L"কর");
  check(L"করছিলে", L"কর");
  check(L"করতাম", L"কর");
  check(L"যাব", L"যা");
  check(L"যাবে", L"যা");
  check(L"করি", L"কর");
  check(L"করো", L"কর");
}

void TestBengaliStemmer::testNouns() 
{
  check(L"মেয়েরা", L"মে");
  check(L"মেয়েদেরকে", L"মে");
  check(L"মেয়েদের", L"মে");

  check(L"একটি", L"এক");
  check(L"মানুষগুলি", L"মানুষ");
}

void TestBengaliStemmer::check(const wstring &input,
                               const wstring &output) 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(input);
  shared_ptr<TokenFilter> tf = make_shared<BengaliStemFilter>(tokenizer);
  assertTokenStreamContents(tf, std::deque<wstring>{output});
}

void TestBengaliStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestBengaliStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestBengaliStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestBengaliStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<BengaliStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::bn