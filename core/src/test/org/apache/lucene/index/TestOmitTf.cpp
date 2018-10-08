using namespace std;

#include "TestOmitTf.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using CollectionStatistics = org::apache::lucene::search::CollectionStatistics;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using TermQuery = org::apache::lucene::search::TermQuery;
using TermStatistics = org::apache::lucene::search::TermStatistics;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

float TestOmitTf::SimpleSimilarity::lengthNorm(int length) { return 1; }

float TestOmitTf::SimpleSimilarity::tf(float freq) { return freq; }

float TestOmitTf::SimpleSimilarity::sloppyFreq(int distance) { return 2.0f; }

float TestOmitTf::SimpleSimilarity::idf(int64_t docFreq, int64_t docCount)
{
  return 1.0f;
}

shared_ptr<Explanation> TestOmitTf::SimpleSimilarity::idfExplain(
    shared_ptr<CollectionStatistics> collectionStats,
    std::deque<std::shared_ptr<TermStatistics>> &termStats)
{
  return Explanation::match(1.0f, L"Inexplicable");
}

float TestOmitTf::SimpleSimilarity::scorePayload(int doc, int start, int end,
                                                 shared_ptr<BytesRef> payload)
{
  return 1.0f;
}

const shared_ptr<org::apache::lucene::document::FieldType>
    TestOmitTf::omitType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);
const shared_ptr<org::apache::lucene::document::FieldType>
    TestOmitTf::normalType =
        make_shared<org::apache::lucene::document::FieldType>(
            org::apache::lucene::document::TextField::TYPE_NOT_STORED);

TestOmitTf::StaticConstructor::StaticConstructor()
{
  omitType->setIndexOptions(IndexOptions::DOCS);
}

TestOmitTf::StaticConstructor TestOmitTf::staticConstructor;

void TestOmitTf::testOmitTermFreqAndPositions() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(ram, newIndexWriterConfig(analyzer));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have Tf
  shared_ptr<Field> f1 =
      newField(L"f1", L"This field has term freqs", normalType);
  d->push_back(f1);

  // this field will NOT have Tf
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO Tf in all docs", omitType);
  d->push_back(f2);

  writer->addDocument(d);
  writer->forceMerge(1);
  // now we add another document which has term freq for field f2 and not for f1
  // and verify if the SegmentMerger keep things constant
  d = make_shared<Document>();

  // Reverse
  f1 = newField(L"f1", L"This field has term freqs", omitType);
  d->push_back(f1);

  f2 = newField(L"f2", L"This field has NO Tf in all docs", normalType);
  d->push_back(f2);

  writer->addDocument(d);

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertEquals(L"OmitTermFreqAndPositions field bit should be set.",
               IndexOptions::DOCS, fi->fieldInfo(L"f1")->getIndexOptions());
  assertEquals(L"OmitTermFreqAndPositions field bit should be set.",
               IndexOptions::DOCS, fi->fieldInfo(L"f2")->getIndexOptions());

  delete reader;
  delete ram;
}

void TestOmitTf::testMixedMerge() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(3)->setMergePolicy(
          newLogMergePolicy(2)));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have Tf
  shared_ptr<Field> f1 =
      newField(L"f1", L"This field has term freqs", normalType);
  d->push_back(f1);

  // this field will NOT have Tf
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO Tf in all docs", omitType);
  d->push_back(f2);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // now we add another document which has term freq for field f2 and not for f1
  // and verify if the SegmentMerger keep things constant
  d = make_shared<Document>();

  // Reverese
  f1 = newField(L"f1", L"This field has term freqs", omitType);
  d->push_back(f1);

  f2 = newField(L"f2", L"This field has NO Tf in all docs", normalType);
  d->push_back(f2);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertEquals(L"OmitTermFreqAndPositions field bit should be set.",
               IndexOptions::DOCS, fi->fieldInfo(L"f1")->getIndexOptions());
  assertEquals(L"OmitTermFreqAndPositions field bit should be set.",
               IndexOptions::DOCS, fi->fieldInfo(L"f2")->getIndexOptions());

  delete reader;
  delete ram;
}

