using namespace std;

#include "TestMixedDocValuesUpdates.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using DocValuesFieldExistsQuery =
    org::apache::lucene::search::DocValuesFieldExistsQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

void TestMixedDocValuesUpdates::testManyReopensAndFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestMixedDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  shared_ptr<LogMergePolicy> lmp = newLogMergePolicy();
  lmp->setMergeFactor(3); // merge often
  conf->setMergePolicy(lmp);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  constexpr bool isNRT = random->nextBoolean();
  shared_ptr<DirectoryReader> reader;
  if (isNRT) {
    reader = DirectoryReader::open(writer);
  } else {
    writer->commit();
    reader = DirectoryReader::open(dir);
  }

  constexpr int numFields = random->nextInt(4) + 3;                // 3-7
  constexpr int numNDVFields = random->nextInt(numFields / 2) + 1; // 1-3
  const std::deque<int64_t> fieldValues = std::deque<int64_t>(numFields);
  for (int i = 0; i < fieldValues.size(); i++) {
    fieldValues[i] = 1;
  }

  int numRounds = atLeast(15);
  int docID = 0;
  for (int i = 0; i < numRounds; i++) {
    int numDocs = atLeast(5);
    // System.out.println("TEST: round=" + i + ", numDocs=" + numDocs);
    for (int j = 0; j < numDocs; j++) {
      shared_ptr<Document> doc = make_shared<Document>();
      doc->push_back(make_shared<StringField>(
          L"id", L"doc-" + to_wstring(docID), Field::Store::NO));
      doc->push_back(make_shared<StringField>(L"key", L"all",
                                              Field::Store::NO)); // update key
      // add all fields with their current value
      for (int f = 0; f < fieldValues.size(); f++) {
        if (f < numNDVFields) {
          doc->push_back(make_shared<NumericDocValuesField>(
              L"f" + to_wstring(f), fieldValues[f]));
        } else {
          doc->push_back(make_shared<BinaryDocValuesField>(
              L"f" + to_wstring(f),
              TestBinaryDocValuesUpdates::toBytes(fieldValues[f])));
        }
      }
      writer->addDocument(doc);
      ++docID;
    }

    int fieldIdx = random->nextInt(fieldValues.size());
    wstring updateField = L"f" + to_wstring(fieldIdx);
    if (fieldIdx < numNDVFields) {
      writer->updateNumericDocValue(make_shared<Term>(L"key", L"all"),
                                    updateField, ++fieldValues[fieldIdx]);
    } else {
      writer->updateBinaryDocValue(
          make_shared<Term>(L"key", L"all"), updateField,
          TestBinaryDocValuesUpdates::toBytes(++fieldValues[fieldIdx]));
    }
    // System.out.println("TEST: updated field '" + updateField + "' to value "
    // + fieldValues[fieldIdx]);

    if (random->nextDouble() < 0.2) {
      int deleteDoc = random->nextInt(
          docID); // might also delete an already deleted document, ok!
      writer->deleteDocuments(
          {make_shared<Term>(L"id", L"doc-" + to_wstring(deleteDoc))});
      //        System.out.println("[" + Thread.currentThread().getName() + "]:
      //        deleted document: doc-" + deleteDoc);
    }

    // verify reader
    if (!isNRT) {
      writer->commit();
    }

    //      System.out.println("[" + Thread.currentThread().getName() + "]:
    //      reopen reader: " + reader);
    shared_ptr<DirectoryReader> newReader =
        DirectoryReader::openIfChanged(reader);
    assertNotNull(newReader);
    reader->close();
    reader = newReader;
    //      System.out.println("[" + Thread.currentThread().getName() + "]:
    //      reopened reader: " + reader);
    assertTrue(reader->numDocs() >
               0); // we delete at most one document per round
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      //        System.out.println(((SegmentReader) r).getSegmentName());
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int field = 0; field < fieldValues.size(); field++) {
        wstring f = L"f" + to_wstring(field);
        shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(f);
        shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(f);
        if (field < numNDVFields) {
          assertNotNull(ndv);
          assertNull(bdv);
        } else {
          assertNull(ndv);
          assertNotNull(bdv);
        }
        int maxDoc = r->maxDoc();
        for (int doc = 0; doc < maxDoc; doc++) {
          if (liveDocs == nullptr || liveDocs->get(doc)) {
            //              System.out.println("doc=" + (doc + context.docBase)
            //              + " f='" + f + "' vslue=" + getValue(bdv, doc,
            //              scratch));
            if (field < numNDVFields) {
              TestUtil::assertEquals(doc, ndv->advance(doc));
              assertEquals(L"invalid numeric value for doc=" + to_wstring(doc) +
                               L", field=" + f + L", reader=" + r,
                           fieldValues[field], ndv->longValue());
            } else {
              TestUtil::assertEquals(doc, bdv->advance(doc));
              assertEquals(L"invalid binary value for doc=" + to_wstring(doc) +
                               L", field=" + f + L", reader=" + r,
                           fieldValues[field],
                           TestBinaryDocValuesUpdates::getValue(bdv));
            }
          }
        }
      }
    }
    //      System.out.println();
  }

  delete writer;
  IOUtils::close({reader, dir});
}

