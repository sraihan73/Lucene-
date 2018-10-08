using namespace std;

#include "TestSoftDeletesRetentionMergePolicy.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using MatchNoDocsQuery = org::apache::lucene::search::MatchNoDocsQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSoftDeletesRetentionMergePolicy::testForceMergeFullyDeleted() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<AtomicBoolean> letItGo = make_shared<AtomicBoolean>(false);
  shared_ptr<MergePolicy> policy = make_shared<SoftDeletesRetentionMergePolicy>(
      L"soft_delete",
      [&]() {
        letItGo->get()
            ? make_shared<org::apache::lucene::search::MatchNoDocsQuery>()
            : make_shared<org::apache::lucene::search::MatchAllDocsQuery>();
      },
      make_shared<LogDocMergePolicy>());
  shared_ptr<IndexWriterConfig> indexWriterConfig =
      newIndexWriterConfig()->setMergePolicy(policy)->setSoftDeletesField(
          L"soft_delete");
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  writer->addDocument(doc);
  writer->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  {
    assertEquals(2, reader->leaves()->size());
    shared_ptr<SegmentReader> *const segmentReader =
        std::static_pointer_cast<SegmentReader>(
            reader->leaves()->get(0).reader());
    assertTrue(policy->keepFullyDeletedSegment([&]() { segmentReader; }));
    assertEquals(0, policy->numDeletesToMerge(segmentReader->getSegmentInfo(),
                                              0, [&]() { segmentReader; }));
  }
  {
    shared_ptr<SegmentReader> segmentReader =
        std::static_pointer_cast<SegmentReader>(
            reader->leaves()->get(1).reader());
    assertTrue(policy->keepFullyDeletedSegment([&]() { segmentReader; }));
    assertEquals(0, policy->numDeletesToMerge(segmentReader->getSegmentInfo(),
                                              0, [&]() { segmentReader; }));
    writer->forceMerge(1);
    reader->close();
  }
  reader = writer->getReader();
  {
    assertEquals(1, reader->leaves()->size());
    shared_ptr<SegmentReader> segmentReader =
        std::static_pointer_cast<SegmentReader>(
            reader->leaves()->get(0).reader());
    assertEquals(2, reader->maxDoc());
    assertTrue(policy->keepFullyDeletedSegment([&]() { segmentReader; }));
    assertEquals(0, policy->numDeletesToMerge(segmentReader->getSegmentInfo(),
                                              0, [&]() { segmentReader; }));
  }
  writer->forceMerge(1); // make sure we don't merge this
  assertNull(DirectoryReader::openIfChanged(reader));

  writer->forceMergeDeletes(); // make sure we don't merge this
  assertNull(DirectoryReader::openIfChanged(reader));
  letItGo->set(true);
  writer->forceMergeDeletes(); // make sure we don't merge this
  shared_ptr<DirectoryReader> directoryReader =
      DirectoryReader::openIfChanged(reader);
  assertNotNull(directoryReader);
  assertEquals(0, directoryReader->numDeletedDocs());
  assertEquals(0, directoryReader->maxDoc());
  IOUtils::close({directoryReader, reader, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::testKeepFullyDeletedSegments() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->leaves()->size());
  shared_ptr<MergePolicy> policy = make_shared<SoftDeletesRetentionMergePolicy>(
      L"soft_delete",
      [&]() {
        make_shared<org::apache::lucene::search::DocValuesFieldExistsQuery>(
            L"keep_around");
      },
      NoMergePolicy::INSTANCE);
  assertFalse(policy->keepFullyDeletedSegment([&]() {
    std::static_pointer_cast<SegmentReader>(reader->leaves()->get(0).reader());
  }));
  reader->close();

  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<NumericDocValuesField>(L"keep_around", 1));
  doc->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  writer->addDocument(doc);

  shared_ptr<DirectoryReader> reader1 = writer->getReader();
  assertEquals(2, reader1->leaves()->size());
  assertFalse(policy->keepFullyDeletedSegment([&]() {
    std::static_pointer_cast<SegmentReader>(reader1->leaves()->get(0).reader());
  }));

  assertTrue(policy->keepFullyDeletedSegment([&]() {
    std::static_pointer_cast<SegmentReader>(reader1->leaves()->get(1).reader());
  }));

  IOUtils::close({reader1, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::testFieldBasedRetention() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  shared_ptr<Instant> now = Instant::now();
  shared_ptr<Instant> time24HoursAgo = now->minus(Duration::ofDays(1));
  wstring softDeletesField = L"soft_delete";
  function<Query *()> docsOfLast24Hours = [&]() {
    org::apache::lucene::document::LongPoint::newRangeQuery(
        L"creation_date", time24HoursAgo->toEpochMilli(), now->toEpochMilli());
  };
  indexWriterConfig->setMergePolicy(
      make_shared<SoftDeletesRetentionMergePolicy>(
          softDeletesField, docsOfLast24Hours,
          make_shared<LogDocMergePolicy>()));
  indexWriterConfig->setSoftDeletesField(softDeletesField);
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  int64_t time28HoursAgo = now->minus(Duration::ofHours(28)).toEpochMilli();
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"1", Field::Store::YES));
  doc->push_back(make_shared<LongPoint>(L"creation_date", time28HoursAgo));
  writer->addDocument(doc);

  writer->flush();
  int64_t time26HoursAgo = now->minus(Duration::ofHours(26)).toEpochMilli();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"2", Field::Store::YES));
  doc->push_back(make_shared<LongPoint>(L"creation_date", time26HoursAgo));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  if (random()->nextBoolean()) {
    writer->flush();
  }
  int64_t time23HoursAgo = now->minus(Duration::ofHours(23)).toEpochMilli();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"3", Field::Store::YES));
  doc->push_back(make_shared<LongPoint>(L"creation_date", time23HoursAgo));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  if (random()->nextBoolean()) {
    writer->flush();
  }
  int64_t time12HoursAgo = now->minus(Duration::ofHours(12)).toEpochMilli();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"4", Field::Store::YES));
  doc->push_back(make_shared<LongPoint>(L"creation_date", time12HoursAgo));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  if (random()->nextBoolean()) {
    writer->flush();
  }
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(make_shared<StringField>(L"version", L"5", Field::Store::YES));
  doc->push_back(make_shared<LongPoint>(L"creation_date", now->toEpochMilli()));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  if (random()->nextBoolean()) {
    writer->flush();
  }
  writer->forceMerge(1);
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(1, reader->numDocs());
  assertEquals(3, reader->maxDoc());
  shared_ptr<Set<wstring>> versions = unordered_set<wstring>();
  versions->add(
      reader->document(0, Collections::singleton(L"version"))->get(L"version"));
  versions->add(
      reader->document(1, Collections::singleton(L"version"))->get(L"version"));
  versions->add(
      reader->document(2, Collections::singleton(L"version"))->get(L"version"));
  assertTrue(versions->contains(L"5"));
  assertTrue(versions->contains(L"4"));
  assertTrue(versions->contains(L"3"));
  IOUtils::close({reader, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::testKeepAllDocsAcrossMerges() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  indexWriterConfig->setMergePolicy(
      make_shared<SoftDeletesRetentionMergePolicy>(
          L"soft_delete",
          [&]() {
            make_shared<org::apache::lucene::search::MatchAllDocsQuery>();
          },
          make_shared<LogDocMergePolicy>()));
  indexWriterConfig->setSoftDeletesField(L"soft_delete");
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);

  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  writer->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});

  writer->commit();
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  doc->push_back(
      make_shared<NumericDocValuesField>(L"soft_delete", 1)); // already deleted
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), doc,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  writer->commit();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  assertEquals(0, reader->numDocs());
  assertEquals(3, reader->maxDoc());
  assertEquals(0, writer->numDocs());
  assertEquals(3, writer->maxDoc());
  assertEquals(3, reader->leaves()->size());
  reader->close();
  writer->forceMerge(1);
  reader = writer->getReader();
  assertEquals(0, reader->numDocs());
  assertEquals(3, reader->maxDoc());
  assertEquals(0, writer->numDocs());
  assertEquals(3, writer->maxDoc());
  assertEquals(1, reader->leaves()->size());
  IOUtils::close({reader, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::testSoftDeleteWithRetention() throw(
    IOException, InterruptedException)
{
  shared_ptr<AtomicInteger> seqIds = make_shared<AtomicInteger>(0);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> indexWriterConfig = newIndexWriterConfig();
  indexWriterConfig->setMergePolicy(
      make_shared<SoftDeletesRetentionMergePolicy>(
          L"soft_delete",
          [&]() {
            org::apache::lucene::document::IntPoint::newRangeQuery(
                L"seq_id", seqIds->intValue() - 50, numeric_limits<int>::max());
          },
          indexWriterConfig->getMergePolicy()));
  indexWriterConfig->setSoftDeletesField(L"soft_delete");
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, indexWriterConfig);
  std::deque<std::shared_ptr<Thread>> threads(2 + random()->nextInt(3));
  shared_ptr<CountDownLatch> startLatch = make_shared<CountDownLatch>(1);
  shared_ptr<CountDownLatch> started =
      make_shared<CountDownLatch>(threads.size());
  bool updateSeveralDocs = random()->nextBoolean();
  shared_ptr<Set<wstring>> ids =
      Collections::synchronizedSet(unordered_set<wstring>());
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<Thread>([&]() {
      try {
        started->countDown();
        startLatch->await();
        for (int d = 0; d < 100; d++) {
          wstring id = wstring::valueOf(random()->nextInt(10));
          int seqId = seqIds->incrementAndGet();
          if (updateSeveralDocs) {
            shared_ptr<Document> doc = make_shared<Document>();
            doc->add(make_shared<StringField>(L"id", id, Field::Store::YES));
            doc->add(make_shared<IntPoint>(L"seq_id", seqId));
                    writer->softUpdateDocuments(make_shared<Term>(L"id", id), deque<? extends deque<? extends IndexableField>> {doc, doc}, {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
          } else {
            shared_ptr<Document> doc = make_shared<Document>();
            doc->add(make_shared<StringField>(L"id", id, Field::Store::YES));
            doc->add(make_shared<IntPoint>(L"seq_id", seqId));
            writer->softUpdateDocument(
                make_shared<Term>(L"id", id), doc,
                {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
          }
          if (rarely()) {
            writer->flush();
          }
          ids->add(id);
        }
      } catch (IOException | InterruptedException e) {
        throw make_shared<AssertionError>(e);
      }
    });
    threads[i]->start();
  }
  started->await();
  startLatch->countDown();

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
  }
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  for (auto id : ids) {
    shared_ptr<TopDocs> topDocs = searcher->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", id)), 10);
    if (updateSeveralDocs) {
      assertEquals(2, topDocs->totalHits);
      assertEquals(abs(topDocs->scoreDocs[0]->doc - topDocs->scoreDocs[1]->doc),
                   1);
    } else {
      assertEquals(1, topDocs->totalHits);
    }
  }
  writer->addDocument(
      make_shared<Document>()); // add a dummy doc to trigger a segment here
  writer->flush();
  writer->forceMerge(1);
  shared_ptr<DirectoryReader> oldReader = reader;
  reader = DirectoryReader::openIfChanged(reader, writer);
  if (reader != nullptr) {
    oldReader->close();
    assertNotSame(oldReader, reader);
  } else {
    reader = oldReader;
  }
  assertEquals(1, reader->leaves()->size());
  shared_ptr<LeafReaderContext> leafReaderContext = reader->leaves()->get(0);
  shared_ptr<LeafReader> leafReader = leafReaderContext->reader();
  searcher = make_shared<IndexSearcher>(
      make_shared<FilterLeafReaderAnonymousInnerClass>(shared_from_this(),
                                                       leafReader));
  shared_ptr<TopDocs> seq_id = searcher->search(
      IntPoint::newRangeQuery(L"seq_id", seqIds->intValue() - 50,
                              numeric_limits<int>::max()),
      10);
  assertTrue(to_wstring(seq_id->totalHits) + L" hits", seq_id->totalHits >= 50);
  searcher = make_shared<IndexSearcher>(reader);
  for (auto id : ids) {
    if (updateSeveralDocs) {
      assertEquals(
          2,
          searcher
              ->search(make_shared<TermQuery>(make_shared<Term>(L"id", id)), 10)
              ->totalHits);
    } else {
      assertEquals(
          1,
          searcher
              ->search(make_shared<TermQuery>(make_shared<Term>(L"id", id)), 10)
              ->totalHits);
    }
  }
  IOUtils::close({reader, writer, dir});
}

TestSoftDeletesRetentionMergePolicy::FilterLeafReaderAnonymousInnerClass::
    FilterLeafReaderAnonymousInnerClass(
        shared_ptr<TestSoftDeletesRetentionMergePolicy> outerInstance,
        shared_ptr<org::apache::lucene::index::LeafReader> leafReader)
    : FilterLeafReader(leafReader)
{
  this->outerInstance = outerInstance;
  this->leafReader = leafReader;
}

shared_ptr<CacheHelper> TestSoftDeletesRetentionMergePolicy::
    FilterLeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return leafReader->getCoreCacheHelper();
}

shared_ptr<CacheHelper> TestSoftDeletesRetentionMergePolicy::
    FilterLeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return leafReader->getReaderCacheHelper();
}

shared_ptr<Bits> TestSoftDeletesRetentionMergePolicy::
    FilterLeafReaderAnonymousInnerClass::getLiveDocs()
{
  return nullptr;
}

int TestSoftDeletesRetentionMergePolicy::FilterLeafReaderAnonymousInnerClass::
    numDocs()
{
  return maxDoc();
}

void TestSoftDeletesRetentionMergePolicy::testForceMergeDeletes() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig()->setSoftDeletesField(L"soft_delete");
  config->setMergePolicy(newMergePolicy(
      random(),
      false)); // no mock MP it might not select segments for force merge
  if (random()->nextBoolean()) {
    config->setMergePolicy(make_shared<SoftDeletesRetentionMergePolicy>(
        L"soft_delete",
        [&]() { make_shared<org::apache::lucene::search::MatchNoDocsQuery>(); },
        config->getMergePolicy()));
  }
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  // The first segment includes d1 and d2
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                          Field::Store::YES));
    writer->addDocument(d);
  }
  writer->flush();
  // The second segment includes only the tombstone
  shared_ptr<Document> tombstone = make_shared<Document>();
  tombstone->push_back(make_shared<NumericDocValuesField>(L"soft_delete", 1));
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"1"), tombstone,
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  writer->forceMergeDeletes(true); // Internally, forceMergeDeletes will call
                                   // flush to flush pending updates
  // Thus, we will have two segments - both having soft-deleted documents.
  // We expect any MP to merge these segments into one segment
  // when calling forceMergeDeletes.
  assertEquals(1, writer->segmentInfos->asList().size());
  assertEquals(1, writer->numDocs());
  assertEquals(1, writer->maxDoc());
  delete writer;
  delete dir;
}