void TestOmitTf::testMixedRAM() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(10)->setMergePolicy(
          newLogMergePolicy(2)));
  shared_ptr<Document> d = make_shared<Document>();

  // this field will have Tf
  shared_ptr<Field> f1 =
      newField(L"f1", L"This field has term freqs", normalType);
  d->push_back(f1);

  // this field will NOT have Tf
  shared_ptr<Field> f2 =
      newField(L"f2", L"This field has NO Tf in all docs", omitType);
  d->push_back(f2);

  for (int i = 0; i < 5; i++) {
    writer->addDocument(d);
  }

  for (int i = 0; i < 20; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);

  // flush
  delete writer;

  shared_ptr<LeafReader> reader = getOnlyLeafReader(DirectoryReader::open(ram));
  shared_ptr<FieldInfos> fi = reader->getFieldInfos();
  assertEquals(L"OmitTermFreqAndPositions field bit should not be set.",
               IndexOptions::DOCS_AND_FREQS_AND_POSITIONS,
               fi->fieldInfo(L"f1")->getIndexOptions());
  assertEquals(L"OmitTermFreqAndPositions field bit should be set.",
               IndexOptions::DOCS, fi->fieldInfo(L"f2")->getIndexOptions());

  delete reader;
  delete ram;
}

void TestOmitTf::assertNoPrx(shared_ptr<Directory> dir) 
{
  const std::deque<wstring> files = dir->listAll();
  for (int i = 0; i < files.size(); i++) {
    assertFalse(StringHelper::endsWith(files[i], L".prx"));
    assertFalse(StringHelper::endsWith(files[i], L".pos"));
  }
}

void TestOmitTf::testNoPrxFile() 
{
  shared_ptr<Directory> ram = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      ram,
      newIndexWriterConfig(analyzer)->setMaxBufferedDocs(3)->setMergePolicy(
          newLogMergePolicy()));
  shared_ptr<LogMergePolicy> lmp = std::static_pointer_cast<LogMergePolicy>(
      writer->getConfig()->getMergePolicy());
  lmp->setMergeFactor(2);
  lmp->setNoCFSRatio(0.0);
  shared_ptr<Document> d = make_shared<Document>();

  shared_ptr<Field> f1 =
      newField(L"f1", L"This field has term freqs", omitType);
  d->push_back(f1);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  writer->commit();

  assertNoPrx(ram);

  // now add some documents with positions, and check
  // there is no prox after full merge
  d = make_shared<Document>();
  f1 = newTextField(L"f1", L"This field has positions", Field::Store::NO);
  d->push_back(f1);

  for (int i = 0; i < 30; i++) {
    writer->addDocument(d);
  }

  // force merge
  writer->forceMerge(1);
  // flush
  delete writer;

  assertNoPrx(ram);
  delete ram;
}

void TestOmitTf::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(analyzer)
               ->setMaxBufferedDocs(2)
               ->setSimilarity(make_shared<SimpleSimilarity>())
               ->setMergePolicy(newLogMergePolicy(2)));

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(265);
  wstring term = L"term";
  for (int i = 0; i < 30; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    sb->append(term)->append(L" ");
    wstring content = sb->toString();
    shared_ptr<Field> noTf =
        newField(L"noTf", content + (i % 2 == 0 ? L"" : L" notf"), omitType);
    d->push_back(noTf);

    shared_ptr<Field> tf =
        newField(L"tf", content + (i % 2 == 0 ? L" tf" : L""), normalType);
    d->push_back(tf);

    writer->addDocument(d);
    // System.out.println(d);
  }

  writer->forceMerge(1);
  // flush
  delete writer;

  /*
   * Verify the index
   */
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  searcher->setSimilarity(make_shared<SimpleSimilarity>());

  shared_ptr<Term> a = make_shared<Term>(L"noTf", term);
  shared_ptr<Term> b = make_shared<Term>(L"tf", term);
  shared_ptr<Term> c = make_shared<Term>(L"noTf", L"notf");
  shared_ptr<Term> d = make_shared<Term>(L"tf", L"tf");
  shared_ptr<TermQuery> q1 = make_shared<TermQuery>(a);
  shared_ptr<TermQuery> q2 = make_shared<TermQuery>(b);
  shared_ptr<TermQuery> q3 = make_shared<TermQuery>(c);
  shared_ptr<TermQuery> q4 = make_shared<TermQuery>(d);

  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(a->field(), a->bytes(), c->bytes());
  runtime_error expected =
      expectThrows(runtime_error::typeid, [&]() { searcher->search(pq, 10); });
  runtime_error cause = expected;
  // If the searcher uses an executor service, the IAE is wrapped into other
  // exceptions
  while (cause.getCause() != nullptr) {
    cause = cause.getCause();
  }
  assertTrue(L"Expected an IAE, got " + cause,
             std::dynamic_pointer_cast<IllegalStateException>(cause) !=
                 nullptr);

  searcher->search(q1, make_shared<CountingHitCollectorAnonymousInnerClass>(
                           shared_from_this()));
  // System.out.println(CountingHitCollector.getCount());

  searcher->search(q2, make_shared<CountingHitCollectorAnonymousInnerClass2>(
                           shared_from_this()));
  // System.out.println(CountingHitCollector.getCount());

  searcher->search(q3, make_shared<CountingHitCollectorAnonymousInnerClass3>(
                           shared_from_this()));
  // System.out.println(CountingHitCollector.getCount());

  searcher->search(q4, make_shared<CountingHitCollectorAnonymousInnerClass4>(
                           shared_from_this()));
  // System.out.println(CountingHitCollector.getCount());

  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(q1, Occur::MUST);
  bq->add(q4, Occur::MUST);

  searcher->search(bq->build(),
                   make_shared<CountingHitCollectorAnonymousInnerClass5>(
                       shared_from_this()));
  assertEquals(15, CountingHitCollector::getCount());

  delete reader;
  delete dir;
}

