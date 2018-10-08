using namespace std;

#include "ThreadedIndexingAndSearchingTestCase.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FailOnNonBulkMergesInfoStream =
    org::apache::lucene::util::FailOnNonBulkMergesInfoStream;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using PrintStreamInfoStream = org::apache::lucene::util::PrintStreamInfoStream;
using TestUtil = org::apache::lucene::util::TestUtil;

ThreadedIndexingAndSearchingTestCase::SubDocs::SubDocs(const wstring &packID,
                                                       deque<wstring> &subIDs)
    : packID(packID), subIDs(subIDs)
{
}

void ThreadedIndexingAndSearchingTestCase::releaseSearcher(
    shared_ptr<IndexSearcher> s) 
{
}

shared_ptr<Directory>
ThreadedIndexingAndSearchingTestCase::getDirectory(shared_ptr<Directory> in_)
{
  return in_;
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: protected void updateDocuments(Term id,
// java.util.List<? extends Iterable<? extends IndexableField>> docs) throws
// Exception
void ThreadedIndexingAndSearchingTestCase::updateDocuments(
    shared_ptr<Term> id, deque<T1> docs) 
{
  writer->updateDocuments(id, docs);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: protected void addDocuments(Term id,
// java.util.List<? extends Iterable<? extends IndexableField>> docs) throws
// Exception
void ThreadedIndexingAndSearchingTestCase::addDocuments(
    shared_ptr<Term> id, deque<T1> docs) 
{
  writer->addDocuments(docs);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: protected void addDocument(Term id, Iterable<?
// extends IndexableField> doc) throws Exception
void ThreadedIndexingAndSearchingTestCase::addDocument(
    shared_ptr<Term> id, deque<T1> doc) 
{
  writer->addDocument(doc);
}

template <typename T1>
// C++ TODO: There is no native C++ template equivalent to this generic
// constraint: ORIGINAL LINE: protected void updateDocument(Term term, Iterable<?
// extends IndexableField> doc) throws Exception
void ThreadedIndexingAndSearchingTestCase::updateDocument(
    shared_ptr<Term> term, deque<T1> doc) 
{
  writer->updateDocument(term, doc);
}

void ThreadedIndexingAndSearchingTestCase::deleteDocuments(
    shared_ptr<Term> term) 
{
  writer->deleteDocuments({term});
}

void ThreadedIndexingAndSearchingTestCase::doAfterIndexingThreadDone() {}

std::deque<std::shared_ptr<Thread>>
ThreadedIndexingAndSearchingTestCase::launchIndexingThreads(
    shared_ptr<LineFileDocs> docs, int numThreads, int64_t const stopTime,
    shared_ptr<Set<wstring>> delIDs, shared_ptr<Set<wstring>> delPackIDs,
    deque<std::shared_ptr<SubDocs>> &allSubDocs)
{
  std::deque<std::shared_ptr<Thread>> threads(numThreads);
  for (int thread = 0; thread < numThreads; thread++) {
    threads[thread] = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), docs, stopTime, delIDs, delPackIDs, allSubDocs);
    threads[thread]->start();
  }

  return threads;
}

ThreadedIndexingAndSearchingTestCase::ThreadAnonymousInnerClass::
    ThreadAnonymousInnerClass(
        shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        shared_ptr<LineFileDocs> docs, int64_t stopTime,
        shared_ptr<Set<wstring>> delIDs, shared_ptr<Set<wstring>> delPackIDs,
        deque<std::shared_ptr<SubDocs>> &allSubDocs)
{
  this->outerInstance = outerInstance;
  this->docs = docs;
  this->stopTime = stopTime;
  this->delIDs = delIDs;
  this->delPackIDs = delPackIDs;
  this->allSubDocs = allSubDocs;
}

void ThreadedIndexingAndSearchingTestCase::ThreadAnonymousInnerClass::run()
{
  // TODO: would be better if this were cross thread, so that we make sure one
  // thread deleting anothers added docs works:
  const deque<wstring> toDeleteIDs = deque<wstring>();
  const deque<std::shared_ptr<SubDocs>> toDeleteSubDocs =
      deque<std::shared_ptr<SubDocs>>();
  while (System::currentTimeMillis() < stopTime &&
         !outerInstance->failed->get()) {
    try {

      // Occasional longish pause if running
      // nightly
      if (LuceneTestCase::TEST_NIGHTLY &&
          LuceneTestCase::random()->nextInt(6) == 3) {
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L": now long sleep"
                << endl;
        }
        delay(TestUtil::nextInt(LuceneTestCase::random(), 50, 500));
      }

      // Rate limit ingest rate:
      if (LuceneTestCase::random()->nextInt(7) == 5) {
        delay(TestUtil::nextInt(LuceneTestCase::random(), 1, 10));
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L": done sleep" << endl;
        }
      }

      shared_ptr<Document> doc = docs->nextDoc();
      if (doc->empty()) {
        break;
      }

      // Maybe add randomly named field
      const wstring addedField;
      if (LuceneTestCase::random()->nextBoolean()) {
        addedField = L"extra" + LuceneTestCase::random()->nextInt(40);
        doc->push_back(LuceneTestCase::newTextField(
            addedField, L"a random field", Field::Store::YES));
      } else {
        addedField = L"";
      }

      if (LuceneTestCase::random()->nextBoolean()) {

        if (LuceneTestCase::random()->nextBoolean()) {
          // Add/update doc block:
          const wstring packID;
          shared_ptr<SubDocs> *const delSubDocs;
          if (toDeleteSubDocs.size() > 0 &&
              LuceneTestCase::random()->nextBoolean()) {
            delSubDocs = toDeleteSubDocs[LuceneTestCase::random()->nextInt(
                toDeleteSubDocs.size())];
            assert(!delSubDocs->deleted);
            // C++ TODO: The Java deque 'remove(Object)' method is not
            // converted:
            toDeleteSubDocs.remove(delSubDocs);
            // Update doc block, replacing prior packID
            packID = delSubDocs->packID;
          } else {
            delSubDocs.reset();
            // Add doc block, using new packID
            packID = outerInstance->packCount->getAndIncrement() + L"";
          }

          shared_ptr<Field> *const packIDField = LuceneTestCase::newStringField(
              L"packID", packID, Field::Store::YES);
          const deque<wstring> docIDs = deque<wstring>();
          shared_ptr<SubDocs> *const subDocs =
              make_shared<SubDocs>(packID, docIDs);
          const deque<std::shared_ptr<Document>> docsList =
              deque<std::shared_ptr<Document>>();

          allSubDocs.push_back(subDocs);
          doc->push_back(packIDField);
          docsList.push_back(TestUtil::cloneDocument(doc));
          docIDs.push_back(doc[L"docid"]);

          constexpr int maxDocCount =
              TestUtil::nextInt(LuceneTestCase::random(), 1, 10);
          while (docsList.size() < maxDocCount) {
            doc = docs->nextDoc();
            if (doc->empty()) {
              break;
            }
            docsList.push_back(TestUtil::cloneDocument(doc));
            docIDs.push_back(doc[L"docid"]);
          }
          outerInstance->addCount->addAndGet(docsList.size());

          shared_ptr<Term> *const packIDTerm =
              make_shared<Term>(L"packID", packID);

          if (delSubDocs != nullptr) {
            delSubDocs->deleted = true;
            delIDs->addAll(delSubDocs->subIDs);
            outerInstance->delCount->addAndGet(delSubDocs->subIDs.size());
            if (VERBOSE) {
              wcout << Thread::currentThread().getName()
                    << L": update pack packID=" << delSubDocs->packID
                    << L" count=" << docsList.size() << L" docs=" << docIDs
                    << endl;
            }
            outerInstance->updateDocuments(packIDTerm, docsList);
          } else {
            if (VERBOSE) {
              wcout << Thread::currentThread().getName()
                    << L": add pack packID=" << packID << L" count="
                    << docsList.size() << L" docs=" << docIDs << endl;
            }
            outerInstance->addDocuments(packIDTerm, docsList);
          }
          doc->removeField(L"packID");

          if (LuceneTestCase::random()->nextInt(5) == 2) {
            if (VERBOSE) {
              wcout << Thread::currentThread().getName() << L": buffer del id:"
                    << packID << endl;
            }
            toDeleteSubDocs.push_back(subDocs);
          }

        } else {
          // Add single doc
          const wstring docid = doc[L"docid"];
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L": add doc docid:"
                  << docid << endl;
          }
          outerInstance->addDocument(make_shared<Term>(L"docid", docid), doc);
          outerInstance->addCount->getAndIncrement();

          if (LuceneTestCase::random()->nextInt(5) == 3) {
            if (VERBOSE) {
              wcout << Thread::currentThread().getName() << L": buffer del id:"
                    << doc[L"docid"] << endl;
            }
            toDeleteIDs.push_back(docid);
          }
        }
      } else {

        // Update single doc, but we never re-use
        // and ID so the delete will never
        // actually happen:
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L": update doc id:"
                << doc[L"docid"] << endl;
        }
        const wstring docid = doc[L"docid"];
        outerInstance->updateDocument(make_shared<Term>(L"docid", docid), doc);
        outerInstance->addCount->getAndIncrement();

        if (LuceneTestCase::random()->nextInt(5) == 3) {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L": buffer del id:"
                  << doc[L"docid"] << endl;
          }
          toDeleteIDs.push_back(docid);
        }
      }

      if (LuceneTestCase::random()->nextInt(30) == 17) {
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L": apply "
                << toDeleteIDs.size() << L" deletes" << endl;
        }
        for (auto id : toDeleteIDs) {
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L": del term=id:"
                  << id << endl;
          }
          outerInstance->deleteDocuments(make_shared<Term>(L"docid", id));
        }
        constexpr int count =
            outerInstance->delCount->addAndGet(toDeleteIDs.size());
        if (VERBOSE) {
          wcout << Thread::currentThread().getName() << L": tot " << count
                << L" deletes" << endl;
        }
        delIDs->addAll(toDeleteIDs);
        toDeleteIDs.clear();

        for (auto subDocs : toDeleteSubDocs) {
          assert(!subDocs->deleted);
          delPackIDs->add(subDocs->packID);
          outerInstance->deleteDocuments(
              make_shared<Term>(L"packID", subDocs->packID));
          subDocs->deleted = true;
          if (VERBOSE) {
            wcout << Thread::currentThread().getName() << L": del subs: "
                  << subDocs->subIDs << L" packID=" << subDocs->packID << endl;
          }
          delIDs->addAll(subDocs->subIDs);
          outerInstance->delCount->addAndGet(subDocs->subIDs.size());
        }
        toDeleteSubDocs.clear();
      }
      if (addedField != L"") {
        doc->removeField(addedField);
      }
    } catch (const runtime_error &t) {
      wcout << Thread::currentThread().getName() << L": hit exc" << endl;
      t.printStackTrace();
      outerInstance->failed->set(true);
      throw runtime_error(t);
    }
  }
  if (VERBOSE) {
    wcout << Thread::currentThread().getName() << L": indexing done" << endl;
  }

  outerInstance->doAfterIndexingThreadDone();
}

