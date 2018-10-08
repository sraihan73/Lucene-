using namespace std;

#include "TestPerFieldAnalyzerWrapper.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockCharFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/SimpleAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ASCIIFoldingFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/PerFieldAnalyzerWrapper.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockCharFilter = org::apache::lucene::analysis::MockCharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SimpleAnalyzer = org::apache::lucene::analysis::core::SimpleAnalyzer;
using WhitespaceAnalyzer =
    org::apache::lucene::analysis::core::WhitespaceAnalyzer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestPerFieldAnalyzerWrapper::testPerField() 
{
  wstring text = L"Qwerty";

  unordered_map<wstring, std::shared_ptr<Analyzer>> analyzerPerField =
      Collections::singletonMap<wstring, std::shared_ptr<Analyzer>>(
          L"special", make_shared<SimpleAnalyzer>());

  shared_ptr<Analyzer> defaultAnalyzer = make_shared<WhitespaceAnalyzer>();

  shared_ptr<PerFieldAnalyzerWrapper> analyzer =
      make_shared<PerFieldAnalyzerWrapper>(defaultAnalyzer, analyzerPerField);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // tokenStream = analyzer.tokenStream("field", text))
  {
    org::apache::lucene::analysis::TokenStream tokenStream =
        analyzer->tokenStream(L"field", text);
    shared_ptr<CharTermAttribute> termAtt =
        tokenStream->getAttribute(CharTermAttribute::typeid);
    tokenStream->reset();

    assertTrue(tokenStream->incrementToken());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"WhitespaceAnalyzer does not lowercase", L"Qwerty",
                 termAtt->toString());
    assertFalse(tokenStream->incrementToken());
    tokenStream->end();
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // tokenStream = analyzer.tokenStream("special", text))
  {
    org::apache::lucene::analysis::TokenStream tokenStream =
        analyzer->tokenStream(L"special", text);
    shared_ptr<CharTermAttribute> termAtt =
        tokenStream->getAttribute(CharTermAttribute::typeid);
    tokenStream->reset();

    assertTrue(tokenStream->incrementToken());
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(L"SimpleAnalyzer lowercases", L"qwerty", termAtt->toString());
    assertFalse(tokenStream->incrementToken());
    tokenStream->end();
  }
  // TODO: fix this about PFAW, this is crazy
  delete analyzer;
  delete defaultAnalyzer;
  IOUtils::close({analyzerPerField.values()});
}

void TestPerFieldAnalyzerWrapper::testReuseWrapped() 
{
  const wstring text = L"Qwerty";

  shared_ptr<Analyzer> *const specialAnalyzer = make_shared<SimpleAnalyzer>();
  shared_ptr<Analyzer> *const defaultAnalyzer =
      make_shared<WhitespaceAnalyzer>();

  shared_ptr<TokenStream> ts1, ts2, ts3, ts4;

  shared_ptr<PerFieldAnalyzerWrapper> *const wrapper1 =
      make_shared<PerFieldAnalyzerWrapper>(
          defaultAnalyzer,
          Collections::singletonMap<wstring, std::shared_ptr<Analyzer>>(
              L"special", specialAnalyzer));

  // test that the PerFieldWrapper returns the same instance as original
  // Analyzer:
  ts1 = defaultAnalyzer->tokenStream(L"something", text);
  ts2 = wrapper1->tokenStream(L"something", text);
  assertSame(ts1, ts2);

  ts1 = specialAnalyzer->tokenStream(L"special", text);
  ts2 = wrapper1->tokenStream(L"special", text);
  assertSame(ts1, ts2);

  // Wrap with another wrapper, which does *not* extend
  // DelegatingAnalyzerWrapper:
  shared_ptr<AnalyzerWrapper> *const wrapper2 =
      make_shared<AnalyzerWrapperAnonymousInnerClass>(
          shared_from_this(), wrapper1->getReuseStrategy(), text,
          specialAnalyzer, wrapper1);
  ts3 = wrapper2->tokenStream(L"special", text);
  assertNotSame(ts1, ts3);
  assertTrue(std::dynamic_pointer_cast<ASCIIFoldingFilter>(ts3) != nullptr);
  // check that cache did not get corrumpted:
  ts2 = wrapper1->tokenStream(L"special", text);
  assertSame(ts1, ts2);

  // Wrap PerField with another PerField. In that case all TokenStreams returned
  // must be the same:
  shared_ptr<PerFieldAnalyzerWrapper> *const wrapper3 =
      make_shared<PerFieldAnalyzerWrapper>(
          wrapper1,
          Collections::singletonMap<wstring, std::shared_ptr<Analyzer>>(
              L"moreSpecial", specialAnalyzer));
  ts1 = specialAnalyzer->tokenStream(L"special", text);
  ts2 = wrapper3->tokenStream(L"special", text);
  assertSame(ts1, ts2);
  ts3 = specialAnalyzer->tokenStream(L"moreSpecial", text);
  ts4 = wrapper3->tokenStream(L"moreSpecial", text);
  assertSame(ts3, ts4);
  assertSame(ts2, ts3);
  IOUtils::close(
      {wrapper3, wrapper2, wrapper1, specialAnalyzer, defaultAnalyzer});
}

TestPerFieldAnalyzerWrapper::AnalyzerWrapperAnonymousInnerClass::
    AnalyzerWrapperAnonymousInnerClass(
        shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance,
        shared_ptr<Analyzer::ReuseStrategy> getReuseStrategy,
        const wstring &text, shared_ptr<Analyzer> specialAnalyzer,
        shared_ptr<org::apache::lucene::analysis::miscellaneous::
                       PerFieldAnalyzerWrapper>
            wrapper1)
    : org::apache::lucene::analysis::AnalyzerWrapper(getReuseStrategy)
{
  this->outerInstance = outerInstance;
  this->text = text;
  this->specialAnalyzer = specialAnalyzer;
  this->wrapper1 = wrapper1;
}

shared_ptr<Analyzer>
TestPerFieldAnalyzerWrapper::AnalyzerWrapperAnonymousInnerClass::
    getWrappedAnalyzer(const wstring &fieldName)
{
  return wrapper1;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPerFieldAnalyzerWrapper::AnalyzerWrapperAnonymousInnerClass::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  assertNotSame(specialAnalyzer->tokenStream(L"special", text),
                components->getTokenStream());
  shared_ptr<TokenFilter> filter =
      make_shared<ASCIIFoldingFilter>(components->getTokenStream());
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(), filter);
}

void TestPerFieldAnalyzerWrapper::testCharFilters() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  assertAnalyzesTo(a, L"ab", std::deque<wstring>{L"aab"}, std::deque<int>{0},
                   std::deque<int>{2});

  // now wrap in PFAW
  shared_ptr<PerFieldAnalyzerWrapper> p = make_shared<PerFieldAnalyzerWrapper>(
      a, Collections::emptyMap<wstring, std::shared_ptr<Analyzer>>());

  assertAnalyzesTo(p, L"ab", std::deque<wstring>{L"aab"}, std::deque<int>{0},
                   std::deque<int>{2});
  delete p;
  delete a; // TODO: fix this about PFAW, its a trap
}

TestPerFieldAnalyzerWrapper::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPerFieldAnalyzerWrapper> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPerFieldAnalyzerWrapper::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

shared_ptr<Reader>
TestPerFieldAnalyzerWrapper::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<MockCharFilter>(reader, 7);
}
} // namespace org::apache::lucene::analysis::miscellaneous