using namespace std;

#include "TestWholeBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestWholeBreakIterator::testSingleSentences() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual = make_shared<WholeBreakIterator>();
  assertSameBreaks(L"a", expected, actual);
  assertSameBreaks(L"ab", expected, actual);
  assertSameBreaks(L"abc", expected, actual);
  assertSameBreaks(L"", expected, actual);
}

void TestWholeBreakIterator::testSliceEnd() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual = make_shared<WholeBreakIterator>();
  assertSameBreaks(L"a000", 0, 1, expected, actual);
  assertSameBreaks(L"ab000", 0, 1, expected, actual);
  assertSameBreaks(L"abc000", 0, 1, expected, actual);
  assertSameBreaks(L"000", 0, 0, expected, actual);
}

void TestWholeBreakIterator::testSliceStart() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual = make_shared<WholeBreakIterator>();
  assertSameBreaks(L"000a", 3, 1, expected, actual);
  assertSameBreaks(L"000ab", 3, 2, expected, actual);
  assertSameBreaks(L"000abc", 3, 3, expected, actual);
  assertSameBreaks(L"000", 3, 0, expected, actual);
}

void TestWholeBreakIterator::testSliceMiddle() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual = make_shared<WholeBreakIterator>();
  assertSameBreaks(L"000a000", 3, 1, expected, actual);
  assertSameBreaks(L"000ab000", 3, 2, expected, actual);
  assertSameBreaks(L"000abc000", 3, 3, expected, actual);
  assertSameBreaks(L"000000", 3, 0, expected, actual);
}

void TestWholeBreakIterator::testFirstPosition() 
{
  shared_ptr<BreakIterator> expected =
      BreakIterator::getSentenceInstance(Locale::ROOT);
  shared_ptr<BreakIterator> actual = make_shared<WholeBreakIterator>();
  assertSameBreaks(L"000ab000", 3, 2, 4, expected, actual);
}

void TestWholeBreakIterator::assertSameBreaks(
    const wstring &text, shared_ptr<BreakIterator> expected,
    shared_ptr<BreakIterator> actual)
{
  assertSameBreaks(make_shared<StringCharacterIterator>(text),
                   make_shared<StringCharacterIterator>(text), expected,
                   actual);
}

void TestWholeBreakIterator::assertSameBreaks(
    const wstring &text, int offset, int length,
    shared_ptr<BreakIterator> expected, shared_ptr<BreakIterator> actual)
{
  assertSameBreaks(text, offset, length, offset, expected, actual);
}

void TestWholeBreakIterator::assertSameBreaks(
    const wstring &text, int offset, int length, int current,
    shared_ptr<BreakIterator> expected, shared_ptr<BreakIterator> actual)
{
  assertSameBreaks(make_shared<StringCharacterIterator>(
                       text, offset, offset + length, current),
                   make_shared<StringCharacterIterator>(
                       text, offset, offset + length, current),
                   expected, actual);
}

void TestWholeBreakIterator::assertSameBreaks(
    shared_ptr<CharacterIterator> one, shared_ptr<CharacterIterator> two,
    shared_ptr<BreakIterator> expected, shared_ptr<BreakIterator> actual)
{
  expected->setText(one);
  actual->setText(two);

  assertEquals(expected->current(), actual->current());

  // next()
  int v = expected->current();
  while (v != BreakIterator::DONE) {
    assertEquals(v = expected->next(), actual->next());
    assertEquals(expected->current(), actual->current());
  }

  // first()
  assertEquals(expected->first(), actual->first());
  assertEquals(expected->current(), actual->current());
  // last()
  assertEquals(expected->last(), actual->last());
  assertEquals(expected->current(), actual->current());

  // previous()
  v = expected->current();
  while (v != BreakIterator::DONE) {
    assertEquals(v = expected->previous(), actual->previous());
    assertEquals(expected->current(), actual->current());
  }

  // following()
  for (int i = one->getBeginIndex(); i <= one->getEndIndex(); i++) {
    expected->first();
    actual->first();
    assertEquals(expected->following(i), actual->following(i));
    assertEquals(expected->current(), actual->current());
  }

  // preceding()
  for (int i = one->getBeginIndex(); i <= one->getEndIndex(); i++) {
    expected->last();
    actual->last();
    assertEquals(expected->preceding(i), actual->preceding(i));
    assertEquals(expected->current(), actual->current());
  }
}
} // namespace org::apache::lucene::search::uhighlight