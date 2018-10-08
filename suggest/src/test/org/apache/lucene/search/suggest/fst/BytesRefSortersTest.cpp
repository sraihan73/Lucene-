using namespace std;

#include "BytesRefSortersTest.h"

namespace org::apache::lucene::search::suggest::fst
{
using InMemorySorter = org::apache::lucene::search::suggest::InMemorySorter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testExternalRefSorter() throws Exception
void BytesRefSortersTest::testExternalRefSorter() 
{
  shared_ptr<Directory> tempDir = newDirectory();
  shared_ptr<ExternalRefSorter> s = make_shared<ExternalRefSorter>(
      make_shared<OfflineSorter>(tempDir, L"temp"));
  check(s);
  IOUtils::close({s, tempDir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testInMemorySorter() throws Exception
void BytesRefSortersTest::testInMemorySorter() 
{
  check(make_shared<InMemorySorter>(Comparator::naturalOrder()));
}

void BytesRefSortersTest::check(shared_ptr<BytesRefSorter> sorter) throw(
    runtime_error)
{
  for (int i = 0; i < 100; i++) {
    std::deque<char> current(random()->nextInt(256));
    random()->nextBytes(current);
    sorter->add(make_shared<BytesRef>(current));
  }

  // Create two iterators and check that they're aligned with each other.
  shared_ptr<BytesRefIterator> i1 = sorter->begin();
  shared_ptr<BytesRefIterator> i2 = sorter->begin();

  // Verify sorter contract.
  expectThrows(IllegalStateException::typeid, [&]() {
    sorter->add(make_shared<BytesRef>(std::deque<char>(1)));
  });

  while (true) {
    shared_ptr<BytesRef> spare1 = i1->next();
    shared_ptr<BytesRef> spare2 = i2->next();
    assertEquals(spare1, spare2);
    if (spare1 == nullptr) {
      break;
    }
  }
}
} // namespace org::apache::lucene::search::suggest::fst