void TestSoftDeletesRetentionMergePolicy::
    testDropFullySoftDeletedSegment() 
{
  shared_ptr<Directory> dir = newDirectory();
  wstring softDelete = random()->nextBoolean() ? L"" : L"soft_delete";
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig()->setSoftDeletesField(softDelete);
  config->setMergePolicy(newMergePolicy(random(), true));
  if (softDelete != L"" && random()->nextBoolean()) {
    config->setMergePolicy(make_shared<SoftDeletesRetentionMergePolicy>(
        softDelete,
        [&]() { make_shared<org::apache::lucene::search::MatchNoDocsQuery>(); },
        config->getMergePolicy()));
  }
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                          Field::Store::YES));
    writer->addDocument(d);
  }
  writer->flush();
  assertEquals(1, writer->segmentInfos->asList().size());

  if (softDelete != L"") {
    // the newly created segment should be dropped as it is fully deleted (i.e.
    // only contains deleted docs).
    if (random()->nextBoolean()) {
      shared_ptr<Document> tombstone = make_shared<Document>();
      tombstone->push_back(make_shared<NumericDocValuesField>(softDelete, 1));
      writer->softUpdateDocument(
          make_shared<Term>(L"id", L"1"), tombstone,
          {make_shared<NumericDocValuesField>(softDelete, 1)});
    } else {
      shared_ptr<Document> doc = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<StringField>(L"id", Integer::toString(1),
                                              Field::Store::YES));
      if (random()->nextBoolean()) {
        writer->softUpdateDocument(
            make_shared<Term>(L"id", L"1"), doc,
            {make_shared<NumericDocValuesField>(softDelete, 1)});
      } else {
        writer->addDocument(doc);
      }
      writer->updateDocValues(
          make_shared<Term>(L"id", L"1"),
          {make_shared<NumericDocValuesField>(softDelete, 1)});
    }
  } else {
    shared_ptr<Document> d = make_shared<Document>();
    d->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
    writer->addDocument(d);
    writer->deleteDocuments({make_shared<Term>(L"id", L"1")});
  }
  writer->commit();
  shared_ptr<IndexReader> reader = writer->getReader();
  assertEquals(reader->numDocs(), 1);
  delete reader;
  assertEquals(1, writer->segmentInfos->asList().size());

  delete writer;
  delete dir;
}