void ThreadedIndexingAndSearchingTestCase::runSearchThreads(
    int64_t const stopTimeMS) 
{
  constexpr int numThreads = TestUtil::nextInt(random(), 1, 5);
  std::deque<std::shared_ptr<Thread>> searchThreads(numThreads);
  shared_ptr<AtomicLong> *const totHits = make_shared<AtomicLong>();

  // silly starting guess:
  shared_ptr<AtomicInteger> *const totTermCount =
      make_shared<AtomicInteger>(100);

  // TODO: we should enrich this to do more interesting searches
  for (int thread = 0; thread < searchThreads.size(); thread++) {
    searchThreads[thread] = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), stopTimeMS, totHits, totTermCount);
    searchThreads[thread]->start();
  }

  for (auto thread : searchThreads) {
    thread->join();
  }

  if (VERBOSE) {
    wcout << L"TEST: DONE search: totHits=" << totHits << endl;
  }
}

ThreadedIndexingAndSearchingTestCase::ThreadAnonymousInnerClass2::
    ThreadAnonymousInnerClass2(
        shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        int64_t stopTimeMS, shared_ptr<AtomicLong> totHits,
        shared_ptr<AtomicInteger> totTermCount)
{
  this->outerInstance = outerInstance;
  this->stopTimeMS = stopTimeMS;
  this->totHits = totHits;
  this->totTermCount = totTermCount;
}