void TestMixedDocValuesUpdates::testStressMultiThreading() 
{
  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(dir, conf);

  // create index
  constexpr int numFields = TestUtil::nextInt(random(), 2, 4);
  constexpr int numThreads = TestUtil::nextInt(random(), 3, 6);
  constexpr int numDocs = atLeast(2000);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"doc" + to_wstring(i),
                                            Field::Store::NO));
    double group = random()->nextDouble();
    wstring g;
    if (group < 0.1) {
      g = L"g0";
    } else if (group < 0.5) {
      g = L"g1";
    } else if (group < 0.8) {
      g = L"g2";
    } else {
      g = L"g3";
    }
    doc->push_back(make_shared<StringField>(L"updKey", g, Field::Store::NO));
    for (int j = 0; j < numFields; j++) {
      int64_t value = random()->nextInt();
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"f" + to_wstring(j), TestBinaryDocValuesUpdates::toBytes(value)));
      doc->push_back(make_shared<NumericDocValuesField>(
          L"cf" + to_wstring(j),
          value * 2)); // control, always updated to f * 2
    }
    writer->addDocument(doc);
  }

  shared_ptr<CountDownLatch> *const done =
      make_shared<CountDownLatch>(numThreads);
  shared_ptr<AtomicInteger> *const numUpdates =
      make_shared<AtomicInteger>(atLeast(100));

  // same thread updates a field as well as reopens
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), L"UpdateThread-" + to_wstring(i), writer, numFields,
        numDocs, done, numUpdates);
  }

  for (auto t : threads) {
    t->start();
  }
  done->await();
  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    for (int i = 0; i < numFields; i++) {
      shared_ptr<BinaryDocValues> bdv =
          r->getBinaryDocValues(L"f" + to_wstring(i));
      shared_ptr<NumericDocValues> control =
          r->getNumericDocValues(L"cf" + to_wstring(i));
      shared_ptr<Bits> liveDocs = r->getLiveDocs();
      for (int j = 0; j < r->maxDoc(); j++) {
        if (liveDocs == nullptr || liveDocs->get(j)) {
          TestUtil::assertEquals(j, control->advance(j));
          int64_t ctrlValue = control->longValue();
          TestUtil::assertEquals(j, bdv->advance(j));
          int64_t bdvValue = TestBinaryDocValuesUpdates::getValue(bdv) * 2;
          //              if (ctrlValue != bdvValue) {
          //                System.out.println("seg=" + r + ", f=f" + i + ",
          //                doc=" + j + ", group=" + r.document(j).get("updKey")
          //                + ", ctrlValue=" + ctrlValue + ", bdvBytes=" +
          //                scratch);
          //              }
          TestUtil::assertEquals(ctrlValue, bdvValue);
        }
      }
    }
  }
  reader->close();

  delete dir;
}

TestMixedDocValuesUpdates::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestMixedDocValuesUpdates> outerInstance,
    wstring L"UpdateThread-" + i,
    shared_ptr<org::apache::lucene::index::IndexWriter> writer, int numFields,
    int numDocs, shared_ptr<CountDownLatch> done,
    shared_ptr<AtomicInteger> numUpdates)
    : Thread(L"UpdateThread-" + i)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->numFields = numFields;
  this->numDocs = numDocs;
  this->done = done;
  this->numUpdates = numUpdates;
}

