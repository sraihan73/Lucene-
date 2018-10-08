using namespace std;

#include "Test2GBCharBlockArray.h"

namespace org::apache::lucene::facet::taxonomy::writercache
{
using org::apache::lucene::util::LuceneTestCase::Monster;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void Test2GBCharBlockArray::test2GBChars() 
{
  int blockSize = 32768;
  shared_ptr<CharBlockArray> array_ = make_shared<CharBlockArray>(blockSize);

  int size = TestUtil::nextInt(random(), 20000, 40000);

  std::deque<wchar_t> chars(size);
  int count = 0;
  while (true) {
    count++;
    try {
      array_->append(chars, 0, size);
    } catch (const IllegalStateException &ise) {
      assertTrue(count * static_cast<int64_t>(size) + blockSize >
                 numeric_limits<int>::max());
      break;
    }
    assertFalse(L"appended " +
                    (count * static_cast<int64_t>(size) -
                     numeric_limits<int>::max()) +
                    L" characters beyond Integer.MAX_VALUE!",
                count * static_cast<int64_t>(size) >
                    numeric_limits<int>::max());
  }
}
} // namespace org::apache::lucene::facet::taxonomy::writercache