void TestSoftDeletesRetentionMergePolicy::
    testSoftDeleteWhileMergeSurvives() 
{
  shared_ptr<Directory> dir = newDirectory();
  wstring softDelete = L"soft_delete";
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig()->setSoftDeletesField(softDelete);
  shared_ptr<AtomicBoolean> update = make_shared<AtomicBoolean>(true);
  config->setReaderPooling(true);
  config->setMergePolicy(make_shared<SoftDeletesRetentionMergePolicy>(
      L"soft_delete",
      [&]() {
        make_shared<org::apache::lucene::search::DocValuesFieldExistsQuery>(
            L"keep");
      },
      make_shared<LogDocMergePolicy>()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  writer->getConfig()->setMergedSegmentWarmer([&](any sr) {
    if (update->compareAndSet(true, false)) {
      try {
        writer->softUpdateDocument(
            make_shared<Term>(L"id", L"0"), make_shared<Document>(),
            {make_shared<NumericDocValuesField>(softDelete, 1),
             make_shared<NumericDocValuesField>(L"keep", 1)});
        writer->commit();
      } catch (const IOException &e) {
        throw make_shared<AssertionError>(e);
      }
    }
  });

  bool preExistingDeletes = random()->nextBoolean();
  for (int i = 0; i < 2; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    d->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                          Field::Store::YES));
    if (preExistingDeletes && random()->nextBoolean()) {
      writer->addDocument(
          d); // randomly add a preexisting hard-delete that we don't carry over
              // C++ TODO: There is no native C++ equivalent to 'toString':
      writer->deleteDocuments({make_shared<Term>(L"id", Integer::toString(i))});
      d->push_back(make_shared<NumericDocValuesField>(L"keep", 1));
      writer->addDocument(d);
    } else {
      d->push_back(make_shared<NumericDocValuesField>(L"keep", 1));
      writer->addDocument(d);
    }
    writer->flush();
  }
  writer->forceMerge(1);
  writer->commit();
  assertFalse(update->get());
  shared_ptr<DirectoryReader> open = DirectoryReader::open(dir);
  assertEquals(0, open->numDeletedDocs());
  assertEquals(3, open->maxDoc());
  IOUtils::close({open, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::
    testDeleteDocWhileMergeThatIsSoftDeleted() 
{
  shared_ptr<Directory> dir = newDirectory();
  wstring softDelete = L"soft_delete";
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig()->setSoftDeletesField(softDelete);
  shared_ptr<AtomicBoolean> delete = make_shared<AtomicBoolean>(true);
  config->setReaderPooling(true);
  config->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  writer->addDocument(d);
  d = make_shared<Document>();
  d->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->addDocument(d);
  if (random()->nextBoolean()) {
    // randomly run with a preexisting hard delete
    d = make_shared<Document>();
    d->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
    writer->addDocument(d);
    writer->deleteDocuments({make_shared<Term>(L"id", L"2")});
  }

  writer->flush();
  shared_ptr<DirectoryReader> reader = writer->getReader();
  writer->softUpdateDocument(
      make_shared<Term>(L"id", L"0"), make_shared<Document>(),
      {make_shared<NumericDocValuesField>(softDelete, 1)});
  writer->flush();
  writer->getConfig()->setMergedSegmentWarmer([&](any sr) {
    if (delete->compareAndSet(true, false)) {
      try {
        int64_t seqNo = writer->tryDeleteDocument(reader, 0);
        assertTrue(L"seqId was -1", seqNo != -1);
      } catch (const IOException &e) {
        throw make_shared<AssertionError>(e);
      }
    }
  });
  writer->forceMerge(1);
  assertEquals(2, writer->numDocs());
  assertEquals(2, writer->maxDoc());
  assertFalse(delete->get());
  IOUtils::close({reader, writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::testUndeleteDocument() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  wstring softDelete = L"soft_delete";
  shared_ptr<IndexWriterConfig> config =
      newIndexWriterConfig()
          ->setSoftDeletesField(softDelete)
          ->setMergePolicy(make_shared<SoftDeletesRetentionMergePolicy>(
              L"soft_delete", MatchAllDocsQuery::new,
              make_shared<LogDocMergePolicy>()));
  config->setReaderPooling(true);
  config->setMergePolicy(make_shared<LogDocMergePolicy>());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(make_shared<StringField>(L"id", L"0", Field::Store::YES));
  d->push_back(make_shared<StringField>(L"seq_id", L"0", Field::Store::YES));
  writer->addDocument(d);
  d = make_shared<Document>();
  d->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
  writer->addDocument(d);
  writer->updateDocValues(
      make_shared<Term>(L"id", L"0"),
      {make_shared<NumericDocValuesField>(L"soft_delete", 1)});
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (IndexReader reader = writer.getReader())
  {
    IndexReader reader = writer->getReader();
    assertEquals(2, reader->maxDoc());
    assertEquals(1, reader->numDocs());
  }
  doUpdate(make_shared<Term>(L"id", L"0"), writer,
           {make_shared<NumericDocValuesField>(L"soft_delete", nullptr)});
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (IndexReader reader = writer.getReader())
  {
    IndexReader reader = writer->getReader();
    assertEquals(2, reader->maxDoc());
    assertEquals(2, reader->numDocs());
  }
  IOUtils::close({writer, dir});
}

void TestSoftDeletesRetentionMergePolicy::doUpdate(
    shared_ptr<Term> doc, shared_ptr<IndexWriter> writer,
    deque<Field> &fields) 
{
  int64_t seqId = -1;
  do { // retry if we just committing a merge
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (DirectoryReader reader =
    // writer.getReader())
    {
      DirectoryReader reader = writer->getReader();
      shared_ptr<TopDocs> topDocs =
          (make_shared<IndexSearcher>(make_shared<NoDeletesWrapper>(reader)))
              ->search(make_shared<TermQuery>(doc), 10);
      assertEquals(1, topDocs->totalHits);
      int theDoc = topDocs->scoreDocs[0]->doc;
      seqId = writer->tryUpdateDocValue(reader, theDoc, {fields});
    }
  } while (seqId == -1);
}

shared_ptr<LeafReader>
TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  return make_shared<FilterLeafReaderAnonymousInnerClass2>(shared_from_this(),
                                                           reader);
}

TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::
    FilterLeafReaderAnonymousInnerClass2::FilterLeafReaderAnonymousInnerClass2(
        shared_ptr<NoDeletesSubReaderWrapper> outerInstance,
        shared_ptr<org::apache::lucene::index::LeafReader> reader)
    : FilterLeafReader(reader)
{
  this->outerInstance = outerInstance;
}

int TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::
    FilterLeafReaderAnonymousInnerClass2::numDocs()
{
  return maxDoc();
}

shared_ptr<Bits>
TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::
    FilterLeafReaderAnonymousInnerClass2::getLiveDocs()
{
  return nullptr;
}

shared_ptr<CacheHelper>
TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::
    FilterLeafReaderAnonymousInnerClass2::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper>
TestSoftDeletesRetentionMergePolicy::NoDeletesSubReaderWrapper::
    FilterLeafReaderAnonymousInnerClass2::getReaderCacheHelper()
{
  return nullptr;
}

TestSoftDeletesRetentionMergePolicy::NoDeletesWrapper::NoDeletesWrapper(
    shared_ptr<DirectoryReader> in_) 
    : FilterDirectoryReader(in_, new NoDeletesSubReaderWrapper())
{
}

shared_ptr<DirectoryReader>
TestSoftDeletesRetentionMergePolicy::NoDeletesWrapper::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<NoDeletesWrapper>(in_);
}

shared_ptr<CacheHelper>
TestSoftDeletesRetentionMergePolicy::NoDeletesWrapper::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::index