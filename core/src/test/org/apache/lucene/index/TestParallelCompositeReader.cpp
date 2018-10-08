using namespace std;

#include "TestParallelCompositeReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestParallelCompositeReader::testQueries() 
{
  single_ = single(random(), false);
  parallel_ = parallel(random(), false);

  queries();

  delete single_->getIndexReader();
  single_.reset();
  delete parallel_->getIndexReader();
  parallel_.reset();
  delete dir;
  dir.reset();
  delete dir1;
  dir1.reset();
  delete dir2;
  dir2.reset();
}

void TestParallelCompositeReader::testQueriesCompositeComposite() throw(
    runtime_error)
{
  single_ = single(random(), true);
  parallel_ = parallel(random(), true);

  queries();

  delete single_->getIndexReader();
  single_.reset();
  delete parallel_->getIndexReader();
  parallel_.reset();
  delete dir;
  dir.reset();
  delete dir1;
  dir1.reset();
  delete dir2;
  dir2.reset();
}

void TestParallelCompositeReader::queries() 
{
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f1", L"v1")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f1", L"v2")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f2", L"v1")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f2", L"v2")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f3", L"v1")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f3", L"v2")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f4", L"v1")));
  queryTest(make_shared<TermQuery>(make_shared<Term>(L"f4", L"v2")));

  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(make_shared<TermQuery>(make_shared<Term>(L"f1", L"v1")),
           Occur::MUST);
  bq1->add(make_shared<TermQuery>(make_shared<Term>(L"f4", L"v1")),
           Occur::MUST);
  queryTest(bq1->build());
}

void TestParallelCompositeReader::testRefCounts1() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<DirectoryReader> ir1, ir2;
  // close subreaders, ParallelReader will not change refCounts, but close on
  // its own close
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      ir1 = DirectoryReader::open(dir1), ir2 = DirectoryReader::open(dir2));
  shared_ptr<IndexReader> psub1 = pr->getSequentialSubReaders()[0];
  // check RefCounts
  assertEquals(1, ir1->getRefCount());
  assertEquals(1, ir2->getRefCount());
  assertEquals(1, psub1->getRefCount());
  pr->close();
  assertEquals(0, ir1->getRefCount());
  assertEquals(0, ir2->getRefCount());
  assertEquals(0, psub1->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelCompositeReader::testRefCounts2() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<DirectoryReader> ir1 = DirectoryReader::open(dir1);
  shared_ptr<DirectoryReader> ir2 = DirectoryReader::open(dir2);

  // don't close subreaders, so ParallelReader will increment refcounts
  shared_ptr<ParallelCompositeReader> pr =
      make_shared<ParallelCompositeReader>(false, ir1, ir2);
  shared_ptr<IndexReader> psub1 = pr->getSequentialSubReaders()[0];
  // check RefCounts
  assertEquals(2, ir1->getRefCount());
  assertEquals(2, ir2->getRefCount());
  assertEquals(L"refCount must be 1, as the synthetic reader was created by "
               L"ParallelCompositeReader",
               1, psub1->getRefCount());
  pr->close();
  assertEquals(1, ir1->getRefCount());
  assertEquals(1, ir2->getRefCount());
  assertEquals(L"refcount must be 0 because parent was closed", 0,
               psub1->getRefCount());
  ir1->close();
  ir2->close();
  assertEquals(0, ir1->getRefCount());
  assertEquals(0, ir2->getRefCount());
  assertEquals(L"refcount should not change anymore", 0, psub1->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelCompositeReader::testReaderClosedListener1(
    bool closeSubReaders, int wrapMultiReaderType) 
{
  shared_ptr<Directory> *const dir1 = getDir1(random());
  shared_ptr<CompositeReader> *const ir2, *const ir1 =
                                              DirectoryReader::open(dir1);
  switch (wrapMultiReaderType) {
  case 0:
    ir2 = ir1;
    break;
  case 1:
    // default case, does close subreaders:
    ir2 = make_shared<MultiReader>(ir1);
    break;
  case 2:
    ir2 = make_shared<MultiReader>(
        std::deque<std::shared_ptr<CompositeReader>>{ir1}, false);
    break;
  default:
    throw make_shared<AssertionError>();
  }

  // with overlapping
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      closeSubReaders, std::deque<std::shared_ptr<CompositeReader>>{ir2},
      std::deque<std::shared_ptr<CompositeReader>>{ir2});

  assertEquals(3, pr->leaves()->size());
  assertEquals(ir1->getReaderCacheHelper(), pr->getReaderCacheHelper());

  int i = 0;
  for (shared_ptr<LeafReaderContext> cxt : pr->leaves()) {
    shared_ptr<LeafReader> originalLeaf = ir1->leaves()[i++]->reader();
    assertEquals(originalLeaf->getCoreCacheHelper(),
                 cxt->reader()->getCoreCacheHelper());
    assertEquals(originalLeaf->getReaderCacheHelper(),
                 cxt->reader()->getReaderCacheHelper());
  }
  pr->close();
  if (!closeSubReaders) {
    delete ir1;
  }

  // We have to close the extra MultiReader, because it will not close its own
  // subreaders:
  if (wrapMultiReaderType == 2) {
    delete ir2;
  }
  delete dir1;
}

void TestParallelCompositeReader::testReaderClosedListener1() throw(
    runtime_error)
{
  testReaderClosedListener1(false, 0);
  testReaderClosedListener1(true, 0);
  testReaderClosedListener1(false, 1);
  testReaderClosedListener1(true, 1);
  testReaderClosedListener1(false, 2);
}

void TestParallelCompositeReader::testCloseInnerReader() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<CompositeReader> ir1 = DirectoryReader::open(dir1);
  assertEquals(1, ir1->getSequentialSubReaders()[0]->getRefCount());

  // with overlapping
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      true, std::deque<std::shared_ptr<CompositeReader>>{ir1},
      std::deque<std::shared_ptr<CompositeReader>>{ir1});

  shared_ptr<IndexReader> psub = pr->getSequentialSubReaders()[0];
  assertEquals(1, psub->getRefCount());

  delete ir1;

  assertEquals(L"refCount of synthetic subreader should be unchanged", 1,
               psub->getRefCount());
  expectThrows(AlreadyClosedException::typeid, [&]() { psub->document(0); });

  expectThrows(AlreadyClosedException::typeid, [&]() { pr->document(0); });

  // noop:
  pr->close();
  assertEquals(0, psub->getRefCount());
  delete dir1;
}

