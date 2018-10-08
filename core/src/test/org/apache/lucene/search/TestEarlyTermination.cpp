using namespace std;

#include "TestEarlyTermination.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestEarlyTermination::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  writer = make_shared<RandomIndexWriter>(random(), dir);
  constexpr int numDocs = atLeast(100);
  for (int i = 0; i < numDocs; i++) {
    writer->addDocument(make_shared<Document>());
    if (rarely()) {
      writer->commit();
    }
  }
}

void TestEarlyTermination::tearDown() 
{
  LuceneTestCase::tearDown();
  delete writer;
  delete dir;
}

void TestEarlyTermination::testEarlyTermination() 
{
  constexpr int iters = atLeast(5);
  shared_ptr<IndexReader> *const reader = writer->getReader();

  for (int i = 0; i < iters; ++i) {
    shared_ptr<IndexSearcher> *const searcher = newSearcher(reader);
    shared_ptr<Collector> *const collector =
        make_shared<SimpleCollectorAnonymousInnerClass>(shared_from_this());

    searcher->search(make_shared<MatchAllDocsQuery>(), collector);
  }
  delete reader;
}

TestEarlyTermination::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<TestEarlyTermination> outerInstance)
{
  this->outerInstance = outerInstance;
  collectionTerminated = true;
}

void TestEarlyTermination::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  assertFalse(collectionTerminated);
  if (LuceneTestCase::rarely()) {
    collectionTerminated = true;
    throw make_shared<CollectionTerminatedException>();
  }
}

void TestEarlyTermination::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  if (LuceneTestCase::random()->nextBoolean()) {
    collectionTerminated = true;
    throw make_shared<CollectionTerminatedException>();
  } else {
    collectionTerminated = false;
  }
}

bool TestEarlyTermination::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}
} // namespace org::apache::lucene::search