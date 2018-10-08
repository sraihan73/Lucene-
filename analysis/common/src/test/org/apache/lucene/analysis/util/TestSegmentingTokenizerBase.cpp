using namespace std;

#include "TestSegmentingTokenizerBase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"

namespace org::apache::lucene::analysis::util
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestSegmentingTokenizerBase::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  sentence = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  sentenceAndWord =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestSegmentingTokenizerBase::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSegmentingTokenizerBase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSegmentingTokenizerBase::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<WholeSentenceTokenizer>());
}

TestSegmentingTokenizerBase::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSegmentingTokenizerBase> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSegmentingTokenizerBase::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<SentenceAndWordTokenizer>());
}

void TestSegmentingTokenizerBase::tearDown() 
{
  IOUtils::close({sentence, sentenceAndWord});
  BaseTokenStreamTestCase::tearDown();
}

void TestSegmentingTokenizerBase::testBasics() 
{
  assertAnalyzesTo(
      sentence,
      L"The acronym for United States is U.S. but this doesn't end a sentence",
      std::deque<wstring>{L"The acronym for United States is U.S. but this "
                           L"doesn't end a sentence"});
  assertAnalyzesTo(sentence,
                   L"He said, \"Are you going?\" John shook his head.",
                   std::deque<wstring>{L"He said, \"Are you going?\" ",
                                        L"John shook his head."});
}

void TestSegmentingTokenizerBase::testCustomAttributes() 
{
  assertAnalyzesTo(
      sentenceAndWord, L"He said, \"Are you going?\" John shook his head.",
      std::deque<wstring>{L"He", L"said", L"Are", L"you", L"going", L"John",
                           L"shook", L"his", L"head"},
      std::deque<int>{0, 3, 10, 14, 18, 26, 31, 37, 41},
      std::deque<int>{2, 7, 13, 17, 23, 30, 36, 40, 45},
      std::deque<int>{1, 1, 1, 1, 1, 2, 1, 1, 1});
}

void TestSegmentingTokenizerBase::testReuse() 
{
  assertAnalyzesTo(
      sentenceAndWord, L"He said, \"Are you going?\"",
      std::deque<wstring>{L"He", L"said", L"Are", L"you", L"going"},
      std::deque<int>{0, 3, 10, 14, 18}, std::deque<int>{2, 7, 13, 17, 23},
      std::deque<int>{1, 1, 1, 1, 1});
  assertAnalyzesTo(sentenceAndWord, L"John shook his head.",
                   std::deque<wstring>{L"John", L"shook", L"his", L"head"},
                   std::deque<int>{0, 5, 11, 15},
                   std::deque<int>{4, 10, 14, 19},
                   std::deque<int>{1, 1, 1, 1});
}

void TestSegmentingTokenizerBase::testEnd() 
{
  // BaseTokenStreamTestCase asserts that end() is set to our StringReader's
  // length for us here. we add some junk whitespace to the end just to test it.
  assertAnalyzesTo(sentenceAndWord, L"John shook his head          ",
                   std::deque<wstring>{L"John", L"shook", L"his", L"head"});
  assertAnalyzesTo(sentenceAndWord, L"John shook his head.          ",
                   std::deque<wstring>{L"John", L"shook", L"his", L"head"});
}

void TestSegmentingTokenizerBase::testHugeDoc() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wchar_t> whitespace(4094);
  Arrays::fill(whitespace, L'\n');
  sb->append(whitespace);
  sb->append(L"testing 1234");
  wstring input = sb->toString();
  assertAnalyzesTo(sentenceAndWord, input,
                   std::deque<wstring>{L"testing", L"1234"});
}

void TestSegmentingTokenizerBase::testHugeTerm() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < 10240; i++) {
    sb->append(L'a');
  }
  wstring input = sb->toString();
  std::deque<wchar_t> token(1024);
  Arrays::fill(token, L'a');
  wstring expectedToken = wstring(token);
  std::deque<wstring> expected = {expectedToken, expectedToken, expectedToken,
                                   expectedToken, expectedToken, expectedToken,
                                   expectedToken, expectedToken, expectedToken,
                                   expectedToken};
  assertAnalyzesTo(sentence, input, expected);
}

void TestSegmentingTokenizerBase::testRandomStrings() 
{
  checkRandomData(random(), sentence, 10000 * RANDOM_MULTIPLIER);
  checkRandomData(random(), sentenceAndWord, 10000 * RANDOM_MULTIPLIER);
}

TestSegmentingTokenizerBase::WholeSentenceTokenizer::WholeSentenceTokenizer()
    : SegmentingTokenizerBase(newAttributeFactory(),
                              BreakIterator::getSentenceInstance(Locale::ROOT))
{
}

void TestSegmentingTokenizerBase::WholeSentenceTokenizer::setNextSentence(
    int sentenceStart, int sentenceEnd)
{
  this->sentenceStart = sentenceStart;
  this->sentenceEnd = sentenceEnd;
  hasSentence = true;
}

bool TestSegmentingTokenizerBase::WholeSentenceTokenizer::incrementWord()
{
  if (hasSentence) {
    hasSentence = false;
    clearAttributes();
    termAtt->copyBuffer(buffer, sentenceStart, sentenceEnd - sentenceStart);
    offsetAtt->setOffset(correctOffset(offset + sentenceStart),
                         correctOffset(offset + sentenceEnd));
    return true;
  } else {
    return false;
  }
}

TestSegmentingTokenizerBase::SentenceAndWordTokenizer::
    SentenceAndWordTokenizer()
    : SegmentingTokenizerBase(newAttributeFactory(),
                              BreakIterator::getSentenceInstance(Locale::ROOT))
{
}

void TestSegmentingTokenizerBase::SentenceAndWordTokenizer::setNextSentence(
    int sentenceStart, int sentenceEnd)
{
  this->wordStart = this->wordEnd = this->sentenceStart = sentenceStart;
  this->sentenceEnd = sentenceEnd;
  posBoost++;
}

void TestSegmentingTokenizerBase::SentenceAndWordTokenizer::reset() throw(
    IOException)
{
  SegmentingTokenizerBase::reset();
  posBoost = -1;
}

bool TestSegmentingTokenizerBase::SentenceAndWordTokenizer::incrementWord()
{
  wordStart = wordEnd;
  while (wordStart < sentenceEnd) {
    if (isalnum(buffer[wordStart])) {
      break;
    }
    wordStart++;
  }

  if (wordStart == sentenceEnd) {
    return false;
  }

  wordEnd = wordStart + 1;
  while (wordEnd < sentenceEnd && isalnum(buffer[wordEnd])) {
    wordEnd++;
  }

  clearAttributes();
  termAtt->copyBuffer(buffer, wordStart, wordEnd - wordStart);
  offsetAtt->setOffset(correctOffset(offset + wordStart),
                       correctOffset(offset + wordEnd));
  posIncAtt->setPositionIncrement(posIncAtt->getPositionIncrement() + posBoost);
  posBoost = 0;
  return true;
}
} // namespace org::apache::lucene::analysis::util