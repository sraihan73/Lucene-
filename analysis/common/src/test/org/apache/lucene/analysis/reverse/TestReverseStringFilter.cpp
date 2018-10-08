using namespace std;

#include "TestReverseStringFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/reverse/ReverseStringFilter.h"

namespace org::apache::lucene::analysis::reverse
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestReverseStringFilter::testFilter() 
{
  shared_ptr<TokenStream> stream = make_shared<MockTokenizer>(
      MockTokenizer::WHITESPACE, false); // 1-4 length string
  (std::static_pointer_cast<Tokenizer>(stream))
      ->setReader(make_shared<StringReader>(L"Do have a nice day"));
  shared_ptr<ReverseStringFilter> filter =
      make_shared<ReverseStringFilter>(stream);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"oD", L"evah", L"a", L"ecin", L"yad"});
}

void TestReverseStringFilter::testFilterWithMark() 
{
  shared_ptr<TokenStream> stream = make_shared<MockTokenizer>(
      MockTokenizer::WHITESPACE, false); // 1-4 length string
  (std::static_pointer_cast<Tokenizer>(stream))
      ->setReader(make_shared<StringReader>(L"Do have a nice day"));
  shared_ptr<ReverseStringFilter> filter =
      make_shared<ReverseStringFilter>(stream, L'\u0001');
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"\u0001oD", L"\u0001evah", L"\u0001a",
                                   L"\u0001ecin", L"\u0001yad"});
}

void TestReverseStringFilter::testReverseString() 
{
  assertEquals(L"A", ReverseStringFilter::reverse(L"A"));
  assertEquals(L"BA", ReverseStringFilter::reverse(L"AB"));
  assertEquals(L"CBA", ReverseStringFilter::reverse(L"ABC"));
}

void TestReverseStringFilter::testReverseChar() 
{
  std::deque<wchar_t> buffer = {L'A', L'B', L'C', L'D', L'E', L'F'};
  ReverseStringFilter::reverse(buffer, 2, 3);
  assertEquals(L"ABEDCF", wstring(buffer));
}

void TestReverseStringFilter::testReverseSupplementary() 
{
  // supplementary at end
  assertEquals(L"𩬅艱鍟䇹愯瀛", ReverseStringFilter::reverse(L"瀛愯䇹鍟艱𩬅"));
  // supplementary at end - 1
  assertEquals(L"a𩬅艱鍟䇹愯瀛",
               ReverseStringFilter::reverse(L"瀛愯䇹鍟艱𩬅a"));
  // supplementary at start
  assertEquals(L"fedcba𩬅", ReverseStringFilter::reverse(L"𩬅abcdef"));
  // supplementary at start + 1
  assertEquals(L"fedcba𩬅z", ReverseStringFilter::reverse(L"z𩬅abcdef"));
  // supplementary medial
  assertEquals(L"gfe𩬅dcba", ReverseStringFilter::reverse(L"abcd𩬅efg"));
}

void TestReverseStringFilter::testReverseSupplementaryChar() throw(
    runtime_error)
{
  // supplementary at end
  std::deque<wchar_t> buffer = (wstring(L"abc瀛愯䇹鍟艱𩬅")).toCharArray();
  ReverseStringFilter::reverse(buffer, 3, 7);
  assertEquals(L"abc𩬅艱鍟䇹愯瀛", wstring(buffer));
  // supplementary at end - 1
  buffer = (wstring(L"abc瀛愯䇹鍟艱𩬅d")).toCharArray();
  ReverseStringFilter::reverse(buffer, 3, 8);
  assertEquals(L"abcd𩬅艱鍟䇹愯瀛", wstring(buffer));
  // supplementary at start
  buffer = (wstring(L"abc𩬅瀛愯䇹鍟艱")).toCharArray();
  ReverseStringFilter::reverse(buffer, 3, 7);
  assertEquals(L"abc艱鍟䇹愯瀛𩬅", wstring(buffer));
  // supplementary at start + 1
  buffer = (wstring(L"abcd𩬅瀛愯䇹鍟艱")).toCharArray();
  ReverseStringFilter::reverse(buffer, 3, 8);
  assertEquals(L"abc艱鍟䇹愯瀛𩬅d", wstring(buffer));
  // supplementary medial
  buffer = (wstring(L"abc瀛愯𩬅def")).toCharArray();
  ReverseStringFilter::reverse(buffer, 3, 7);
  assertEquals(L"abcfed𩬅愯瀛", wstring(buffer));
}

void TestReverseStringFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestReverseStringFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestReverseStringFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestReverseStringFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ReverseStringFilter>(tokenizer));
}

void TestReverseStringFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestReverseStringFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestReverseStringFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestReverseStringFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ReverseStringFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::reverse