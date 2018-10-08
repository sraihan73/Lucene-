using namespace std;

#include "TestParallelLeafReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using namespace org::apache::lucene::search;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestParallelLeafReader::testQueries() 
{
  single_ = single(random());
  parallel_ = parallel(random());

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

void TestParallelLeafReader::testFieldNames() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      getOnlyLeafReader(DirectoryReader::open(dir1)),
      getOnlyLeafReader(DirectoryReader::open(dir2)));
  shared_ptr<FieldInfos> fieldInfos = pr->getFieldInfos();
  TestUtil::assertEquals(4, fieldInfos->size());
  assertNotNull(fieldInfos->fieldInfo(L"f1"));
  assertNotNull(fieldInfos->fieldInfo(L"f2"));
  assertNotNull(fieldInfos->fieldInfo(L"f3"));
  assertNotNull(fieldInfos->fieldInfo(L"f4"));
  delete pr;
  delete dir1;
  delete dir2;
}

void TestParallelLeafReader::testRefCounts1() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<LeafReader> ir1, ir2;
  // close subreaders, ParallelReader will not change refCounts, but close on
  // its own close
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      ir1 = getOnlyLeafReader(DirectoryReader::open(dir1)),
      ir2 = getOnlyLeafReader(DirectoryReader::open(dir2)));

  // check RefCounts
  TestUtil::assertEquals(1, ir1->getRefCount());
  TestUtil::assertEquals(1, ir2->getRefCount());
  delete pr;
  TestUtil::assertEquals(0, ir1->getRefCount());
  TestUtil::assertEquals(0, ir2->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelLeafReader::testRefCounts2() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<LeafReader> ir1 = getOnlyLeafReader(DirectoryReader::open(dir1));
  shared_ptr<LeafReader> ir2 = getOnlyLeafReader(DirectoryReader::open(dir2));
  // don't close subreaders, so ParallelReader will increment refcounts
  shared_ptr<ParallelLeafReader> pr =
      make_shared<ParallelLeafReader>(false, ir1, ir2);
  // check RefCounts
  TestUtil::assertEquals(2, ir1->getRefCount());
  TestUtil::assertEquals(2, ir2->getRefCount());
  delete pr;
  TestUtil::assertEquals(1, ir1->getRefCount());
  TestUtil::assertEquals(1, ir2->getRefCount());
  delete ir1;
  delete ir2;
  TestUtil::assertEquals(0, ir1->getRefCount());
  TestUtil::assertEquals(0, ir2->getRefCount());
  delete dir1;
  delete dir2;
}

void TestParallelLeafReader::testCloseInnerReader() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<LeafReader> ir1 = getOnlyLeafReader(DirectoryReader::open(dir1));

  // with overlapping
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      true, std::deque<std::shared_ptr<LeafReader>>{ir1},
      std::deque<std::shared_ptr<LeafReader>>{ir1});

  delete ir1;

  // should already be closed because inner reader is closed!
  expectThrows(AlreadyClosedException::typeid, [&]() { pr->document(0); });

  // noop:
  delete pr;
  delete dir1;
}

void TestParallelLeafReader::testIncompatibleIndexes() 
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

  shared_ptr<LeafReader> ir1 = getOnlyLeafReader(DirectoryReader::open(dir1));
  shared_ptr<LeafReader> ir2 = getOnlyLeafReader(DirectoryReader::open(dir2));

  // indexes don't have the same number of documents
  expectThrows(invalid_argument::typeid,
               [&]() { make_shared<ParallelLeafReader>(ir1, ir2); });

  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ParallelLeafReader>(
        random()->nextBoolean(),
        std::deque<std::shared_ptr<LeafReader>>{ir1, ir2},
        std::deque<std::shared_ptr<LeafReader>>{ir1, ir2});
  });

  // check RefCounts
  TestUtil::assertEquals(1, ir1->getRefCount());
  TestUtil::assertEquals(1, ir2->getRefCount());
  delete ir1;
  delete ir2;
  delete dir1;
  delete dir2;
}