void TestParallelCompositeReader::testIncompatibleIndexes1() 
{
  // two documents:
  shared_ptr<Directory> dir1 = getDir1(random());

  // one document only:
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d3 = make_shared<Document>();

  d3->push_back(newTextField(L"f3", L"v1", Field::Store::YES));
  w2->addDocument(d3);
  delete w2;

  shared_ptr<DirectoryReader> ir1 = DirectoryReader::open(dir1),
                              ir2 = DirectoryReader::open(dir2);

  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<ParallelCompositeReader>(ir1, ir2); });

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ParallelCompositeReader>(random()->nextBoolean(), ir1, ir2);
  });

  assertEquals(1, ir1->getRefCount());
  assertEquals(1, ir2->getRefCount());
  ir1->close();
  ir2->close();
  assertEquals(0, ir1->getRefCount());
  assertEquals(0, ir2->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelCompositeReader::testIncompatibleIndexes2() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getInvalidStructuredDir2(random());

  shared_ptr<DirectoryReader> ir1 = DirectoryReader::open(dir1),
                              ir2 = DirectoryReader::open(dir2);
  std::deque<std::shared_ptr<CompositeReader>> readers = {ir1, ir2};
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<ParallelCompositeReader>(readers); });

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ParallelCompositeReader>(random()->nextBoolean(), readers,
                                         readers);
  });

  assertEquals(1, ir1->getRefCount());
  assertEquals(1, ir2->getRefCount());
  ir1->close();
  ir2->close();
  assertEquals(0, ir1->getRefCount());
  assertEquals(0, ir2->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelCompositeReader::testIgnoreStoredFields() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<CompositeReader> ir1 = DirectoryReader::open(dir1);
  shared_ptr<CompositeReader> ir2 = DirectoryReader::open(dir2);

  // with overlapping
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      false, std::deque<std::shared_ptr<CompositeReader>>{ir1, ir2},
      std::deque<std::shared_ptr<CompositeReader>>{ir1});
  assertEquals(L"v1", pr->document(0)->get(L"f1"));
  assertEquals(L"v1", pr->document(0)->get(L"f2"));
  assertNull(pr->document(0)->get(L"f3"));
  assertNull(pr->document(0)->get(L"f4"));
  // check that fields are there
  shared_ptr<Fields> slow = MultiFields::getFields(pr);
  assertNotNull(slow->terms(L"f1"));
  assertNotNull(slow->terms(L"f2"));
  assertNotNull(slow->terms(L"f3"));
  assertNotNull(slow->terms(L"f4"));
  pr->close();

  // no stored fields at all
  pr = make_shared<ParallelCompositeReader>(
      false, std::deque<std::shared_ptr<CompositeReader>>{ir2},
      std::deque<std::shared_ptr<CompositeReader>>(0));
  assertNull(pr->document(0)->get(L"f1"));
  assertNull(pr->document(0)->get(L"f2"));
  assertNull(pr->document(0)->get(L"f3"));
  assertNull(pr->document(0)->get(L"f4"));
  // check that fields are there
  slow = MultiFields::getFields(pr);
  assertNull(slow->terms(L"f1"));
  assertNull(slow->terms(L"f2"));
  assertNotNull(slow->terms(L"f3"));
  assertNotNull(slow->terms(L"f4"));
  pr->close();

  // without overlapping
  pr = make_shared<ParallelCompositeReader>(
      true, std::deque<std::shared_ptr<CompositeReader>>{ir2},
      std::deque<std::shared_ptr<CompositeReader>>{ir1});
  assertEquals(L"v1", pr->document(0)->get(L"f1"));
  assertEquals(L"v1", pr->document(0)->get(L"f2"));
  assertNull(pr->document(0)->get(L"f3"));
  assertNull(pr->document(0)->get(L"f4"));
  // check that fields are there
  slow = MultiFields::getFields(pr);
  assertNull(slow->terms(L"f1"));
  assertNull(slow->terms(L"f2"));
  assertNotNull(slow->terms(L"f3"));
  assertNotNull(slow->terms(L"f4"));
  pr->close();

  // no main readers
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ParallelCompositeReader>(
        true, std::deque<std::shared_ptr<CompositeReader>>(0),
        std::deque<std::shared_ptr<CompositeReader>>{ir1});
  });

  delete dir1;
  delete dir2;
}

