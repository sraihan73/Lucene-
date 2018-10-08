using namespace std;

#include "BreakIteratorBoundaryScannerTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring BreakIteratorBoundaryScannerTest::TEXT =
    wstring(L"Apache Lucene(TM) is a high-performance, full-featured text "
            L"search engine library written entirely in Java.") +
    L"\nIt is a technology suitable for nearly any application that "
    L"requires\n" +
    L"full-text search, especially cross-platform. \nApache Lucene is an open "
    L"source project available for free download.";

void BreakIteratorBoundaryScannerTest::testOutOfRange() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  shared_ptr<BreakIterator> bi = BreakIterator::getWordInstance(Locale::ROOT);
  shared_ptr<BoundaryScanner> scanner =
      make_shared<BreakIteratorBoundaryScanner>(bi);

  int start = TEXT.length() + 1;
  assertEquals(start, scanner->findStartOffset(text, start));
  assertEquals(start, scanner->findEndOffset(text, start));
  start = 0;
  assertEquals(start, scanner->findStartOffset(text, start));
  start = -1;
  assertEquals(start, scanner->findEndOffset(text, start));
}

void BreakIteratorBoundaryScannerTest::testWordBoundary() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  shared_ptr<BreakIterator> bi = BreakIterator::getWordInstance(Locale::ROOT);
  shared_ptr<BoundaryScanner> scanner =
      make_shared<BreakIteratorBoundaryScanner>(bi);

  int start = (int)TEXT.find(L"formance");
  int expected = (int)TEXT.find(L"high-performance");
  testFindStartOffset(text, start, expected, scanner);

  expected = (int)TEXT.find(L", full");
  testFindEndOffset(text, start, expected, scanner);
}

void BreakIteratorBoundaryScannerTest::testSentenceBoundary() throw(
    runtime_error)
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  // we test this with default locale, it's randomized by LuceneTestCase
  shared_ptr<BreakIterator> bi =
      BreakIterator::getSentenceInstance(Locale::getDefault());
  shared_ptr<BoundaryScanner> scanner =
      make_shared<BreakIteratorBoundaryScanner>(bi);

  int start = (int)TEXT.find(L"any application");
  int expected = (int)TEXT.find(L"It is a");
  testFindStartOffset(text, start, expected, scanner);

  expected = (int)TEXT.find(L"Apache Lucene is an open source");
  testFindEndOffset(text, start, expected, scanner);
}

void BreakIteratorBoundaryScannerTest::testLineBoundary() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  // we test this with default locale, it's randomized by LuceneTestCase
  shared_ptr<BreakIterator> bi =
      BreakIterator::getLineInstance(Locale::getDefault());
  shared_ptr<BoundaryScanner> scanner =
      make_shared<BreakIteratorBoundaryScanner>(bi);

  int start = (int)TEXT.find(L"any application");
  int expected = (int)TEXT.find(L"nearly");
  testFindStartOffset(text, start, expected, scanner);

  expected = (int)TEXT.find(L"application that requires");
  testFindEndOffset(text, start, expected, scanner);
}

void BreakIteratorBoundaryScannerTest::testFindStartOffset(
    shared_ptr<StringBuilder> text, int start, int expected,
    shared_ptr<BoundaryScanner> scanner) 
{
  assertEquals(expected, scanner->findStartOffset(text, start));
}

void BreakIteratorBoundaryScannerTest::testFindEndOffset(
    shared_ptr<StringBuilder> text, int start, int expected,
    shared_ptr<BoundaryScanner> scanner) 
{
  assertEquals(expected, scanner->findEndOffset(text, start));
}
} // namespace org::apache::lucene::search::vectorhighlight