void ThreadedIndexingAndSearchingTestCase::ThreadAnonymousInnerClass2::run()
{
  if (VERBOSE) {
    wcout << Thread::currentThread().getName() << L": launch search thread"
          << endl;
  }
  while (System::currentTimeMillis() < stopTimeMS &&
         !outerInstance->failed->get()) {
    try {
      shared_ptr<IndexSearcher> *const s = outerInstance->getCurrentSearcher();
      try {
        // Verify 1) IW is correctly setting
        // diagnostics, and 2) segment warming for
        // merged segments is actually happening:
        for (auto sub : s->getIndexReader()->leaves()) {
          shared_ptr<SegmentReader> segReader =
              std::static_pointer_cast<SegmentReader>(sub->reader());
          unordered_map<wstring, wstring> diagnostics =
              segReader->getSegmentInfo()->info->getDiagnostics();
          assertNotNull(diagnostics);
          wstring source = diagnostics[L"source"];
          assertNotNull(source);
          if (source == L"merge") {
            assertTrue(L"sub reader " + sub + L" wasn't warmed: warmed=" +
                           outerInstance->warmed + L" diagnostics=" +
                           diagnostics + L" si=" + segReader->getSegmentInfo(),
                       !outerInstance->assertMergedSegmentsWarmed ||
                           outerInstance->warmed.find(segReader->core) !=
                               outerInstance->warmed.end());
          }
        }
        if (s->getIndexReader()->numDocs() > 0) {
          outerInstance->smokeTestSearcher(s);
          shared_ptr<Terms> terms =
              MultiFields::getTerms(s->getIndexReader(), L"body");
          if (terms == nullptr) {
            continue;
          }
          shared_ptr<TermsEnum> termsEnum = terms->begin();
          int seenTermCount = 0;
          int shift;
          int trigger;
          if (totTermCount->get() < 30) {
            shift = 0;
            trigger = 1;
          } else {
            trigger = totTermCount->get() / 30;
            shift = LuceneTestCase::random()->nextInt(trigger);
          }
          while (System::currentTimeMillis() < stopTimeMS) {
            shared_ptr<BytesRef> term = termsEnum->next();
            if (term == nullptr) {
              totTermCount->set(seenTermCount);
              break;
            }
            seenTermCount++;
            // search 30 terms
            if ((seenTermCount + shift) % trigger == 0) {
              // if (VERBOSE) {
              // System.out.println(Thread.currentThread().getName() + " now
              // search body:" + term.utf8ToString());
              //}
              totHits->addAndGet(outerInstance->runQuery(
                  s, make_shared<TermQuery>(make_shared<Term>(
                         L"body", BytesRef::deepCopyOf(term)))));
            }
          }
          // if (VERBOSE) {
          // System.out.println(Thread.currentThread().getName() + ": search
          // done");
          //}
        }
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        outerInstance->releaseSearcher(s);
      }
    } catch (const runtime_error &t) {
      wcout << Thread::currentThread().getName() << L": hit exc" << endl;
      outerInstance->failed->set(true);
      t.printStackTrace(System::out);
      throw runtime_error(t);
    }
  }
}