void TestParallelCompositeReader::testToString() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<CompositeReader> ir1 = DirectoryReader::open(dir1);
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      std::deque<std::shared_ptr<CompositeReader>>{ir1});

  // C++ TODO: There is no native C++ equivalent to 'toString':
  const wstring s = pr->toString();
  assertTrue(L"toString incorrect: " + s,
             StringHelper::startsWith(
                 s, L"ParallelCompositeReader(ParallelLeafReader("));

  pr->close();
  delete dir1;
}

void TestParallelCompositeReader::testToStringCompositeComposite() throw(
    IOException)
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<CompositeReader> ir1 = DirectoryReader::open(dir1);
  shared_ptr<ParallelCompositeReader> pr = make_shared<ParallelCompositeReader>(
      std::deque<std::shared_ptr<CompositeReader>>{
          make_shared<MultiReader>(ir1)});

  // C++ TODO: There is no native C++ equivalent to 'toString':
  const wstring s = pr->toString();
  assertTrue(L"toString incorrect (should be flattened): " + s,
             StringHelper::startsWith(
                 s, L"ParallelCompositeReader(ParallelLeafReader("));

  pr->close();
  delete dir1;
}

void TestParallelCompositeReader::queryTest(shared_ptr<Query> query) throw(
    IOException)
{
  std::deque<std::shared_ptr<ScoreDoc>> parallelHits =
      parallel_->search(query, 1000)->scoreDocs;
  std::deque<std::shared_ptr<ScoreDoc>> singleHits =
      single_->search(query, 1000)->scoreDocs;
  assertEquals(parallelHits.size(), singleHits.size());
  for (int i = 0; i < parallelHits.size(); i++) {
    assertEquals(parallelHits[i]->score, singleHits[i]->score, 0.001f);
    shared_ptr<Document> docParallel = parallel_->doc(parallelHits[i]->doc);
    shared_ptr<Document> docSingle = single_->doc(singleHits[i]->doc);
    assertEquals(docParallel[L"f1"], docSingle[L"f1"]);
    assertEquals(docParallel[L"f2"], docSingle[L"f2"]);
    assertEquals(docParallel[L"f3"], docSingle[L"f3"]);
    assertEquals(docParallel[L"f4"], docSingle[L"f4"]);
  }
}

