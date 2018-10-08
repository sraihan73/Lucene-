using namespace std;

#include "TestForTooMuchCloning.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermRangeQuery = org::apache::lucene::search::TermRangeQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestForTooMuchCloning::test() 
{
  shared_ptr<MockDirectoryWrapper> *const dir = newMockDirectory();
  shared_ptr<TieredMergePolicy> *const tmp = make_shared<TieredMergePolicy>();
  tmp->setMaxMergeAtOnce(2);
  shared_ptr<RandomIndexWriter> *const w = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMaxBufferedDocs(2)
          ->setMergePolicy(tmp));
  constexpr int numDocs = 20;
  for (int docs = 0; docs < numDocs; docs++) {
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    for (int terms = 0; terms < 100; terms++) {
      sb->append(TestUtil::randomRealisticUnicodeString(random()));
      sb->append(L' ');
    }
    shared_ptr<Document> *const doc = make_shared<Document>();
    doc->push_back(
        make_shared<TextField>(L"field", sb->toString(), Field::Store::NO));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  // System.out.println("merge clone count=" + cloneCount);
  assertTrue(L"too many calls to IndexInput.clone during merging: " +
                 to_wstring(dir->getInputCloneCount()),
             dir->getInputCloneCount() < 500);

  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  // important: set this after newSearcher, it might have run checkindex
  constexpr int cloneCount = dir->getInputCloneCount();
  // dir.setVerboseClone(true);

  // MTQ that matches all terms so the AUTO_REWRITE should
  // cutover to filter rewrite and reuse a single DocsEnum
  // across all terms;
  shared_ptr<TopDocs> *const hits = s->search(
      make_shared<TermRangeQuery>(L"field", make_shared<BytesRef>(),
                                  make_shared<BytesRef>(L"\uFFFF"), true, true),
      10);
  assertTrue(hits->totalHits > 0);
  constexpr int queryCloneCount = dir->getInputCloneCount() - cloneCount;
  // System.out.println("query clone count=" + queryCloneCount);
  assertTrue(L"too many calls to IndexInput.clone during TermRangeQuery: " +
                 to_wstring(queryCloneCount),
             queryCloneCount < 50);
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index