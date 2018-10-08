using namespace std;

#include "TestCharArrayIterator.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharArrayIterator.h"

namespace org::apache::lucene::analysis::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestCharArrayIterator::testWordInstance()
{
  doTests(CharArrayIterator::newWordInstance());
}

void TestCharArrayIterator::testConsumeWordInstance()
{
  // we use the default locale, as it's randomized by LuceneTestCase
  shared_ptr<BreakIterator> bi =
      BreakIterator::getWordInstance(Locale::getDefault());
  shared_ptr<CharArrayIterator> ci = CharArrayIterator::newWordInstance();
  for (int i = 0; i < 10000; i++) {
    std::deque<wchar_t> text =
        TestUtil::randomUnicodeString(random()).toCharArray();
    ci->setText(text, 0, text.size());
    consume(bi, ci);
  }
}

void TestCharArrayIterator::testSentenceInstance()
{
  doTests(CharArrayIterator::newSentenceInstance());
}

void TestCharArrayIterator::testConsumeSentenceInstance()
{
  // we use the default locale, as it's randomized by LuceneTestCase
  shared_ptr<BreakIterator> bi =
      BreakIterator::getSentenceInstance(Locale::getDefault());
  shared_ptr<CharArrayIterator> ci = CharArrayIterator::newSentenceInstance();
  for (int i = 0; i < 10000; i++) {
    std::deque<wchar_t> text =
        TestUtil::randomUnicodeString(random()).toCharArray();
    ci->setText(text, 0, text.size());
    consume(bi, ci);
  }
}

void TestCharArrayIterator::doTests(shared_ptr<CharArrayIterator> ci)
{
  // basics
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  TestUtil::assertEquals(0, ci->getBeginIndex());
  TestUtil::assertEquals(7, ci->getEndIndex());
  TestUtil::assertEquals(0, ci->getIndex());
  TestUtil::assertEquals(L't', ci->current());
  TestUtil::assertEquals(L'e', ci->next());
  TestUtil::assertEquals(L'g', ci->last());
  TestUtil::assertEquals(L'n', ci->previous());
  TestUtil::assertEquals(L't', ci->first());
  TestUtil::assertEquals(CharacterIterator::DONE, ci->previous());

  // first()
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  ci->next();
  // Sets the position to getBeginIndex() and returns the character at that
  // position.
  TestUtil::assertEquals(L't', ci->first());
  TestUtil::assertEquals(ci->getBeginIndex(), ci->getIndex());
  // or DONE if the text is empty
  ci->setText(std::deque<wchar_t>(), 0, 0);
  TestUtil::assertEquals(CharacterIterator::DONE, ci->first());

  // last()
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  // Sets the position to getEndIndex()-1 (getEndIndex() if the text is empty)
  // and returns the character at that position.
  TestUtil::assertEquals(L'g', ci->last());
  TestUtil::assertEquals(ci->getIndex(), ci->getEndIndex() - 1);
  // or DONE if the text is empty
  ci->setText(std::deque<wchar_t>(), 0, 0);
  TestUtil::assertEquals(CharacterIterator::DONE, ci->last());
  TestUtil::assertEquals(ci->getEndIndex(), ci->getIndex());

  // current()
  // Gets the character at the current position (as returned by getIndex()).
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  TestUtil::assertEquals(L't', ci->current());
  ci->last();
  ci->next();
  // or DONE if the current position is off the end of the text.
  TestUtil::assertEquals(CharacterIterator::DONE, ci->current());

  // next()
  ci->setText((wstring(L"te")).toCharArray(), 0, 2);
  // Increments the iterator's index by one and returns the character at the new
  // index.
  TestUtil::assertEquals(L'e', ci->next());
  TestUtil::assertEquals(1, ci->getIndex());
  // or DONE if the new position is off the end of the text range.
  TestUtil::assertEquals(CharacterIterator::DONE, ci->next());
  TestUtil::assertEquals(ci->getEndIndex(), ci->getIndex());

  // setIndex()
  ci->setText((wstring(L"test")).toCharArray(), 0, (wstring(L"test")).length());
  expectThrows(invalid_argument::typeid, [&]() { ci->setIndex(5); });

  // clone()
  std::deque<wchar_t> text = (wstring(L"testing")).toCharArray();
  ci->setText(text, 0, text.size());
  ci->next();
  shared_ptr<CharArrayIterator> ci2 = ci->clone();
  TestUtil::assertEquals(ci->getIndex(), ci2->getIndex());
  TestUtil::assertEquals(ci->next(), ci2->next());
  TestUtil::assertEquals(ci->last(), ci2->last());
}

void TestCharArrayIterator::consume(shared_ptr<BreakIterator> bi,
                                    shared_ptr<CharacterIterator> ci)
{
  bi->setText(ci);
  while (bi->next() != BreakIterator::DONE) {
    ;
  }
}
} // namespace org::apache::lucene::analysis::util