void ThreadedIndexingAndSearchingTestCase::doAfterWriter(
    shared_ptr<ExecutorService> es) 
{
}

void ThreadedIndexingAndSearchingTestCase::doClose()  {}

void ThreadedIndexingAndSearchingTestCase::runTest(
    const wstring &testName) 
{

  failed->set(false);
  addCount->set(0);
  delCount->set(0);
  packCount->set(0);

  constexpr int64_t t0 = System::currentTimeMillis();

  shared_ptr<Random> random = make_shared<Random>(
      ThreadedIndexingAndSearchingTestCase::random()->nextLong());
  shared_ptr<LineFileDocs> *const docs = make_shared<LineFileDocs>(random);
  shared_ptr<Path> *const tempDir = createTempDir(testName);
  dir = getDirectory(
      newMockFSDirectory(tempDir)); // some subclasses rely on this being MDW
  if (std::dynamic_pointer_cast<BaseDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<BaseDirectoryWrapper>(dir))
        ->setCheckIndexOnClose(
            false); // don't double-checkIndex, we do it ourselves.
  }
  shared_ptr<MockAnalyzer> analyzer =
      make_shared<MockAnalyzer>(ThreadedIndexingAndSearchingTestCase::random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(ThreadedIndexingAndSearchingTestCase::random(), 1,
                        IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriterConfig> *const conf =
      newIndexWriterConfig(analyzer)->setCommitOnClose(false);
  conf->setInfoStream(make_shared<FailOnNonBulkMergesInfoStream>());
  if (std::dynamic_pointer_cast<MockRandomMergePolicy>(
          conf->getMergePolicy()) != nullptr) {
    (std::static_pointer_cast<MockRandomMergePolicy>(conf->getMergePolicy()))
        ->setDoNonBulkMerges(false);
  }

  if (LuceneTestCase::TEST_NIGHTLY) {
    // newIWConfig makes smallish max seg size, which
    // results in tons and tons of segments for this test
    // when run nightly:
    shared_ptr<MergePolicy> mp = conf->getMergePolicy();
    if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
      (std::static_pointer_cast<TieredMergePolicy>(mp))
          ->setMaxMergedSegmentMB(5000.0);
    } else if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) !=
               nullptr) {
      (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
          ->setMaxMergeMB(1000.0);
    } else if (std::dynamic_pointer_cast<LogMergePolicy>(mp) != nullptr) {
      (std::static_pointer_cast<LogMergePolicy>(mp))->setMaxMergeDocs(100000);
    }
    // when running nightly, merging can still have crazy parameters,
    // and might use many per-field codecs. turn on CFS for IW flushes
    // and ensure CFS ratio is reasonable to keep it contained.
    conf->setUseCompoundFile(true);
    mp->setNoCFSRatio(max(0.25, mp->getNoCFSRatio()));
  }

  conf->setMergedSegmentWarmer([&](reader) {
    if (VERBOSE) {
      wcout << L"TEST: now warm merged reader=" << reader << endl;
    }
    warmed.emplace((std::static_pointer_cast<SegmentReader>(reader))->core,
                   Boolean::TRUE);
    constexpr int maxDoc = reader::maxDoc();
    shared_ptr<Bits> *const liveDocs = reader::getLiveDocs();
    int sum = 0;
    constexpr int inc = max(1, maxDoc / 50);
    for (int docID = 0; docID < maxDoc; docID += inc) {
      if (liveDocs == nullptr || liveDocs->get(docID)) {
        shared_ptr<Document> *const doc = reader::document(docID);
        sum += doc->getFields().size();
      }
    }
    shared_ptr<IndexSearcher> searcher = newSearcher(reader, false);
    sum += searcher
               ->search(make_shared<TermQuery>(
                            make_shared<Term>(L"body", L"united")),
                        10)
               ->totalHits;
    if (VERBOSE) {
      wcout << L"TEST: warm visited " << sum << L" fields" << endl;
    }
  });

  if (VERBOSE) {
    conf->setInfoStream(make_shared<PrintStreamInfoStreamAnonymousInnerClass>(
        shared_from_this(), System::out));
  }
  writer = make_shared<IndexWriter>(dir, conf);
  TestUtil::reduceOpenFiles(writer);

  shared_ptr<ExecutorService> *const es =
      ThreadedIndexingAndSearchingTestCase::random()->nextBoolean()
          ? nullptr
          : Executors::newCachedThreadPool(
                make_shared<NamedThreadFactory>(testName));

  doAfterWriter(es);

  constexpr int NUM_INDEX_THREADS =
      TestUtil::nextInt(ThreadedIndexingAndSearchingTestCase::random(), 2, 4);

  constexpr int RUN_TIME_SEC =
      LuceneTestCase::TEST_NIGHTLY ? 300 : RANDOM_MULTIPLIER;

  shared_ptr<Set<wstring>> *const delIDs =
      Collections::synchronizedSet(unordered_set<wstring>());
  shared_ptr<Set<wstring>> *const delPackIDs =
      Collections::synchronizedSet(unordered_set<wstring>());
  const deque<std::shared_ptr<SubDocs>> allSubDocs =
      Collections::synchronizedList(deque<std::shared_ptr<SubDocs>>());

  constexpr int64_t stopTime =
      System::currentTimeMillis() + RUN_TIME_SEC * 1000;

  std::deque<std::shared_ptr<Thread>> indexThreads = launchIndexingThreads(
      docs, NUM_INDEX_THREADS, stopTime, delIDs, delPackIDs, allSubDocs);

  if (VERBOSE) {
    wcout << L"TEST: DONE start " << NUM_INDEX_THREADS << L" indexing threads ["
          << (System::currentTimeMillis() - t0) << L" ms]" << endl;
  }

  // Let index build up a bit
  delay(100);

  doSearching(es, stopTime);

  if (VERBOSE) {
    wcout << L"TEST: all searching done [" << (System::currentTimeMillis() - t0)
          << L" ms]" << endl;
  }

  for (auto thread : indexThreads) {
    thread->join();
  }

  if (VERBOSE) {
    wcout << L"TEST: done join indexing threads ["
          << (System::currentTimeMillis() - t0) << L" ms]; addCount="
          << addCount << L" delCount=" << delCount << endl;
  }

  shared_ptr<IndexSearcher> *const s = getFinalSearcher();
  if (VERBOSE) {
    wcout << L"TEST: finalSearcher=" << s << endl;
  }

  assertFalse(failed->get());

  bool doFail = false;

  // Verify: make sure delIDs are in fact deleted:
  for (auto id : delIDs) {
    shared_ptr<TopDocs> *const hits =
        s->search(make_shared<TermQuery>(make_shared<Term>(L"docid", id)), 1);
    if (hits->totalHits != 0) {
      wcout << L"doc id=" << id << L" is supposed to be deleted, but got "
            << hits->totalHits << L" hits; first docID="
            << hits->scoreDocs[0]->doc << endl;
      doFail = true;
    }
  }

  // Verify: make sure delPackIDs are in fact deleted:
  for (auto id : delPackIDs) {
    shared_ptr<TopDocs> *const hits =
        s->search(make_shared<TermQuery>(make_shared<Term>(L"packID", id)), 1);
    if (hits->totalHits != 0) {
      wcout << L"packID=" << id << L" is supposed to be deleted, but got "
            << hits->totalHits << L" matches" << endl;
      doFail = true;
    }
  }

  // Verify: make sure each group of sub-docs are still in docID order:
  for (auto subDocs : allSubDocs) {
    shared_ptr<TopDocs> hits = s->search(
        make_shared<TermQuery>(make_shared<Term>(L"packID", subDocs->packID)),
        20);
    if (!subDocs->deleted) {
      // We sort by relevance but the scores should be identical so sort falls
      // back to by docID:
      if (hits->totalHits != subDocs->subIDs.size()) {
        wcout << L"packID=" << subDocs->packID << L": expected "
              << subDocs->subIDs.size() << L" hits but got " << hits->totalHits
              << endl;
        doFail = true;
      } else {
        int lastDocID = -1;
        int startDocID = -1;
        for (auto scoreDoc : hits->scoreDocs) {
          constexpr int docID = scoreDoc->doc;
          if (lastDocID != -1) {
            TestUtil::assertEquals(1 + lastDocID, docID);
          } else {
            startDocID = docID;
          }
          lastDocID = docID;
          shared_ptr<Document> *const doc = s->doc(docID);
          TestUtil::assertEquals(subDocs->packID, doc[L"packID"]);
        }

        lastDocID = startDocID - 1;
        for (auto subID : subDocs->subIDs) {
          hits = s->search(
              make_shared<TermQuery>(make_shared<Term>(L"docid", subID)), 1);
          TestUtil::assertEquals(1, hits->totalHits);
          constexpr int docID = hits->scoreDocs[0]->doc;
          if (lastDocID != -1) {
            TestUtil::assertEquals(1 + lastDocID, docID);
          }
          lastDocID = docID;
        }
      }
    } else {
      // Pack was deleted -- make sure its docs are
      // deleted.  We can't verify packID is deleted
      // because we can re-use packID for update:
      for (auto subID : subDocs->subIDs) {
        TestUtil::assertEquals(
            0,
            s->search(
                 make_shared<TermQuery>(make_shared<Term>(L"docid", subID)), 1)
                ->totalHits);
      }
    }
  }

  // Verify: make sure all not-deleted docs are in fact
  // not deleted:
  constexpr int endID = static_cast<Integer>(docs->nextDoc()[L"docid"]);
  delete docs;

  for (int id = 0; id < endID; id++) {
    wstring stringID = L"" + to_wstring(id);
    if (!delIDs->contains(stringID)) {
      shared_ptr<TopDocs> *const hits = s->search(
          make_shared<TermQuery>(make_shared<Term>(L"docid", stringID)), 1);
      if (hits->totalHits != 1) {
        wcout << L"doc id=" << stringID
              << L" is not supposed to be deleted, but got hitCount="
              << hits->totalHits << L"; delIDs=" << delIDs << endl;
        doFail = true;
      }
    }
  }
  assertFalse(doFail);

  assertEquals(L"index=" + writer->segString() + L" addCount=" + addCount +
                   L" delCount=" + delCount,
               addCount->get() - delCount->get(),
               s->getIndexReader()->numDocs());
  releaseSearcher(s);

  writer->commit();

  assertEquals(L"index=" + writer->segString() + L" addCount=" + addCount +
                   L" delCount=" + delCount,
               addCount->get() - delCount->get(), writer->numDocs());

  doClose();

  try {
    writer->commit();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete writer;
  }

  // Cannot close until after writer is closed because
  // writer has merged segment warmer that uses IS to run
  // searches, and that IS may be using this es!
  if (es != nullptr) {
    es->shutdown();
    es->awaitTermination(1, TimeUnit::SECONDS);
  }

  TestUtil::checkIndex(dir);
  delete dir;

  if (VERBOSE) {
    wcout << L"TEST: done [" << (System::currentTimeMillis() - t0) << L" ms]"
          << endl;
  }
}