void TestMixedDocValuesUpdates::ThreadAnonymousInnerClass::run()
{
  shared_ptr<DirectoryReader> reader = nullptr;
  bool success = false;
  try {
    shared_ptr<Random> random = TestMixedDocValuesUpdates::random();
    while (numUpdates->getAndDecrement() > 0) {
      double group = random->nextDouble();
      shared_ptr<Term> t;
      if (group < 0.1) {
        t = make_shared<Term>(L"updKey", L"g0");
      } else if (group < 0.5) {
        t = make_shared<Term>(L"updKey", L"g1");
      } else if (group < 0.8) {
        t = make_shared<Term>(L"updKey", L"g2");
      } else {
        t = make_shared<Term>(L"updKey", L"g3");
      }
      //              System.out.println("[" + Thread.currentThread().getName()
      //              + "] numUpdates=" + numUpdates + " updateTerm=" + t);
      int field = TestMixedDocValuesUpdates::random()->nextInt(numFields);
      const wstring f = L"f" + to_wstring(field);
      const wstring cf = L"cf" + to_wstring(field);
      int64_t updValue = random->nextInt();
      //              System.err.println("[" + Thread.currentThread().getName()
      //              + "] t=" + t + ", f=" + f + ", updValue=" + updValue);
      writer->updateDocValues(
          t, {make_shared<BinaryDocValuesField>(
                  f, TestBinaryDocValuesUpdates::toBytes(updValue)),
              make_shared<NumericDocValuesField>(cf, updValue * 2)});

      if (random->nextDouble() < 0.2) {
        // delete a random document
        int doc = random->nextInt(numDocs);
        //                System.out.println("[" +
        //                Thread.currentThread().getName() + "] deleteDoc=doc" +
        //                doc);
        writer->deleteDocuments(
            {make_shared<Term>(L"id", L"doc" + to_wstring(doc))});
      }

      if (random->nextDouble() < 0.05) { // commit every 20 updates on average
        //                  System.out.println("[" +
        //                  Thread.currentThread().getName() + "] commit");
        writer->commit();
      }

      if (random->nextDouble() < 0.1) { // reopen NRT reader (apply updates), on
                                        // average once every 10 updates
        if (reader == nullptr) {
          //                  System.out.println("[" +
          //                  Thread.currentThread().getName() + "] open NRT");
          reader = DirectoryReader::open(writer);
        } else {
          //                  System.out.println("[" +
          //                  Thread.currentThread().getName() + "] reopen
          //                  NRT");
          shared_ptr<DirectoryReader> r2 =
              DirectoryReader::openIfChanged(reader, writer);
          if (r2 != nullptr) {
            reader->close();
            reader = r2;
          }
        }
      }
    }
    //            System.out.println("[" + Thread.currentThread().getName() + "]
    //            DONE");
    success = true;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (reader != nullptr) {
      try {
        reader->close();
      } catch (const IOException &e) {
        if (success) { // suppress this exception only if there was another
                       // exception
          throw runtime_error(e);
        }
      }
    }
    done->countDown();
  }
}

void TestMixedDocValuesUpdates::testUpdateDifferentDocsInDifferentGens() throw(
    runtime_error)
{
  // update same document multiple times across generations
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMaxBufferedDocs(4);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  constexpr int numDocs = atLeast(10);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"doc" + to_wstring(i),
                                            Field::Store::NO));
    int64_t value = random()->nextInt();
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"f", TestBinaryDocValuesUpdates::toBytes(value)));
    doc->push_back(make_shared<NumericDocValuesField>(L"cf", value * 2));
    writer->addDocument(doc);
  }

  int numGens = atLeast(5);
  for (int i = 0; i < numGens; i++) {
    int doc = random()->nextInt(numDocs);
    shared_ptr<Term> t = make_shared<Term>(L"id", L"doc" + to_wstring(doc));
    int64_t value = random()->nextLong();
    if (random()->nextBoolean()) {
      doUpdate(t, writer,
               {make_shared<BinaryDocValuesField>(
                    L"f", TestBinaryDocValuesUpdates::toBytes(value)),
                make_shared<NumericDocValuesField>(L"cf", value * 2)});
    } else {
      writer->updateDocValues(
          t, {make_shared<BinaryDocValuesField>(
                  L"f", TestBinaryDocValuesUpdates::toBytes(value)),
              make_shared<NumericDocValuesField>(L"cf", value * 2)});
    }

    shared_ptr<DirectoryReader> reader = DirectoryReader::open(writer);
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> fbdv = r->getBinaryDocValues(L"f");
      shared_ptr<NumericDocValues> cfndv = r->getNumericDocValues(L"cf");
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, cfndv->nextDoc());
        TestUtil::assertEquals(j, fbdv->nextDoc());
        TestUtil::assertEquals(cfndv->longValue(),
                               TestBinaryDocValuesUpdates::getValue(fbdv) * 2);
      }
    }
    reader->close();
  }
  delete writer;
  delete dir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testTonsOfUpdates() throws Exception
