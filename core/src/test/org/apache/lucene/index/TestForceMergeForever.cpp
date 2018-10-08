using namespace std;

#include "TestForceMergeForever.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Directory = org::apache::lucene::store::Directory;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TestForceMergeForever::MyIndexWriter::MyIndexWriter(
    shared_ptr<Directory> dir,
    shared_ptr<IndexWriterConfig> conf) 
    : IndexWriter(dir, conf)
{
}

void TestForceMergeForever::MyIndexWriter::merge(
    shared_ptr<MergePolicy::OneMerge> merge) 
{
  if (merge->maxNumSegments != -1 && (first || merge->segments.size() == 1)) {
    first = false;
    if (VERBOSE) {
      wcout << L"TEST: maxNumSegments merge" << endl;
    }
    mergeCount->incrementAndGet();
  }
  IndexWriter::merge(merge);
}

void TestForceMergeForever::test() 
{
  shared_ptr<Directory> *const d = newDirectory();
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  // SMS can cause this test to run indefinitely long:
  iwc->setMergeScheduler(make_shared<ConcurrentMergeScheduler>());

  shared_ptr<MyIndexWriter> *const w = make_shared<MyIndexWriter>(d, iwc);

  // Try to make an index that requires merging:
  w->getConfig()->setMaxBufferedDocs(TestUtil::nextInt(random(), 2, 11));
  constexpr int numStartDocs = atLeast(20);
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random());
  for (int docIDX = 0; docIDX < numStartDocs; docIDX++) {
    w->addDocument(docs->nextDoc());
  }
  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  constexpr int mergeAtOnce = 1 + w->segmentInfos->size();
  if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
    (std::static_pointer_cast<TieredMergePolicy>(mp))
        ->setMaxMergeAtOnce(mergeAtOnce);
  } else if (std::dynamic_pointer_cast<LogMergePolicy>(mp) != nullptr) {
    (std::static_pointer_cast<LogMergePolicy>(mp))->setMergeFactor(mergeAtOnce);
  } else {
    // skip test
    delete w;
    delete d;
    return;
  }

  shared_ptr<AtomicBoolean> *const doStop = make_shared<AtomicBoolean>();
  w->getConfig()->setMaxBufferedDocs(2);
  shared_ptr<Thread> t = make_shared<ThreadAnonymousInnerClass>(
      shared_from_this(), w, numStartDocs, docs, doStop);
  t->start();
  w->forceMerge(1);
  doStop->set(true);
  t->join();
  assertTrue(L"merge count is " + w->mergeCount->get(),
             w->mergeCount->get() <= 1);
  delete w;
  delete d;
  delete docs;
}

TestForceMergeForever::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestForceMergeForever> outerInstance,
    shared_ptr<org::apache::lucene::index::TestForceMergeForever::MyIndexWriter>
        w,
    int numStartDocs, shared_ptr<LineFileDocs> docs,
    shared_ptr<AtomicBoolean> doStop)
{
  this->outerInstance = outerInstance;
  this->w = w;
  this->numStartDocs = numStartDocs;
  this->docs = docs;
  this->doStop = doStop;
}

void TestForceMergeForever::ThreadAnonymousInnerClass::run()
{
  try {
    while (doStop->get() == false) {
      w->updateDocument(
          make_shared<Term>(
              L"docid", L"" + LuceneTestCase::random()->nextInt(numStartDocs)),
          docs->nextDoc());
      // Force deletes to apply
      w->getReader()->close();
    }
  } catch (const runtime_error &t) {
    throw runtime_error(t);
  }
}
} // namespace org::apache::lucene::index