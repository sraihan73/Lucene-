using namespace std;

#include "TestKeepWordFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/KeepWordFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestKeepWordFilter::testStopAndGo() 
{
  shared_ptr<Set<wstring>> words = unordered_set<wstring>();
  words->add(L"aaa");
  words->add(L"bbb");

  wstring input = L"xxx yyy aaa zzz BBB ccc ddd EEE";

  // Test Stopwords
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(input);
  stream = make_shared<KeepWordFilter>(stream,
                                       make_shared<CharArraySet>(words, true));
  assertTokenStreamContents(stream, std::deque<wstring>{L"aaa", L"BBB"},
                            std::deque<int>{3, 2});

  // Now force case
  stream = whitespaceMockTokenizer(input);
  stream = make_shared<KeepWordFilter>(stream,
                                       make_shared<CharArraySet>(words, false));
  assertTokenStreamContents(stream, std::deque<wstring>{L"aaa"},
                            std::deque<int>{3});
}

void TestKeepWordFilter::testRandomStrings() 
{
  shared_ptr<Set<wstring>> *const words = unordered_set<wstring>();
  words->add(L"a");
  words->add(L"b");

  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), words);

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestKeepWordFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestKeepWordFilter> outerInstance,
    shared_ptr<Set<wstring>> words)
{
  this->outerInstance = outerInstance;
  this->words = words;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKeepWordFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> stream = make_shared<KeepWordFilter>(
      tokenizer, make_shared<CharArraySet>(words, true));
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}
} // namespace org::apache::lucene::analysis::miscellaneous