void TestMixedDocValuesUpdates::testTonsOfUpdates() 
{
  // LUCENE-5248: make sure that when there are many updates, we don't use too
  // much RAM
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Random> *const random = TestMixedDocValuesUpdates::random();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random));
  conf->setRAMBufferSizeMB(IndexWriterConfig::DEFAULT_RAM_BUFFER_SIZE_MB);
  conf->setMaxBufferedDocs(
      IndexWriterConfig::DISABLE_AUTO_FLUSH); // don't flush by doc
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  // test data: lots of documents (few 10Ks) and lots of update terms (few
  // hundreds)
  constexpr int numDocs = atLeast(20000);
  constexpr int numBinaryFields = atLeast(5);
  constexpr int numTerms =
      TestUtil::nextInt(random, 10, 100); // terms should affect many docs
  shared_ptr<Set<wstring>> updateTerms = unordered_set<wstring>();
  while (updateTerms->size() < numTerms) {
    updateTerms->add(TestUtil::randomSimpleString(random));
  }

  //    System.out.println("numDocs=" + numDocs + " numBinaryFields=" +
  //    numBinaryFields + " numTerms=" + numTerms);

  // build a large index with many BDV fields and update terms
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numUpdateTerms = TestUtil::nextInt(random, 1, numTerms / 10);
    for (int j = 0; j < numUpdateTerms; j++) {
      doc->push_back(make_shared<StringField>(
          L"upd", RandomPicks::randomFrom(random, updateTerms),
          Field::Store::NO));
    }
    for (int j = 0; j < numBinaryFields; j++) {
      int64_t val = random->nextInt();
      doc->push_back(make_shared<BinaryDocValuesField>(
          L"f" + to_wstring(j), TestBinaryDocValuesUpdates::toBytes(val)));
      doc->push_back(
          make_shared<NumericDocValuesField>(L"cf" + to_wstring(j), val * 2));
    }
    writer->addDocument(doc);
  }

  writer->commit(); // commit so there's something to apply to

  // set to flush every 2048 bytes (approximately every 12 updates), so we get
  // many flushes during binary updates
  writer->getConfig()->setRAMBufferSizeMB(2048.0 / 1024 / 1024);
  constexpr int numUpdates = atLeast(100);
  //    System.out.println("numUpdates=" + numUpdates);
  for (int i = 0; i < numUpdates; i++) {
    int field = random->nextInt(numBinaryFields);
    shared_ptr<Term> updateTerm =
        make_shared<Term>(L"upd", RandomPicks::randomFrom(random, updateTerms));
    int64_t value = random->nextInt();
    writer->updateDocValues(updateTerm,
                            {make_shared<BinaryDocValuesField>(
                                 L"f" + to_wstring(field),
                                 TestBinaryDocValuesUpdates::toBytes(value)),
                             make_shared<NumericDocValuesField>(
                                 L"cf" + to_wstring(field), value * 2)});
  }

  delete writer;

  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
    for (int i = 0; i < numBinaryFields; i++) {
      shared_ptr<LeafReader> r = context->reader();
      shared_ptr<BinaryDocValues> f =
          r->getBinaryDocValues(L"f" + to_wstring(i));
      shared_ptr<NumericDocValues> cf =
          r->getNumericDocValues(L"cf" + to_wstring(i));
      for (int j = 0; j < r->maxDoc(); j++) {
        TestUtil::assertEquals(j, cf->nextDoc());
        TestUtil::assertEquals(j, f->nextDoc());
        assertEquals(L"reader=" + r + L", field=f" + to_wstring(i) + L", doc=" +
                         to_wstring(j),
                     cf->longValue(),
                     TestBinaryDocValuesUpdates::getValue(f) * 2);
      }
    }
  }
  reader->close();

  delete dir;
}

