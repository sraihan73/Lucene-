using namespace std;

#include "TestIndexManyDocuments.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestIndexManyDocuments::test() 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> iwc = make_shared<IndexWriterConfig>();
  iwc->setMaxBufferedDocs(TestUtil::nextInt(random(), 100, 2000));

  int numDocs = atLeast(10000);

  shared_ptr<IndexWriter> *const w = make_shared<IndexWriter>(dir, iwc);
  shared_ptr<AtomicInteger> *const count = make_shared<AtomicInteger>();
  std::deque<std::shared_ptr<Thread>> threads(2);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(shared_from_this(),
                                                        numDocs, w, count);
    threads[i]->start();
  }

  for (auto thread : threads) {
    thread->join();
  }

  assertEquals(L"lost " + to_wstring(numDocs - w->maxDoc()) +
                   L" documents; maxBufferedDocs=" +
                   to_wstring(iwc->getMaxBufferedDocs()),
               numDocs, w->maxDoc());
  delete w;

  shared_ptr<IndexReader> r = DirectoryReader::open(dir);
  TestUtil::assertEquals(numDocs, r->maxDoc());
  IOUtils::close({r, dir});
}

TestIndexManyDocuments::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestIndexManyDocuments> outerInstance, int numDocs,
    shared_ptr<org::apache::lucene::index::IndexWriter> w,
    shared_ptr<AtomicInteger> count)
{
  this->outerInstance = outerInstance;
  this->numDocs = numDocs;
  this->w = w;
  this->count = count;
}

void TestIndexManyDocuments::ThreadAnonymousInnerClass::run()
{
  while (count->getAndIncrement() < numDocs) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        LuceneTestCase::newTextField(L"field", L"text", Field::Store::NO));
    try {
      w->addDocument(doc);
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  }
}
} // namespace org::apache::lucene::index