ThreadedIndexingAndSearchingTestCase::PrintStreamInfoStreamAnonymousInnerClass::
    PrintStreamInfoStreamAnonymousInnerClass(
        shared_ptr<ThreadedIndexingAndSearchingTestCase> outerInstance,
        shared_ptr<UnknownType> out)
    : org::apache::lucene::util::PrintStreamInfoStream(out)
{
  this->outerInstance = outerInstance;
}

void ThreadedIndexingAndSearchingTestCase::
    PrintStreamInfoStreamAnonymousInnerClass::message(const wstring &component,
                                                      const wstring &message)
{
  if (L"TP" == component) {
    return; // ignore test points!
  }
  outerInstance->super->message(component, message);
}

int64_t ThreadedIndexingAndSearchingTestCase::runQuery(
    shared_ptr<IndexSearcher> s, shared_ptr<Query> q) 
{
  s->search(q, 10);
  int64_t hitCount = s->search(q, 10,
                                 make_shared<Sort>(make_shared<SortField>(
                                     L"titleDV", SortField::Type::STRING)))
                           ->totalHits;
  shared_ptr<Sort> *const dvSort = make_shared<Sort>(
      make_shared<SortField>(L"titleDV", SortField::Type::STRING));
  int64_t hitCount2 = s->search(q, 10, dvSort)->totalHits;
  TestUtil::assertEquals(hitCount, hitCount2);
  return hitCount;
}

void ThreadedIndexingAndSearchingTestCase::smokeTestSearcher(
    shared_ptr<IndexSearcher> s) 
{
  runQuery(s, make_shared<TermQuery>(make_shared<Term>(L"body", L"united")));
  runQuery(s, make_shared<TermQuery>(
                  make_shared<Term>(L"titleTokenized", L"states")));
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"body", L"united", L"states");
  runQuery(s, pq);
}
} // namespace org::apache::lucene::index