void TestMixedDocValuesUpdates::testTryUpdateDocValues() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  int numDocs = 1 + random()->nextInt(128);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"" + to_wstring(i),
                                            Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"id", i));
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"binaryId",
        make_shared<BytesRef>(std::deque<char>{static_cast<char>(i)})));
    writer->addDocument(doc);
    if (random()->nextBoolean()) {
      writer->flush();
    }
  }
  int doc = random()->nextInt(numDocs);
  doUpdate(make_shared<Term>(L"id", L"" + to_wstring(doc)), writer,
           {make_shared<NumericDocValuesField>(L"id", doc + 1),
            make_shared<BinaryDocValuesField>(
                L"binaryId", make_shared<BytesRef>(std::deque<char>{
                                 static_cast<char>(doc + 1)}))});
  shared_ptr<IndexReader> reader = writer->getReader();
  shared_ptr<NumericDocValues> idValues = nullptr;
  shared_ptr<BinaryDocValues> binaryIdValues = nullptr;
  for (auto c : reader->leaves()) {
    shared_ptr<TopDocs> topDocs =
        (make_shared<IndexSearcher>(c->reader()))
            ->search(make_shared<TermQuery>(
                         make_shared<Term>(L"id", L"" + to_wstring(doc))),
                     10);
    if (topDocs->totalHits == 1) {
      assertNull(idValues);
      assertNull(binaryIdValues);
      idValues = c->reader()->getNumericDocValues(L"id");
      TestUtil::assertEquals(topDocs->scoreDocs[0]->doc,
                             idValues->advance(topDocs->scoreDocs[0]->doc));
      binaryIdValues = c->reader()->getBinaryDocValues(L"binaryId");
      TestUtil::assertEquals(
          topDocs->scoreDocs[0]->doc,
          binaryIdValues->advance(topDocs->scoreDocs[0]->doc));
    } else {
      TestUtil::assertEquals(0, topDocs->totalHits);
    }
  }

  assertNotNull(idValues);
  assertNotNull(binaryIdValues);

  TestUtil::assertEquals(doc + 1, idValues->longValue());
  TestUtil::assertEquals(
      make_shared<BytesRef>(std::deque<char>{static_cast<char>(doc + 1)}),
      binaryIdValues->binaryValue());
  IOUtils::close({reader, writer, dir});
}

void TestMixedDocValuesUpdates::testTryUpdateMultiThreaded() throw(
    IOException, BrokenBarrierException, InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  std::deque<std::shared_ptr<ReentrantLock>> locks(25 + random()->nextInt(50));
  std::deque<optional<int64_t>> values(locks.size());

  for (int i = 0; i < locks.size(); i++) {
    locks[i] = make_shared<ReentrantLock>();
    shared_ptr<Document> doc = make_shared<Document>();
    values[i] = random()->nextLong();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<StringField>(L"id", Integer::toString(i),
                                            Field::Store::NO));
    doc->push_back(make_shared<NumericDocValuesField>(L"value", values[i]));
    writer->addDocument(doc);
  }

  std::deque<std::shared_ptr<Thread>> threads(2 + random()->nextInt(3));
  shared_ptr<CyclicBarrier> barrier =
      make_shared<CyclicBarrier>(threads.size() + 1);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<Thread>([&]() {
      try {
        barrier->await();
        for (int doc = 0; doc < 1000; doc++) {
          int docId = random()->nextInt(locks.length);
          locks[docId]->lock();
          try {
            optional<int64_t> value =
                rarely() ? nullptr : random()->nextLong();
            if (random()->nextBoolean()) {
              writer->updateDocValues(
                  make_shared<Term>(L"id", to_wstring(docId) + L""),
                  {make_shared<NumericDocValuesField>(L"value", value)});
            } else {
              doUpdate(make_shared<Term>(L"id", to_wstring(docId) + L""),
                       writer,
                       {make_shared<NumericDocValuesField>(L"value", value)});
            }
            values[docId] = value;
          } catch (const IOException &e) {
            throw make_shared<AssertionError>(e);
          }
          // C++ TODO: There is no native C++ equivalent to the exception
          // 'finally' clause:
          finally {
            locks[docId]->unlock();
          }
          if (rarely()) {
            writer->flush();
          }
        }
      } catch (const runtime_error &e) {
        throw make_shared<AssertionError>(e);
      }
    });
    threads[i]->start();
  }

  barrier->await();
  for (auto t : threads) {
    t->join();
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (DirectoryReader reader = writer.getReader())
  {
    DirectoryReader reader = writer->getReader();
    for (int i = 0; i < locks.size(); i++) {
      locks[i]->lock();
      try {
        optional<int64_t> value = values[i];
        shared_ptr<TopDocs> topDocs =
            (make_shared<IndexSearcher>(reader))
                ->search(make_shared<TermQuery>(
                             make_shared<Term>(L"id", L"" + to_wstring(i))),
                         10);
        TestUtil::assertEquals(topDocs->totalHits, 1);
        int docID = topDocs->scoreDocs[0]->doc;
        deque<std::shared_ptr<LeafReaderContext>> leaves = reader.leaves();
        int subIndex = ReaderUtil::subIndex(docID, leaves);
        shared_ptr<LeafReader> leafReader = leaves[subIndex]->reader();
        docID -= leaves[subIndex]->docBase;
        shared_ptr<NumericDocValues> numericDocValues =
            leafReader->getNumericDocValues(L"value");
        if (!value) {
          assertFalse(L"docID: " + to_wstring(docID),
                      numericDocValues->advanceExact(docID));
        } else {
          assertTrue(L"docID: " + to_wstring(docID),
                     numericDocValues->advanceExact(docID));
          TestUtil::assertEquals(numericDocValues->longValue(), value.value());
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        locks[i]->unlock();
      }
    }
  }

  IOUtils::close({writer, dir});
}

void TestMixedDocValuesUpdates::doUpdate(
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
          (make_shared<IndexSearcher>(reader))
              ->search(make_shared<TermQuery>(doc), 10);
      TestUtil::assertEquals(1, topDocs->totalHits);
      int theDoc = topDocs->scoreDocs[0]->doc;
      seqId = writer->tryUpdateDocValue(reader, theDoc, {fields});
    }
  } while (seqId == -1);
}