void TestParallelLeafReader::testIgnoreStoredFields() 
{
  shared_ptr<Directory> dir1 = getDir1(random());
  shared_ptr<Directory> dir2 = getDir2(random());
  shared_ptr<LeafReader> ir1 = getOnlyLeafReader(DirectoryReader::open(dir1));
  shared_ptr<LeafReader> ir2 = getOnlyLeafReader(DirectoryReader::open(dir2));

  // with overlapping
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      false, std::deque<std::shared_ptr<LeafReader>>{ir1, ir2},
      std::deque<std::shared_ptr<LeafReader>>{ir1});
  TestUtil::assertEquals(L"v1", pr->document(0)[L"f1"]);
  TestUtil::assertEquals(L"v1", pr->document(0)[L"f2"]);
  assertNull(pr->document(0)[L"f3"]);
  assertNull(pr->document(0)[L"f4"]);
  // check that fields are there
  assertNotNull(pr->terms(L"f1"));
  assertNotNull(pr->terms(L"f2"));
  assertNotNull(pr->terms(L"f3"));
  assertNotNull(pr->terms(L"f4"));
  delete pr;

  // no stored fields at all
  pr = make_shared<ParallelLeafReader>(
      false, std::deque<std::shared_ptr<LeafReader>>{ir2},
      std::deque<std::shared_ptr<LeafReader>>(0));
  assertNull(pr->document(0)[L"f1"]);
  assertNull(pr->document(0)[L"f2"]);
  assertNull(pr->document(0)[L"f3"]);
  assertNull(pr->document(0)[L"f4"]);
  // check that fields are there
  assertNull(pr->terms(L"f1"));
  assertNull(pr->terms(L"f2"));
  assertNotNull(pr->terms(L"f3"));
  assertNotNull(pr->terms(L"f4"));
  delete pr;

  // without overlapping
  pr = make_shared<ParallelLeafReader>(
      true, std::deque<std::shared_ptr<LeafReader>>{ir2},
      std::deque<std::shared_ptr<LeafReader>>{ir1});
  TestUtil::assertEquals(L"v1", pr->document(0)[L"f1"]);
  TestUtil::assertEquals(L"v1", pr->document(0)[L"f2"]);
  assertNull(pr->document(0)[L"f3"]);
  assertNull(pr->document(0)[L"f4"]);
  // check that fields are there
  assertNull(pr->terms(L"f1"));
  assertNull(pr->terms(L"f2"));
  assertNotNull(pr->terms(L"f3"));
  assertNotNull(pr->terms(L"f4"));
  delete pr;

  // no main readers
  expectThrows(invalid_argument::typeid, [&]() {
    make_shared<ParallelLeafReader>(
        true, std::deque<std::shared_ptr<LeafReader>>(0),
        std::deque<std::shared_ptr<LeafReader>>{ir1});
  });

  delete dir1;
  delete dir2;
}

void TestParallelLeafReader::queryTest(shared_ptr<Query> query) throw(
    IOException)
{
  std::deque<std::shared_ptr<ScoreDoc>> parallelHits =
      parallel_->search(query, 1000)->scoreDocs;
  std::deque<std::shared_ptr<ScoreDoc>> singleHits =
      single_->search(query, 1000)->scoreDocs;
  TestUtil::assertEquals(parallelHits.size(), singleHits.size());
  for (int i = 0; i < parallelHits.size(); i++) {
    assertEquals(parallelHits[i]->score, singleHits[i]->score, 0.001f);
    shared_ptr<Document> docParallel = parallel_->doc(parallelHits[i]->doc);
    shared_ptr<Document> docSingle = single_->doc(singleHits[i]->doc);
    TestUtil::assertEquals(docParallel[L"f1"], docSingle[L"f1"]);
    TestUtil::assertEquals(docParallel[L"f2"], docSingle[L"f2"]);
    TestUtil::assertEquals(docParallel[L"f3"], docSingle[L"f3"]);
    TestUtil::assertEquals(docParallel[L"f4"], docSingle[L"f4"]);
  }
}

shared_ptr<IndexSearcher>
TestParallelLeafReader::single(shared_ptr<Random> random) 
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
  delete w;

  shared_ptr<DirectoryReader> ir = DirectoryReader::open(dir);
  return newSearcher(ir);
}

shared_ptr<IndexSearcher>
TestParallelLeafReader::parallel(shared_ptr<Random> random) 
{
  dir1 = getDir1(random);
  dir2 = getDir2(random);
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      getOnlyLeafReader(DirectoryReader::open(dir1)),
      getOnlyLeafReader(DirectoryReader::open(dir2)));
  TestUtil::checkReader(pr);
  return newSearcher(pr);
}

