using namespace std;

#include "TestSplittingBreakIterator.h"

namespace org::apache::lucene::search::uhighlight
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const shared_ptr<java::text::BreakIterator>
    TestSplittingBreakIterator::LINE_BI =
        java::text::BreakIterator::getLineInstance(java::util::Locale::ROOT);
const shared_ptr<java::text::BreakIterator>
    TestSplittingBreakIterator::SPLIT_BI =
        make_shared<SplittingBreakIterator>(LINE_BI, L'|');

void TestSplittingBreakIterator::testLineBreakIterator()
{
  testWithoutSplits(LINE_BI);
}

void TestSplittingBreakIterator::testWithoutSplits(shared_ptr<BreakIterator> bi)
{
  // these tests have no '|'
  testBreakIterator(bi, L" a", L"^^^");
  testBreakIterator(bi, L"aa", L"^ ^");
  testBreakIterator(bi, L"aa a", L"^  ^^");
}

void TestSplittingBreakIterator::testWithoutSplits()
{
  testWithoutSplits(SPLIT_BI);
}

void TestSplittingBreakIterator::testOnlySingleSplitChar()
{
  testBreakIterator(SPLIT_BI, L"|", L"^^");
}

void TestSplittingBreakIterator::testSplitThenValue()
{
  testBreakIterator(SPLIT_BI, L"|a", L"^^^");
}

void TestSplittingBreakIterator::testValueThenSplit()
{
  testBreakIterator(SPLIT_BI, L"a|", L"^^^");
}

void TestSplittingBreakIterator::testValueThenSplitThenValue()
{
  testBreakIterator(SPLIT_BI, L"aa|aa", L"^ ^^ ^");
}

void TestSplittingBreakIterator::testValueThenDoubleSplitThenValue()
{
  testBreakIterator(SPLIT_BI, L"aa||aa", L"^ ^^^ ^");
}

void TestSplittingBreakIterator::
    testValueThenSplitThenDoubleValueThenSplitThenValue()
{
  testBreakIterator(SPLIT_BI, L"a|bb cc|d", L"^^^  ^ ^^^");
}

void TestSplittingBreakIterator::testBreakIterator(shared_ptr<BreakIterator> bi,
                                                   const wstring &text,
                                                   const wstring &boundaries)
{
  bi->setText(text);

  // Test first & last
  testFirstAndLast(bi, text, boundaries);

  // Test if expected boundaries are consistent with reading them from next() in
  // a loop:
  assertEquals(boundaries, readBoundariesToString(bi, text));

  // Test following() and preceding():
  // get each index, randomized in case their is a sequencing bug:
  deque<int> indexes = randomIntsBetweenInclusive(text.length() + 1);
  testFollowing(bi, text, boundaries, indexes);
  testPreceding(bi, text, boundaries, indexes);

  // Test previous():
  testPrevious(bi, text, boundaries);
}

void TestSplittingBreakIterator::testFirstAndLast(shared_ptr<BreakIterator> bi,
                                                  const wstring &text,
                                                  const wstring &boundaries)
{
  wstring message = L"Text: " + text;
  int current = bi->current();
  assertEquals(message, (int)boundaries.find(L'^'), current);
  assertEquals(message, current, bi->first());
  assertEquals(message, current, bi->current());
  current = bi->last();
  assertEquals((int)boundaries.rfind(L'^'), current);
  assertEquals(message, current, bi->current());
}

void TestSplittingBreakIterator::testFollowing(shared_ptr<BreakIterator> bi,
                                               const wstring &text,
                                               const wstring &boundaries,
                                               deque<int> &indexes)
{
  wstring message = L"Text: " + text;
  for (shared_ptr<> : : optional<int> index : indexes) {
    int got = bi->following(index);
    if (index == boundaries.length()) {
      assertEquals(message, BreakIterator::DONE, got);
      assertEquals((int)boundaries.rfind(L'^'), bi->current());
      continue;
    }
    assertEquals(message + L" index:" + index,
                 (int)boundaries.find(L'^', index + 1), got);
  }
}

void TestSplittingBreakIterator::testPreceding(shared_ptr<BreakIterator> bi,
                                               const wstring &text,
                                               const wstring &boundaries,
                                               deque<int> &indexes)
{
  wstring message = L"Text: " + text;
  for (shared_ptr<> : : optional<int> index : indexes) {
    int got = bi->preceding(index);
    if (index == 0) {
      assertEquals(message, BreakIterator::DONE, got);
      assertEquals((int)boundaries.find(L'^'), bi->current());
      continue;
    }
    //            if (index == text.length() && got == BreakIterator.DONE) {
    //                continue;//hack to accept faulty default impl of
    //                BreakIterator.preceding()
    //            }
    assertEquals(message + L" index:" + index,
                 (int)boundaries.rfind(L'^', index - 1), got);
  }
}

deque<int> TestSplittingBreakIterator::randomIntsBetweenInclusive(int end)
{
  deque<int> indexes = deque<int>(end);
  for (int i = 0; i < end; i++) {
    indexes.push_back(i);
  }
  Collections::shuffle(indexes, random());
  return indexes;
}

void TestSplittingBreakIterator::testPrevious(shared_ptr<BreakIterator> bi,
                                              const wstring &text,
                                              const wstring &boundaries)
{
  wstring message = L"Text: " + text;

  bi->setText(text);
  int idx = bi->last(); // position at the end
  while (true) {
    idx = (int)boundaries.rfind(L'^', idx - 1);
    if (idx == -1) {
      assertEquals(message, BreakIterator::DONE, bi->previous());
      break;
    }
    assertEquals(message, idx, bi->previous());
  }
  assertEquals(message, (int)boundaries.find(L'^'),
               bi->current()); // finishes at first
}

wstring
TestSplittingBreakIterator::readBoundariesToString(shared_ptr<BreakIterator> bi,
                                                   const wstring &text)
{
  // init markers to spaces
  shared_ptr<StringBuilder> markers = make_shared<StringBuilder>();
  markers->setLength(text.length() + 1);
  for (int k = 0; k < markers->length(); k++) {
    markers->setCharAt(k, L' ');
  }

  bi->setText(text);
  for (int boundary = bi->current(); boundary != BreakIterator::DONE;
       boundary = bi->next()) {
    markers->setCharAt(boundary, L'^');
  }
  return markers->toString();
}
} // namespace org::apache::lucene::search::uhighlight