void TestMixedDocValuesUpdates::testResetValue() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"id", L"0", Field::Store::NO));
  doc->push_back(make_shared<NumericDocValuesField>(L"val", 5));
  doc->push_back(make_shared<BinaryDocValuesField>(
      L"val-bin",
      make_shared<BytesRef>(std::deque<char>{static_cast<char>(5)})));
  writer->addDocument(doc);

  if (random()->nextBoolean()) {
    writer->commit();
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(DirectoryReader reader = writer.getReader())
  {
    DirectoryReader reader = writer->getReader();
    TestUtil::assertEquals(1, reader.leaves()->size());
    shared_ptr<LeafReader> r = reader.leaves()->get(0).reader();
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"val");
    TestUtil::assertEquals(0, ndv->nextDoc());
    TestUtil::assertEquals(5, ndv->longValue());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, ndv->nextDoc());

    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"val-bin");
    TestUtil::assertEquals(0, bdv->nextDoc());
    TestUtil::assertEquals(
        make_shared<BytesRef>(std::deque<char>{static_cast<char>(5)}),
        bdv->binaryValue());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, bdv->nextDoc());
  }

  writer->updateDocValues(
      make_shared<Term>(L"id", L"0"),
      {make_shared<BinaryDocValuesField>(L"val-bin", nullptr)});
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(DirectoryReader reader = writer.getReader())
  {
    DirectoryReader reader = writer->getReader();
    TestUtil::assertEquals(1, reader.leaves()->size());
    shared_ptr<LeafReader> r = reader.leaves()->get(0).reader();
    shared_ptr<NumericDocValues> ndv = r->getNumericDocValues(L"val");
    TestUtil::assertEquals(0, ndv->nextDoc());
    TestUtil::assertEquals(5, ndv->longValue());
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, ndv->nextDoc());

    shared_ptr<BinaryDocValues> bdv = r->getBinaryDocValues(L"val-bin");
    TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, bdv->nextDoc());
  }
  IOUtils::close({writer, dir});
}

