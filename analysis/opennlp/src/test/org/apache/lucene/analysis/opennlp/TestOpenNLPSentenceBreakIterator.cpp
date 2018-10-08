using namespace std;

#include "TestOpenNLPSentenceBreakIterator.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ClasspathResourceLoader.h"
#include "../../../../../../java/org/apache/lucene/analysis/opennlp/OpenNLPSentenceBreakIterator.h"
#include "../../../../../../java/org/apache/lucene/analysis/opennlp/tools/NLPSentenceDetectorOp.h"
#include "../../../../../../java/org/apache/lucene/analysis/opennlp/tools/OpenNLPOpsFactory.h"

namespace org::apache::lucene::analysis::opennlp
{
using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;
using OpenNLPOpsFactory =
    org::apache::lucene::analysis::opennlp::tools::OpenNLPOpsFactory;
using CharArrayIterator =
    org::apache::lucene::analysis::util::CharArrayIterator;
using ClasspathResourceLoader =
    org::apache::lucene::analysis::util::ClasspathResourceLoader;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::BeforeClass;
const wstring TestOpenNLPSentenceBreakIterator::TEXT =
    L"Sentence number 1 has 6 words. Sentence number 2, 5 words. And finally, "
    L"sentence number 3 has 8 words.";
std::deque<wstring> const TestOpenNLPSentenceBreakIterator::SENTENCES =
    std::deque<wstring>{L"Sentence number 1 has 6 words. ",
                         L"Sentence number 2, 5 words. ",
                         L"And finally, sentence number 3 has 8 words."};
const wstring TestOpenNLPSentenceBreakIterator::PADDING = L" Word. Word. ";
const wstring TestOpenNLPSentenceBreakIterator::sentenceModelFile =
    L"en-test-sent.bin";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void populateCache() throws
// java.io.IOException
void TestOpenNLPSentenceBreakIterator::populateCache() 
{
  OpenNLPOpsFactory::getSentenceModel(
      sentenceModelFile, make_shared<ClasspathResourceLoader>(
                             TestOpenNLPSentenceBreakIterator::typeid));
}

void TestOpenNLPSentenceBreakIterator::testThreeSentences() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(TEXT); // std::wstring is converted to StringCharacterIterator
  do3SentenceTest(bi);

  bi->setText(getCharArrayIterator(TEXT));
  do3SentenceTest(bi);
}

shared_ptr<CharacterIterator>
TestOpenNLPSentenceBreakIterator::getCharArrayIterator(const wstring &text)
{
  return getCharArrayIterator(text, 0, text.length());
}

shared_ptr<CharacterIterator>
TestOpenNLPSentenceBreakIterator::getCharArrayIterator(const wstring &text,
                                                       int start, int length)
{
  shared_ptr<CharArrayIterator> charArrayIterator =
      make_shared<CharArrayIteratorAnonymousInnerClass>(shared_from_this());
  charArrayIterator->setText(text.toCharArray(), start, length);
  return charArrayIterator;
}

TestOpenNLPSentenceBreakIterator::CharArrayIteratorAnonymousInnerClass::
    CharArrayIteratorAnonymousInnerClass(
        shared_ptr<TestOpenNLPSentenceBreakIterator> outerInstance)
{
  this->outerInstance = outerInstance;
}

wchar_t TestOpenNLPSentenceBreakIterator::CharArrayIteratorAnonymousInnerClass::
    jreBugWorkaround(wchar_t ch)
{
  return ch >= 0xD800 && ch <= 0xDFFF ? 0x002C : ch;
}

void TestOpenNLPSentenceBreakIterator::do3SentenceTest(
    shared_ptr<BreakIterator> bi)
{
  assertEquals(0, bi->current());
  assertEquals(0, bi->first());
  assertEquals(SENTENCES[0],
               TEXT.substr(bi->current(), bi->next() - bi->current()));
  assertEquals(SENTENCES[1],
               TEXT.substr(bi->current(), bi->next() - bi->current()));
  int current = bi->current();
  assertEquals(bi->getText().getEndIndex(), bi->next());
  int next = bi->current();
  assertEquals(SENTENCES[2], TEXT.substr(current, next - current));
  assertEquals(BreakIterator::DONE, bi->next());

  assertEquals(TEXT.length(), bi->last());
  int end = bi->current();
  assertEquals(SENTENCES[2], TEXT.substr(bi->previous(), end - bi->previous()));
  end = bi->current();
  assertEquals(SENTENCES[1], TEXT.substr(bi->previous(), end - bi->previous()));
  end = bi->current();
  assertEquals(SENTENCES[0], TEXT.substr(bi->previous(), end - bi->previous()));
  assertEquals(BreakIterator::DONE, bi->previous());
  assertEquals(0, bi->current());

  assertEquals(59, bi->following(39));
  assertEquals(59, bi->following(31));
  assertEquals(31, bi->following(30));

  assertEquals(0, bi->preceding(57));
  assertEquals(0, bi->preceding(58));
  assertEquals(31, bi->preceding(59));

  assertEquals(0, bi->first());
  assertEquals(59, bi->next(2));
  assertEquals(0, bi->next(-2));
}

