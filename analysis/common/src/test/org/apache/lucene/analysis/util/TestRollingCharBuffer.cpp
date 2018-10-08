using namespace std;

#include "TestRollingCharBuffer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/RollingCharBuffer.h"

namespace org::apache::lucene::analysis::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestRollingCharBuffer::test() 
{
  constexpr int ITERS = atLeast(1000);

  shared_ptr<RollingCharBuffer> buffer = make_shared<RollingCharBuffer>();

  shared_ptr<Random> random = TestRollingCharBuffer::random();
  for (int iter = 0; iter < ITERS; iter++) {
    constexpr int stringLen =
        random->nextBoolean() ? random->nextInt(50) : random->nextInt(20000);
    const wstring s;
    if (stringLen == 0) {
      s = L"";
    } else {
      s = TestUtil::randomUnicodeString(random, stringLen);
    }
    if (VERBOSE) {
      wcout << L"\nTEST: iter=" << iter << L" s.length()=" << s.length()
            << endl;
    }
    buffer->reset(make_shared<StringReader>(s));
    int nextRead = 0;
    int availCount = 0;
    while (nextRead < s.length()) {
      if (VERBOSE) {
        wcout << L"  cycle nextRead=" << nextRead << L" avail=" << availCount
              << endl;
      }
      if (availCount == 0 || random->nextBoolean()) {
        // Read next char
        if (VERBOSE) {
          wcout << L"    new char" << endl;
        }
        TestUtil::assertEquals(s[nextRead], buffer->get(nextRead));
        nextRead++;
        availCount++;
      } else if (random->nextBoolean()) {
        // Read previous char
        int pos =
            TestUtil::nextInt(random, nextRead - availCount, nextRead - 1);
        if (VERBOSE) {
          wcout << L"    old char pos=" << pos << endl;
        }
        TestUtil::assertEquals(s[pos], buffer->get(pos));
      } else {
        // Read slice
        int length;
        if (availCount == 1) {
          length = 1;
        } else {
          length = TestUtil::nextInt(random, 1, availCount);
        }
        int start;
        if (length == availCount) {
          start = nextRead - availCount;
        } else {
          start = nextRead - availCount + random->nextInt(availCount - length);
        }
        if (VERBOSE) {
          wcout << L"    slice start=" << start << L" length=" << length
                << endl;
        }
        TestUtil::assertEquals(s.substr(start, length),
                               wstring(buffer->get(start, length)));
      }

      if (availCount > 0 && random->nextInt(20) == 17) {
        constexpr int toFree = random->nextInt(availCount);
        if (VERBOSE) {
          wcout << L"    free " << toFree << L" (avail="
                << (availCount - toFree) << L")" << endl;
        }
        buffer->freeBefore(nextRead - (availCount - toFree));
        availCount -= toFree;
      }
    }
  }
}
} // namespace org::apache::lucene::analysis::util