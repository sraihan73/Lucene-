using namespace std;

#include "TestCharArrayIterator.h"
#include "../../../../../../../java/org/apache/lucene/analysis/icu/segmentation/CharArrayIterator.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCharArrayIterator::testBasicUsage()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  assertEquals(0, ci->getBeginIndex());
  assertEquals(7, ci->getEndIndex());
  assertEquals(0, ci->getIndex());
  assertEquals(L't', ci->current());
  assertEquals(L'e', ci->next());
  assertEquals(L'g', ci->last());
  assertEquals(L'n', ci->previous());
  assertEquals(L't', ci->first());
  assertEquals(CharacterIterator::DONE, ci->previous());
}

void TestCharArrayIterator::testFirst()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  ci->next();
  // Sets the position to getBeginIndex() and returns the character at that
  // position.
  assertEquals(L't', ci->first());
  assertEquals(ci->getBeginIndex(), ci->getIndex());
  // or DONE if the text is empty
  ci->setText(std::deque<wchar_t>(), 0, 0);
  assertEquals(CharacterIterator::DONE, ci->first());
}

void TestCharArrayIterator::testLast()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  // Sets the position to getEndIndex()-1 (getEndIndex() if the text is empty)
  // and returns the character at that position.
  assertEquals(L'g', ci->last());
  assertEquals(ci->getIndex(), ci->getEndIndex() - 1);
  // or DONE if the text is empty
  ci->setText(std::deque<wchar_t>(), 0, 0);
  assertEquals(CharacterIterator::DONE, ci->last());
  assertEquals(ci->getEndIndex(), ci->getIndex());
}

void TestCharArrayIterator::testCurrent()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  // Gets the character at the current position (as returned by getIndex()).
  ci->setText((wstring(L"testing")).toCharArray(), 0,
              (wstring(L"testing")).length());
  assertEquals(L't', ci->current());
  ci->last();
  ci->next();
  // or DONE if the current position is off the end of the text.
  assertEquals(CharacterIterator::DONE, ci->current());
}

void TestCharArrayIterator::testNext()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText((wstring(L"te")).toCharArray(), 0, 2);
  // Increments the iterator's index by one and returns the character at the new
  // index.
  assertEquals(L'e', ci->next());
  assertEquals(1, ci->getIndex());
  // or DONE if the new position is off the end of the text range.
  assertEquals(CharacterIterator::DONE, ci->next());
  assertEquals(ci->getEndIndex(), ci->getIndex());
}

void TestCharArrayIterator::testSetIndex()
{
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText((wstring(L"test")).toCharArray(), 0, (wstring(L"test")).length());
  expectThrows(invalid_argument::typeid, [&]() { ci->setIndex(5); });
}

void TestCharArrayIterator::testClone()
{
  std::deque<wchar_t> text = (wstring(L"testing")).toCharArray();
  shared_ptr<CharArrayIterator> ci = make_shared<CharArrayIterator>();
  ci->setText(text, 0, text.size());
  ci->next();
  shared_ptr<CharArrayIterator> ci2 = ci->clone();
  assertEquals(ci->getIndex(), ci2->getIndex());
  assertEquals(ci->next(), ci2->next());
  assertEquals(ci->last(), ci2->last());
}
} // namespace org::apache::lucene::analysis::icu::segmentation