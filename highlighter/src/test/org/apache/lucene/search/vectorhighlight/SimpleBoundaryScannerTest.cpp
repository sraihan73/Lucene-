using namespace std;

#include "SimpleBoundaryScannerTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring SimpleBoundaryScannerTest::TEXT =
    L"Apache Lucene(TM) is a high-performance, full-featured\ntext search "
    L"engine library written entirely in Java.";

void SimpleBoundaryScannerTest::testFindStartOffset() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  shared_ptr<BoundaryScanner> scanner = make_shared<SimpleBoundaryScanner>();

  // test out of range
  int start = TEXT.length() + 1;
  assertEquals(start, scanner->findStartOffset(text, start));
  start = 0;
  assertEquals(start, scanner->findStartOffset(text, start));

  start = (int)TEXT.find(L"formance");
  int expected = (int)TEXT.find(L"high-performance");
  assertEquals(expected, scanner->findStartOffset(text, start));

  start = (int)TEXT.find(L"che");
  expected = (int)TEXT.find(L"Apache");
  assertEquals(expected, scanner->findStartOffset(text, start));
}

void SimpleBoundaryScannerTest::testFindEndOffset() 
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>(TEXT);
  shared_ptr<BoundaryScanner> scanner = make_shared<SimpleBoundaryScanner>();

  // test out of range
  int start = TEXT.length() + 1;
  assertEquals(start, scanner->findEndOffset(text, start));
  start = -1;
  assertEquals(start, scanner->findEndOffset(text, start));

  start = (int)TEXT.find(L"full-");
  int expected = (int)TEXT.find(L"\ntext");
  assertEquals(expected, scanner->findEndOffset(text, start));
}
} // namespace org::apache::lucene::search::vectorhighlight