void TestMixedDocValuesUpdates::testResetValueMultipleDocs() throw(
    runtime_error)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  int numDocs = 10 + random()->nextInt(50);
  int currentSeqId = 0;
  std::deque<int> seqId = {-1, -1, -1, -1, -1};
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int id = random()->nextInt(5);
    seqId[id] = currentSeqId;
    doc->push_back(make_shared<StringField>(L"id", L"" + to_wstring(id),
                                            Field::Store::YES));
    doc->push_back(
        make_shared<NumericDocValuesField>(L"seqID", currentSeqId++));
    doc->push_back(make_shared<NumericDocValuesField>(L"is_live", 1));
    if (i > 0) {
      writer->updateDocValues(
          make_shared<Term>(L"id", L"" + to_wstring(id)),
          {make_shared<NumericDocValuesField>(L"is_live", nullptr)});
    }
    writer->addDocument(doc);
    if (random()->nextBoolean()) {
      writer->flush();
    }
  }

  if (random()->nextBoolean()) {
    writer->commit();
  }
  int numHits = 0; // check if every doc has been selected at least once
  for (auto i : seqId) {
    if (i > -1) {
      numHits++;
    }
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(DirectoryReader reader = writer.getReader())
  {
    DirectoryReader reader = writer->getReader();
    shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

    shared_ptr<TopDocs> is_live =
        searcher->search(make_shared<DocValuesFieldExistsQuery>(L"is_live"), 5);
    TestUtil::assertEquals(numHits, is_live->totalHits);
    for (auto doc : is_live->scoreDocs) {
      int id = static_cast<Integer>(reader.document(doc->doc)->get(L"id"));
      int i = ReaderUtil::subIndex(doc->doc, reader.leaves());
      assertTrue(i >= 0);
      shared_ptr<LeafReaderContext> leafReaderContext = reader.leaves()->get(i);
      shared_ptr<NumericDocValues> seqID =
          leafReaderContext->reader()->getNumericDocValues(L"seqID");
      assertNotNull(seqID);
      assertTrue(seqID->advanceExact(doc->doc - leafReaderContext->docBase));
      TestUtil::assertEquals(seqId[id], seqID->longValue());
    }
  }
  IOUtils::close({writer, dir});
}

void TestMixedDocValuesUpdates::testUpdateNotExistingFieldDV() throw(
    IOException)
{
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); IndexWriter writer = new IndexWriter(dir, conf))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    IndexWriter writer = IndexWriter(dir, conf);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
    doc->push_back(make_shared<NumericDocValuesField>(L"test", 1));
    writer->addDocument(doc);
    if (random()->nextBoolean()) {
      writer->commit();
    }
    writer->updateDocValues(
        make_shared<Term>(L"id", L"1"),
        {make_shared<NumericDocValuesField>(L"not_existing", 1)});

    shared_ptr<Document> doc1 = make_shared<Document>();
    doc1->push_back(make_shared<StringField>(L"id", L"2", Field::Store::YES));
    doc1->push_back(make_shared<BinaryDocValuesField>(L"not_existing",
                                                      make_shared<BytesRef>()));
    invalid_argument iae = expectThrows(invalid_argument::typeid,
                                        [&]() { writer->addDocument(doc1); });
    TestUtil::assertEquals(L"cannot change DocValues type from NUMERIC to "
                           L"BINARY for field \"not_existing\"",
                           iae.what());

    iae = expectThrows(invalid_argument::typeid, [&]() {
      writer->updateDocValues(
          make_shared<Term>(L"id", L"1"),
          {make_shared<org::apache::lucene::document::BinaryDocValuesField>(
              L"not_existing",
              make_shared<org::apache::lucene::util::BytesRef>())});
    });
    TestUtil::assertEquals(L"cannot change DocValues type from NUMERIC to "
                           L"BINARY for field \"not_existing\"",
                           iae.what());
  }
}

void TestMixedDocValuesUpdates::testUpdateFieldWithNoPreviousDocValues() throw(
    IOException)
{
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.Directory dir =
  // newDirectory(); IndexWriter writer = new IndexWriter(dir, conf))
  {
    org::apache::lucene::store::Directory dir = newDirectory();
    IndexWriter writer = IndexWriter(dir, conf);
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"id", L"1", Field::Store::YES));
    writer->addDocument(doc);
    if (random()->nextBoolean()) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try (DirectoryReader reader =
      // writer.getReader())
      {
        DirectoryReader reader = writer->getReader();
        shared_ptr<NumericDocValues> id =
            reader.leaves()->get(0).reader().getNumericDocValues(L"id");
        assertNull(id);
      }
    } else if (random()->nextBoolean()) {
      writer->commit();
    }
    writer->updateDocValues(make_shared<Term>(L"id", L"1"),
                            {make_shared<NumericDocValuesField>(L"id", 1)});
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (DirectoryReader reader =
    // writer.getReader())
    {
      DirectoryReader reader = writer->getReader();
      shared_ptr<NumericDocValues> id =
          reader.leaves()->get(0).reader().getNumericDocValues(L"id");
      assertNotNull(id);
      assertTrue(id->advanceExact(0));
      TestUtil::assertEquals(1, id->longValue());
    }
  }
}
} // namespace org::apache::lucene::index