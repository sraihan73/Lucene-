using namespace std;

#include "TestDocsWithFieldSet.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestDocsWithFieldSet::testDense() 
{
  shared_ptr<DocsWithFieldSet> set = make_shared<DocsWithFieldSet>();
  shared_ptr<DocIdSetIterator> it = set->begin();
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());

  set->add(0);
  it = set->begin();
  TestUtil::assertEquals(0, it->nextDoc());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());

  int64_t ramBytesUsed = set->ramBytesUsed();
  for (int i = 1; i < 1000; ++i) {
    set->add(i);
  }
  TestUtil::assertEquals(ramBytesUsed, set->ramBytesUsed());
  it = set->begin();
  for (int i = 0; i < 1000; ++i) {
    TestUtil::assertEquals(i, it->nextDoc());
  }
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());
}

void TestDocsWithFieldSet::testSparse() 
{
  shared_ptr<DocsWithFieldSet> set = make_shared<DocsWithFieldSet>();
  int doc = random()->nextInt(10000);
  set->add(doc);
  shared_ptr<DocIdSetIterator> it = set->begin();
  TestUtil::assertEquals(doc, it->nextDoc());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());
  int doc2 = doc + TestUtil::nextInt(random(), 1, 100);
  set->add(doc2);
  it = set->begin();
  TestUtil::assertEquals(doc, it->nextDoc());
  TestUtil::assertEquals(doc2, it->nextDoc());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());
}

void TestDocsWithFieldSet::testDenseThenSparse() 
{
  int denseCount = random()->nextInt(10000);
  int nextDoc = denseCount + random()->nextInt(10000);
  shared_ptr<DocsWithFieldSet> set = make_shared<DocsWithFieldSet>();
  for (int i = 0; i < denseCount; ++i) {
    set->add(i);
  }
  set->add(nextDoc);
  shared_ptr<DocIdSetIterator> it = set->begin();
  for (int i = 0; i < denseCount; ++i) {
    TestUtil::assertEquals(i, it->nextDoc());
  }
  TestUtil::assertEquals(nextDoc, it->nextDoc());
  TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, it->nextDoc());
}
} // namespace org::apache::lucene::index