shared_ptr<Directory>
TestParallelLeafReader::getDir1(shared_ptr<Random> random) 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random)));
  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(newTextField(L"f1", L"v1", Field::Store::YES));
  d1->push_back(newTextField(L"f2", L"v1", Field::Store::YES));
  w1->addDocument(d1);
  shared_ptr<Document> d2 = make_shared<Document>();
  d2->push_back(newTextField(L"f1", L"v2", Field::Store::YES));
  d2->push_back(newTextField(L"f2", L"v2", Field::Store::YES));
  w1->addDocument(d2);
  w1->forceMerge(1);
  delete w1;
  return dir1;
}

shared_ptr<Directory>
TestParallelLeafReader::getDir2(shared_ptr<Random> random) 
{
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random)));
  shared_ptr<Document> d3 = make_shared<Document>();
  d3->push_back(newTextField(L"f3", L"v1", Field::Store::YES));
  d3->push_back(newTextField(L"f4", L"v1", Field::Store::YES));
  w2->addDocument(d3);
  shared_ptr<Document> d4 = make_shared<Document>();
  d4->push_back(newTextField(L"f3", L"v2", Field::Store::YES));
  d4->push_back(newTextField(L"f4", L"v2", Field::Store::YES));
  w2->addDocument(d4);
  w2->forceMerge(1);
  delete w2;
  return dir2;
}

void TestParallelLeafReader::testWithIndexSort1() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc1->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT)));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);
  w1->addDocument(make_shared<Document>());
  w1->commit();
  w1->addDocument(make_shared<Document>());
  w1->forceMerge(1);
  delete w1;
  shared_ptr<IndexReader> r1 = DirectoryReader::open(dir1);

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc2->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"bar", SortField::Type::INT)));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc2);
  w2->addDocument(make_shared<Document>());
  w2->commit();
  w2->addDocument(make_shared<Document>());
  w2->forceMerge(1);
  delete w2;
  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir2);

  wstring message =
      expectThrows(invalid_argument::typeid,
                   [&]() {
                     make_shared<ParallelLeafReader>(getOnlyLeafReader(r1),
                                                     getOnlyLeafReader(r2));
                   })
          .what();
  TestUtil::assertEquals(
      L"cannot combine LeafReaders that have different index sorts: saw both "
      L"sort=<int: \"foo\"> and <int: \"bar\">",
      message);
  IOUtils::close({r1, dir1, r2, dir2});
}

void TestParallelLeafReader::testWithIndexSort2() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  iwc1->setIndexSort(
      make_shared<Sort>(make_shared<SortField>(L"foo", SortField::Type::INT)));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);
  w1->addDocument(make_shared<Document>());
  w1->commit();
  w1->addDocument(make_shared<Document>());
  w1->forceMerge(1);
  delete w1;
  shared_ptr<IndexReader> r1 = DirectoryReader::open(dir1);

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc2 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w2 = make_shared<IndexWriter>(dir2, iwc2);
  w2->addDocument(make_shared<Document>());
  w2->addDocument(make_shared<Document>());
  delete w2;

  shared_ptr<IndexReader> r2 = DirectoryReader::open(dir2);
  delete (make_shared<ParallelLeafReader>(false, getOnlyLeafReader(r1),
                                          getOnlyLeafReader(r2)));
  delete (make_shared<ParallelLeafReader>(false, getOnlyLeafReader(r2),
                                          getOnlyLeafReader(r1)));
  IOUtils::close({r1, dir1, r2, dir2});
}

void TestParallelLeafReader::testWithDocValuesUpdates() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<IndexWriterConfig> iwc1 =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> w1 = make_shared<IndexWriter>(dir1, iwc1);
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"name", L"billy", Field::Store::NO));
  d->push_back(make_shared<NumericDocValuesField>(L"age", 21));
  w1->addDocument(d);
  w1->commit();
  w1->updateNumericDocValue(make_shared<Term>(L"name", L"billy"), L"age", 22);
  delete w1;

  shared_ptr<IndexReader> r1 = DirectoryReader::open(dir1);
  shared_ptr<LeafReader> lr =
      make_shared<ParallelLeafReader>(false, getOnlyLeafReader(r1));

  shared_ptr<NumericDocValues> dv = lr->getNumericDocValues(L"age");
  TestUtil::assertEquals(0, dv->nextDoc());
  TestUtil::assertEquals(22, dv->longValue());

  TestUtil::assertEquals(
      1, lr->getFieldInfos()->fieldInfo(L"age")->getDocValuesGen());

  IOUtils::close({lr, r1, dir1});
}
} // namespace org::apache::lucene::index