shared_ptr<IndexSearcher>
TestParallelCompositeReader::single(shared_ptr<Random> random,
                                    bool compositeComposite) 
{
  dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random)));
  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"f1", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f2", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f3", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f4", L"v1", Field::Store::YES));
  w->addDocument(d1);
  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"f1", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f2", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f3", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f4", L"v2", Field::Store::YES));
  w->addDocument(d2);
  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"f1", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f2", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f3", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f4", L"v3", Field::Store::YES));
  w->addDocument(d3);
  shared_ptr<Document> d4 = make_shared<Document>();
  d4->push_back(newTextField(L"f1", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f2", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f3", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f4", L"v4", Field::Store::YES));
  w->addDocument(d4);
  delete w;

  shared_ptr<CompositeReader> *const ir;
  if (compositeComposite) {
    ir = make_shared<MultiReader>(DirectoryReader::open(dir),
                                  DirectoryReader::open(dir));
  } else {
    ir = DirectoryReader::open(dir);
  }
  return newSearcher(ir);
}

shared_ptr<IndexSearcher> TestParallelCompositeReader::parallel(
    shared_ptr<Random> random, bool compositeComposite) 
{
  dir1 = getDir1(random);
  dir2 = getDir2(random);
  shared_ptr<CompositeReader> *const rd1, *const rd2;
  if (compositeComposite) {
    rd1 = make_shared<MultiReader>(DirectoryReader::open(dir1),
                                   DirectoryReader::open(dir1));
    rd2 = make_shared<MultiReader>(DirectoryReader::open(dir2),
                                   DirectoryReader::open(dir2));
    assertEquals(2, rd1->getContext()->children().size());
    assertEquals(2, rd2->getContext()->children().size());
  } else {
    rd1 = DirectoryReader::open(dir1);
    rd2 = DirectoryReader::open(dir2);
    assertEquals(3, rd1->getContext()->children().size());
    assertEquals(3, rd2->getContext()->children().size());
  }
  shared_ptr<ParallelCompositeReader> pr =
      make_shared<ParallelCompositeReader>(rd1, rd2);
  return newSearcher(pr);
}

shared_ptr<Directory> TestParallelCompositeReader::getDir1(
    shared_ptr<Random> random) 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
                ->setMergePolicy(NoMergePolicy::INSTANCE));
  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"f1", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f2", L"v1", Field::Store::YES));
  w1->addDocument(d1);
  w1->commit();
  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"f1", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f2", L"v2", Field::Store::YES));
  w1->addDocument(d2);
  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"f1", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f2", L"v3", Field::Store::YES));
  w1->addDocument(d3);
  w1->commit();
  shared_ptr<Document> d4 = make_shared<Document>();
  d4->push_back(newTextField(L"f1", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f2", L"v4", Field::Store::YES));
  w1->addDocument(d4);
  delete w1;
  return dir1;
}

shared_ptr<Directory> TestParallelCompositeReader::getDir2(
    shared_ptr<Random> random) 
{
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
                ->setMergePolicy(NoMergePolicy::INSTANCE));
  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"f3", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f4", L"v1", Field::Store::YES));
  w2->addDocument(d1);
  w2->commit();
  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"f3", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f4", L"v2", Field::Store::YES));
  w2->addDocument(d2);
  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"f3", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f4", L"v3", Field::Store::YES));
  w2->addDocument(d3);
  w2->commit();
  shared_ptr<Document> d4 = make_shared<Document>();
  d4->push_back(newTextField(L"f3", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f4", L"v4", Field::Store::YES));
  w2->addDocument(d4);
  delete w2;
  return dir2;
}

shared_ptr<Directory> TestParallelCompositeReader::getInvalidStructuredDir2(
    shared_ptr<Random> random) 
{
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
                ->setMergePolicy(NoMergePolicy::INSTANCE));
  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"f3", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f4", L"v1", Field::Store::YES));
  w2->addDocument(d1);
  w2->commit();
  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"f3", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f4", L"v2", Field::Store::YES));
  w2->addDocument(d2);
  w2->commit();
  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"f3", L"v3", Field::Store::YES));
  d3->push_back(newTextField(L"f4", L"v3", Field::Store::YES));
  w2->addDocument(d3);
  shared_ptr<Document> d4 = make_shared<Document>();
  d4->push_back(newTextField(L"f3", L"v4", Field::Store::YES));
  d4->push_back(newTextField(L"f4", L"v4", Field::Store::YES));
  w2->addDocument(d4);
  delete w2;
  return dir2;
}
} // namespace org::apache::lucene::index