void TestOpenNLPSentenceBreakIterator::testSingleSentence() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(getCharArrayIterator(SENTENCES[0]));
  test1Sentence(bi, SENTENCES[0]);
}

void TestOpenNLPSentenceBreakIterator::test1Sentence(
    shared_ptr<BreakIterator> bi, const wstring &text)
{
  int start = bi->getText().getBeginIndex();
  assertEquals(start, bi->first());
  int current = bi->current();
  assertEquals(bi->getText().getEndIndex(), bi->next());
  int end = bi->current() - start;
  assertEquals(text,
               text.substr(current - start, (end - start) - (current - start)));

  assertEquals(text.length(), bi->last() - start);
  end = bi->current();
  bi->previous();
  assertEquals(BreakIterator::DONE, bi->previous());
  int previous = bi->current();
  assertEquals(
      text, text.substr(previous - start, (end - start) - (previous - start)));
  assertEquals(start, bi->current());

  assertEquals(BreakIterator::DONE, bi->following(bi->last() / 2 + start));

  assertEquals(BreakIterator::DONE, bi->preceding(bi->last() / 2 + start));

  assertEquals(start, bi->first());
  assertEquals(BreakIterator::DONE, bi->next(13));
  assertEquals(BreakIterator::DONE, bi->next(-8));
}

void TestOpenNLPSentenceBreakIterator::testSliceEnd() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(
      getCharArrayIterator(SENTENCES[0] + PADDING, 0, SENTENCES[0].length()));

  test1Sentence(bi, SENTENCES[0]);
}

void TestOpenNLPSentenceBreakIterator::testSliceStart() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(getCharArrayIterator(PADDING + SENTENCES[0], PADDING.length(),
                                   SENTENCES[0].length()));
  test1Sentence(bi, SENTENCES[0]);
}

void TestOpenNLPSentenceBreakIterator::testSliceMiddle() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(getCharArrayIterator(PADDING + SENTENCES[0] + PADDING,
                                   PADDING.length(), SENTENCES[0].length()));

  test1Sentence(bi, SENTENCES[0]);
}

void TestOpenNLPSentenceBreakIterator::testFirstPosition() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(getCharArrayIterator(SENTENCES[0]));
  assertEquals(SENTENCES[0].length(),
               bi->last()); // side-effect: set current position to last()
  test1Sentence(bi, SENTENCES[0]);
}

void TestOpenNLPSentenceBreakIterator::testWhitespaceOnly() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(L"   \n \n\n\r\n\t  \n");
  test0Sentences(bi);
}

void TestOpenNLPSentenceBreakIterator::testEmptyString() 
{
  shared_ptr<NLPSentenceDetectorOp> sentenceDetectorOp =
      OpenNLPOpsFactory::getSentenceDetector(sentenceModelFile);
  shared_ptr<BreakIterator> bi =
      make_shared<OpenNLPSentenceBreakIterator>(sentenceDetectorOp);
  bi->setText(L"");
  test0Sentences(bi);
}

void TestOpenNLPSentenceBreakIterator::test0Sentences(
    shared_ptr<BreakIterator> bi)
{
  assertEquals(0, bi->current());
  assertEquals(0, bi->first());
  assertEquals(BreakIterator::DONE, bi->next());
  assertEquals(0, bi->last());
  assertEquals(BreakIterator::DONE, bi->previous());
  assertEquals(BreakIterator::DONE, bi->following(0));
  assertEquals(BreakIterator::DONE, bi->preceding(0));
  assertEquals(0, bi->first());
  assertEquals(BreakIterator::DONE, bi->next(13));
  assertEquals(BreakIterator::DONE, bi->next(-8));
}
} // namespace org::apache::lucene::analysis::opennlp