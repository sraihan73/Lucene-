using namespace std;

#include "TestStopAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopAnalyzer.h"

namespace org::apache::lucene::analysis::core
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

void TestStopAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();

  CharArraySet::const_iterator it =
      StopAnalyzer::ENGLISH_STOP_WORDS_SET->begin();
  while (it != StopAnalyzer::ENGLISH_STOP_WORDS_SET->end()) {
    inValidTokens->add(*it);
    it++;
  }
  stop = make_shared<StopAnalyzer>();
}

void TestStopAnalyzer::tearDown() 
{
  delete stop;
  BaseTokenStreamTestCase::tearDown();
}

void TestStopAnalyzer::testDefaults() 
{
  assertTrue(stop != nullptr);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = stop.tokenStream("test", "This is a test of the english stop
  // analyzer"))
  {
    org::apache::lucene::analysis::TokenStream stream = stop->tokenStream(
        L"test", L"This is a test of the english stop analyzer");
    assertTrue(stream != nullptr);
    shared_ptr<CharTermAttribute> termAtt =
        stream->getAttribute(CharTermAttribute::typeid);
    stream->reset();

    while (stream->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertFalse(inValidTokens->contains(termAtt->toString()));
    }
    stream->end();
  }
}

void TestStopAnalyzer::testStopList() 
{
  shared_ptr<CharArraySet> stopWordsSet =
      make_shared<CharArraySet>(asSet({L"good", L"test", L"analyzer"}), false);
  shared_ptr<StopAnalyzer> newStop = make_shared<StopAnalyzer>(stopWordsSet);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = newStop.tokenStream("test", "This is a good test of the english
  // stop analyzer"))
  {
    org::apache::lucene::analysis::TokenStream stream = newStop->tokenStream(
        L"test", L"This is a good test of the english stop analyzer");
    assertNotNull(stream);
    shared_ptr<CharTermAttribute> termAtt =
        stream->getAttribute(CharTermAttribute::typeid);

    stream->reset();
    while (stream->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring text = termAtt->toString();
      assertFalse(stopWordsSet->contains(text));
    }
    stream->end();
  }
  delete newStop;
}

void TestStopAnalyzer::testStopListPositions() 
{
  shared_ptr<CharArraySet> stopWordsSet =
      make_shared<CharArraySet>(asSet({L"good", L"test", L"analyzer"}), false);
  shared_ptr<StopAnalyzer> newStop = make_shared<StopAnalyzer>(stopWordsSet);
  wstring s =
      L"This is a good test of the english stop analyzer with positions";
  std::deque<int> expectedIncr = {1, 1, 1, 3, 1, 1, 1, 2, 1};
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream
  // stream = newStop.tokenStream("test", s))
  {
    org::apache::lucene::analysis::TokenStream stream =
        newStop->tokenStream(L"test", s);
    assertNotNull(stream);
    int i = 0;
    shared_ptr<CharTermAttribute> termAtt =
        stream->getAttribute(CharTermAttribute::typeid);
    shared_ptr<PositionIncrementAttribute> posIncrAtt =
        stream->addAttribute(PositionIncrementAttribute::typeid);

    stream->reset();
    while (stream->incrementToken()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring text = termAtt->toString();
      assertFalse(stopWordsSet->contains(text));
      assertEquals(expectedIncr[i++], posIncrAtt->getPositionIncrement());
    }
    stream->end();
  }
  delete newStop;
}
} // namespace org::apache::lucene::analysis::core