TestOmitTf::CountingHitCollectorAnonymousInnerClass::
    CountingHitCollectorAnonymousInnerClass(
        shared_ptr<TestOmitTf> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestOmitTf::CountingHitCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass::collect(
    int doc) 
{
  // System.out.println("Q1: Doc=" + doc + " score=" + score);
  float score = scorer::score();
  assertTrue(L"got score=" + to_wstring(score), score == 1.0f);
  outerInstance->super->collect(doc);
}

TestOmitTf::CountingHitCollectorAnonymousInnerClass2::
    CountingHitCollectorAnonymousInnerClass2(
        shared_ptr<TestOmitTf> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestOmitTf::CountingHitCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  // System.out.println("Q2: Doc=" + doc + " score=" + score);
  float score = scorer::score();
  assertEquals(1.0f + doc, score, 0.00001f);
  outerInstance->super->collect(doc);
}

TestOmitTf::CountingHitCollectorAnonymousInnerClass3::
    CountingHitCollectorAnonymousInnerClass3(
        shared_ptr<TestOmitTf> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestOmitTf::CountingHitCollectorAnonymousInnerClass3::needsScores()
{
  return true;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass3::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass3::collect(
    int doc) 
{
  // System.out.println("Q1: Doc=" + doc + " score=" + score);
  float score = scorer::score();
  assertTrue(score == 1.0f);
  assertFalse(doc % 2 == 0);
  outerInstance->super->collect(doc);
}

TestOmitTf::CountingHitCollectorAnonymousInnerClass4::
    CountingHitCollectorAnonymousInnerClass4(
        shared_ptr<TestOmitTf> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestOmitTf::CountingHitCollectorAnonymousInnerClass4::needsScores()
{
  return true;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass4::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass4::collect(
    int doc) 
{
  float score = scorer::score();
  // System.out.println("Q1: Doc=" + doc + " score=" + score);
  assertTrue(score == 1.0f);
  assertTrue(doc % 2 == 0);
  outerInstance->super->collect(doc);
}

TestOmitTf::CountingHitCollectorAnonymousInnerClass5::
    CountingHitCollectorAnonymousInnerClass5(
        shared_ptr<TestOmitTf> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TestOmitTf::CountingHitCollectorAnonymousInnerClass5::collect(
    int doc) 
{
  // System.out.println("BQ: Doc=" + doc + " score=" + score);
  outerInstance->super->collect(doc);
}

int TestOmitTf::CountingHitCollector::count = 0;
int TestOmitTf::CountingHitCollector::sum = 0;

TestOmitTf::CountingHitCollector::CountingHitCollector()
{
  count = 0;
  sum = 0;
}

void TestOmitTf::CountingHitCollector::collect(int doc) 
{
  count++;
  sum += doc + docBase; // use it to avoid any possibility of being merged away
}

int TestOmitTf::CountingHitCollector::getCount() { return count; }

int TestOmitTf::CountingHitCollector::getSum() { return sum; }

void TestOmitTf::CountingHitCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  docBase = context->docBase;
}

bool TestOmitTf::CountingHitCollector::needsScores() { return false; }

void TestOmitTf::testStats() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->freeze();
  shared_ptr<Field> f = newField(L"foo", L"bar", ft);
  doc->push_back(f);
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  assertEquals(-1, ir->totalTermFreq(make_shared<Term>(
                       L"foo", make_shared<BytesRef>(L"bar"))));
  assertEquals(-1, ir->getSumTotalTermFreq(L"foo"));
  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::index