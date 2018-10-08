using namespace std;

#include "TestRemoveDuplicatesTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/RemoveDuplicatesTokenFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SynonymFilter = org::apache::lucene::analysis::synonym::SynonymFilter;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharsRef = org::apache::lucene::util::CharsRef;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Token> TestRemoveDuplicatesTokenFilter::tok(int pos,
                                                       const wstring &t,
                                                       int start, int end)
{
  shared_ptr<Token> tok = make_shared<Token>(t, start, end);
  tok->setPositionIncrement(pos);
  return tok;
}

shared_ptr<Token> TestRemoveDuplicatesTokenFilter::tok(int pos,
                                                       const wstring &t)
{
  return tok(pos, t, 0, 0);
}

void TestRemoveDuplicatesTokenFilter::testDups(
    const wstring &expected, deque<Token> &tokens) 
{

  const Iterator<std::shared_ptr<Token>> toks = Arrays::asList(tokens).begin();
  shared_ptr<TokenStream> *const ts = make_shared<RemoveDuplicatesTokenFilter>(
      (make_shared<TokenStreamAnonymousInnerClass>(shared_from_this(), toks)));

  assertTokenStreamContents(ts, expected.split(L"\\s"));
}

TestRemoveDuplicatesTokenFilter::TokenStreamAnonymousInnerClass::
    TokenStreamAnonymousInnerClass(
        shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance,
        shared_ptr<Iterator<std::shared_ptr<Token>>> toks)
{
  this->outerInstance = outerInstance;
  this->toks = toks;
  termAtt = addAttribute(CharTermAttribute::typeid);
  offsetAtt = addAttribute(OffsetAttribute::typeid);
  posIncAtt = addAttribute(PositionIncrementAttribute::typeid);
}

bool TestRemoveDuplicatesTokenFilter::TokenStreamAnonymousInnerClass::
    incrementToken()
{
  if (toks->hasNext()) {
    clearAttributes();
    shared_ptr<Token> tok = toks->next();
    termAtt::setEmpty()->append(tok);
    offsetAtt::setOffset(tok->startOffset(), tok->endOffset());
    posIncAtt::setPositionIncrement(tok->getPositionIncrement());
    return true;
  } else {
    return false;
  }
}

void TestRemoveDuplicatesTokenFilter::testNoDups() 
{

  testDups(L"A B B C D E",
           {tok(1, L"A", 0, 4), tok(1, L"B", 5, 10), tok(1, L"B", 11, 15),
            tok(1, L"C", 16, 20), tok(0, L"D", 16, 20), tok(1, L"E", 21, 25)});
}

void TestRemoveDuplicatesTokenFilter::testSimpleDups() 
{

  testDups(L"A B C D E",
           {tok(1, L"A", 0, 4), tok(1, L"B", 5, 10), tok(0, L"B", 11, 15),
            tok(1, L"C", 16, 20), tok(0, L"D", 16, 20), tok(1, L"E", 21, 25)});
}

void TestRemoveDuplicatesTokenFilter::testComplexDups() 
{

  testDups(L"A B C D E F G H I J K",
           {tok(1, L"A"), tok(1, L"B"), tok(0, L"B"), tok(1, L"C"),
            tok(1, L"D"), tok(0, L"D"), tok(0, L"D"), tok(1, L"E"),
            tok(1, L"F"), tok(0, L"F"), tok(1, L"G"), tok(0, L"H"),
            tok(0, L"H"), tok(1, L"I"), tok(1, L"J"), tok(0, L"K"),
            tok(0, L"J")});
}

wstring TestRemoveDuplicatesTokenFilter::randomNonEmptyString()
{
  while (true) {
    const wstring s =
        StringHelper::trim(TestUtil::randomUnicodeString(random()));
    if (s.length() != 0 && s.find(L'\u0000') == wstring::npos) {
      return s;
    }
  }
}

void TestRemoveDuplicatesTokenFilter::add(shared_ptr<SynonymMap::Builder> b,
                                          const wstring &input,
                                          const wstring &output, bool keepOrig)
{
  b->add(make_shared<CharsRef>(input.replaceAll(L" +", L"\u0000")),
         make_shared<CharsRef>(output.replaceAll(L" +", L"\u0000")), keepOrig);
}

void TestRemoveDuplicatesTokenFilter::testRandomStrings() 
{
  constexpr int numIters = atLeast(10);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<SynonymMap::Builder> b =
        make_shared<SynonymMap::Builder>(random()->nextBoolean());
    constexpr int numEntries = atLeast(10);
    for (int j = 0; j < numEntries; j++) {
      add(b, randomNonEmptyString(), randomNonEmptyString(),
          random()->nextBoolean());
    }
    shared_ptr<SynonymMap> *const map_obj = b->build();
    constexpr bool ignoreCase = random()->nextBoolean();

    shared_ptr<Analyzer> *const analyzer =
        make_shared<AnalyzerAnonymousInnerClass>(shared_from_this(), map_obj,
                                                 ignoreCase);

    checkRandomData(random(), analyzer, 200);
    delete analyzer;
  }
}

TestRemoveDuplicatesTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance,
        shared_ptr<SynonymMap> map_obj, bool ignoreCase)
{
  this->outerInstance = outerInstance;
  this->map_obj = map_obj;
  this->ignoreCase = ignoreCase;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestRemoveDuplicatesTokenFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::SIMPLE, true);
  shared_ptr<TokenStream> stream =
      make_shared<SynonymFilter>(tokenizer, map_obj, ignoreCase);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<RemoveDuplicatesTokenFilter>(stream));
}

void TestRemoveDuplicatesTokenFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestRemoveDuplicatesTokenFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestRemoveDuplicatesTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestRemoveDuplicatesTokenFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<RemoveDuplicatesTokenFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::miscellaneous