using namespace std;

#include "TestStressNRT.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestStressNRT::initModel(int ndocs)
{
  snapshotCount = 0;
  committedModelClock = 0;
  lastId = 0;

  syncArr = std::deque<any>(ndocs);

  for (int i = 0; i < ndocs; i++) {
    model->put(i, -1LL);
    syncArr[i] = any();
  }
  committedModel.putAll(model);
}

void TestStressNRT::test() 
{
  // update variables
  constexpr int commitPercent = random()->nextInt(20);
  constexpr int softCommitPercent =
      random()->nextInt(100); // what percent of the commits are soft
  constexpr int deletePercent = random()->nextInt(50);
  constexpr int deleteByQueryPercent = random()->nextInt(25);
  constexpr int ndocs = atLeast(50);
  constexpr int nWriteThreads =
      TestUtil::nextInt(random(), 1, TEST_NIGHTLY ? 10 : 5);
  constexpr int maxConcurrentCommits = TestUtil::nextInt(
      random(), 1,
      TEST_NIGHTLY ? 10
                   : 5); // number of committers at a time... needed if we want
                         // to avoid commit errors due to exceeding the max
  constexpr bool useSoftDeletes = random()->nextInt(10) < 3;

  constexpr bool tombstones = random()->nextBoolean();

  // query variables
  shared_ptr<AtomicLong> *const operations = make_shared<AtomicLong>(
      atLeast(10000)); // number of query operations to perform in total

  constexpr int nReadThreads =
      TestUtil::nextInt(random(), 1, TEST_NIGHTLY ? 10 : 5);
  initModel(ndocs);

  shared_ptr<FieldType> *const storedOnlyType = make_shared<FieldType>();
  storedOnlyType->setStored(true);

  if (VERBOSE) {
    wcout << L"\n" << endl;
    wcout << L"TEST: commitPercent=" << commitPercent << endl;
    wcout << L"TEST: softCommitPercent=" << softCommitPercent << endl;
    wcout << L"TEST: deletePercent=" << deletePercent << endl;
    wcout << L"TEST: deleteByQueryPercent=" << deleteByQueryPercent << endl;
    wcout << L"TEST: ndocs=" << ndocs << endl;
    wcout << L"TEST: nWriteThreads=" << nWriteThreads << endl;
    wcout << L"TEST: nReadThreads=" << nReadThreads << endl;
    wcout << L"TEST: maxConcurrentCommits=" << maxConcurrentCommits << endl;
    wcout << L"TEST: tombstones=" << tombstones << endl;
    wcout << L"TEST: operations=" << operations << endl;
    wcout << L"\n" << endl;
  }

  shared_ptr<AtomicInteger> *const numCommitting = make_shared<AtomicInteger>();

  deque<std::shared_ptr<Thread>> threads = deque<std::shared_ptr<Thread>>();

  shared_ptr<Directory> dir = newMaybeVirusCheckingDirectory();

  shared_ptr<RandomIndexWriter> *const writer = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())),
      useSoftDeletes);
  writer->setDoRandomForceMergeAssert(false);
  writer->commit();
  if (useSoftDeletes) {
    reader = make_shared<SoftDeletesDirectoryReaderWrapper>(
        DirectoryReader::open(dir),
        writer->w->getConfig()->getSoftDeletesField());
  } else {
    reader = DirectoryReader::open(dir);
  }

  for (int i = 0; i < nWriteThreads; i++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass>(
        shared_from_this(), L"WRITER" + to_wstring(i), commitPercent,
        softCommitPercent, deletePercent, deleteByQueryPercent, ndocs,
        maxConcurrentCommits, tombstones, operations, storedOnlyType,
        numCommitting, writer);

    threads.push_back(thread);
  }

  for (int i = 0; i < nReadThreads; i++) {
    shared_ptr<Thread> thread = make_shared<ThreadAnonymousInnerClass2>(
        shared_from_this(), L"READER" + to_wstring(i), ndocs, tombstones,
        operations);

    threads.push_back(thread);
  }

  for (auto thread : threads) {
    thread->start();
  }

  for (auto thread : threads) {
    thread->join();
  }

  delete writer;
  if (VERBOSE) {
    wcout << L"TEST: close reader=" << reader << endl;
  }
  reader->close();
  delete dir;
}

TestStressNRT::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestStressNRT> outerInstance, wstring L"WRITER" + i,
    int commitPercent, int softCommitPercent, int deletePercent,
    int deleteByQueryPercent, int ndocs, int maxConcurrentCommits,
    bool tombstones, shared_ptr<AtomicLong> operations,
    shared_ptr<FieldType> storedOnlyType,
    shared_ptr<AtomicInteger> numCommitting,
    shared_ptr<org::apache::lucene::index::RandomIndexWriter> writer)
    : Thread(L"WRITER" + i)
{
  this->outerInstance = outerInstance;
  this->commitPercent = commitPercent;
  this->softCommitPercent = softCommitPercent;
  this->deletePercent = deletePercent;
  this->deleteByQueryPercent = deleteByQueryPercent;
  this->ndocs = ndocs;
  this->maxConcurrentCommits = maxConcurrentCommits;
  this->tombstones = tombstones;
  this->operations = operations;
  this->storedOnlyType = storedOnlyType;
  this->numCommitting = numCommitting;
  this->writer = writer;
  rand = make_shared<Random>(LuceneTestCase::random()->nextInt());
}

void TestStressNRT::ThreadAnonymousInnerClass::run()
{
  try {
    while (operations->get() > 0) {
      int oper = rand::nextInt(100);

      if (oper < commitPercent) {
        if (numCommitting->incrementAndGet() <= maxConcurrentCommits) {
          unordered_map<int, int64_t> newCommittedModel;
          int64_t version;
          shared_ptr<DirectoryReader> oldReader;

          {
            lock_guard<mutex> lock(outerInstance);
            newCommittedModel =
                unordered_map<>(outerInstance->model); // take a snapshot
            version = outerInstance->snapshotCount++;
            oldReader = outerInstance->reader;
            oldReader->incRef(); // increment the reference since we will use
                                 // this for reopening
          }

          shared_ptr<DirectoryReader> newReader;
          if (rand::nextInt(100) < softCommitPercent) {
            // assertU(h.commit("softCommit","true"));
            if (LuceneTestCase::random()->nextBoolean()) {
              if (VERBOSE) {
                wcout << L"TEST: " << Thread::currentThread().getName()
                      << L": call writer.getReader" << endl;
              }
              newReader = writer->getReader();
            } else {
              if (VERBOSE) {
                wcout << L"TEST: " << Thread::currentThread().getName()
                      << L": reopen reader=" << oldReader << L" version="
                      << version << endl;
              }
              newReader = DirectoryReader::openIfChanged(oldReader, writer->w);
            }
          } else {
            // assertU(commit());
            if (VERBOSE) {
              wcout << L"TEST: " << Thread::currentThread().getName()
                    << L": commit+reopen reader=" << oldReader << L" version="
                    << version << endl;
            }
            writer->commit();
            if (VERBOSE) {
              wcout << L"TEST: " << Thread::currentThread().getName()
                    << L": now reopen after commit" << endl;
            }
            newReader = DirectoryReader::openIfChanged(oldReader);
          }

          // Code below assumes newReader comes w/
          // extra ref:
          if (newReader == nullptr) {
            oldReader->incRef();
            newReader = oldReader;
          }

          oldReader->decRef();

          {
            lock_guard<mutex> lock(outerInstance);
            // install the new reader if it's newest (and check the current
            // version since another reader may have already been installed)
            // System.out.println(Thread.currentThread().getName() + ":
            // newVersion=" + newReader.getVersion());
            assert(newReader->getRefCount() > 0);
            assert(outerInstance->reader->getRefCount() > 0);
            if (newReader->getVersion() > outerInstance->reader->getVersion()) {
              if (VERBOSE) {
                wcout << L"TEST: " << Thread::currentThread().getName()
                      << L": install new reader=" << newReader << endl;
              }
              outerInstance->reader->decRef();
              outerInstance->reader = newReader;

              // Silly: forces fieldInfos to be
              // loaded so we don't hit IOE on later
              // reader.toString
              // C++ TODO: There is no native C++ equivalent to 'toString':
              newReader->toString();

              // install this snapshot only if it's newer than the current one
              if (version >= outerInstance->committedModelClock) {
                if (VERBOSE) {
                  wcout << L"TEST: " << Thread::currentThread().getName()
                        << L": install new model version=" << version << endl;
                }
                outerInstance->committedModel = newCommittedModel;
                outerInstance->committedModelClock = version;
              } else {
                if (VERBOSE) {
                  wcout << L"TEST: " << Thread::currentThread().getName()
                        << L": skip install new model version=" << version
                        << endl;
                }
              }
            } else {
              // if the same reader, don't decRef.
              if (VERBOSE) {
                wcout << L"TEST: " << Thread::currentThread().getName()
                      << L": skip install new reader=" << newReader << endl;
              }
              newReader->decRef();
            }
          }
        }
        numCommitting->decrementAndGet();
      } else {

        int id = rand::nextInt(ndocs);
        any sync = outerInstance->syncArr[id];

        // set the lastId before we actually change it sometimes to try and
        // uncover more race conditions between writing and reading
        bool before = LuceneTestCase::random()->nextBoolean();
        if (before) {
          outerInstance->lastId = id;
        }

        // We can't concurrently update the same document and retain our
        // invariants of increasing values since we can't guarantee what order
        // the updates will be executed.
        {
          lock_guard<mutex> lock(sync);
          optional<int64_t> val = outerInstance->model->get(id);
          int64_t nextVal = abs(val) + 1;

          if (oper < commitPercent + deletePercent) {
            // assertU("<delete><id>" + id + "</id></delete>");

            // add tombstone first
            if (tombstones) {
              shared_ptr<Document> d = make_shared<Document>();
              // C++ TODO: There is no native C++ equivalent to 'toString':
              d->push_back(LuceneTestCase::newStringField(
                  L"id", L"-" + Integer::toString(id), Field::Store::YES));
              // C++ TODO: There is no native C++ equivalent to 'toString':
              d->push_back(LuceneTestCase::newField(outerInstance->field,
                                                    Long::toString(nextVal),
                                                    storedOnlyType));
              // C++ TODO: There is no native C++ equivalent to 'toString':
              writer->updateDocument(
                  make_shared<Term>(L"id", L"-" + Integer::toString(id)), d);
            }

            if (VERBOSE) {
              wcout << L"TEST: " << Thread::currentThread().getName()
                    << L": term delDocs id:" << id << L" nextVal=" << nextVal
                    << endl;
            }
            // C++ TODO: There is no native C++ equivalent to 'toString':
            writer->deleteDocuments(
                make_shared<Term>(L"id", Integer::toString(id)));
            outerInstance->model->put(id, -nextVal);
          } else if (oper <
                     commitPercent + deletePercent + deleteByQueryPercent) {
            // assertU("<delete><query>id:" + id + "</query></delete>");

            // add tombstone first
            if (tombstones) {
              shared_ptr<Document> d = make_shared<Document>();
              // C++ TODO: There is no native C++ equivalent to 'toString':
              d->push_back(LuceneTestCase::newStringField(
                  L"id", L"-" + Integer::toString(id), Field::Store::YES));
              // C++ TODO: There is no native C++ equivalent to 'toString':
              d->push_back(LuceneTestCase::newField(outerInstance->field,
                                                    Long::toString(nextVal),
                                                    storedOnlyType));
              // C++ TODO: There is no native C++ equivalent to 'toString':
              writer->updateDocument(
                  make_shared<Term>(L"id", L"-" + Integer::toString(id)), d);
            }

            if (VERBOSE) {
              wcout << L"TEST: " << Thread::currentThread().getName()
                    << L": query delDocs id:" << id << L" nextVal=" << nextVal
                    << endl;
            }
            // C++ TODO: There is no native C++ equivalent to 'toString':
            writer->deleteDocuments(make_shared<TermQuery>(
                make_shared<Term>(L"id", Integer::toString(id))));
            outerInstance->model->put(id, -nextVal);
          } else {
            // assertU(adoc("id",Integer.toString(id), field,
            // Long.toString(nextVal)));
            shared_ptr<Document> d = make_shared<Document>();
            // C++ TODO: There is no native C++ equivalent to 'toString':
            d->push_back(LuceneTestCase::newStringField(
                L"id", Integer::toString(id), Field::Store::YES));
            // C++ TODO: There is no native C++ equivalent to 'toString':
            d->push_back(LuceneTestCase::newField(
                outerInstance->field, Long::toString(nextVal), storedOnlyType));
            if (VERBOSE) {
              wcout << L"TEST: " << Thread::currentThread().getName()
                    << L": u id:" << id << L" val=" << nextVal << endl;
            }
            // C++ TODO: There is no native C++ equivalent to 'toString':
            writer->updateDocument(
                make_shared<Term>(L"id", Integer::toString(id)), d);
            if (tombstones) {
              // remove tombstone after new addition (this should be optional?)
              // C++ TODO: There is no native C++ equivalent to 'toString':
              writer->deleteDocuments(
                  make_shared<Term>(L"id", L"-" + Integer::toString(id)));
            }
            outerInstance->model->put(id, nextVal);
          }
        }

        if (!before) {
          outerInstance->lastId = id;
        }
      }
    }
  } catch (const runtime_error &e) {
    wcout << Thread::currentThread().getName()
          << L": FAILED: unexpected exception" << endl;
    e.printStackTrace(System::out);
    throw runtime_error(e);
  }
}

TestStressNRT::ThreadAnonymousInnerClass2::ThreadAnonymousInnerClass2(
    shared_ptr<TestStressNRT> outerInstance, wstring L"READER" + i, int ndocs,
    bool tombstones, shared_ptr<AtomicLong> operations)
    : Thread(L"READER" + i)
{
  this->outerInstance = outerInstance;
  this->ndocs = ndocs;
  this->tombstones = tombstones;
  this->operations = operations;
  rand = make_shared<Random>(LuceneTestCase::random()->nextInt());
}

void TestStressNRT::ThreadAnonymousInnerClass2::run()
{
  try {
    shared_ptr<IndexReader> lastReader = nullptr;
    shared_ptr<IndexSearcher> lastSearcher = nullptr;

    while (operations->decrementAndGet() >= 0) {
      // bias toward a recently changed doc
      int id = rand::nextInt(100) < 25 ? outerInstance->lastId
                                       : rand::nextInt(ndocs);

      // when indexing, we update the index, then the model
      // so when querying, we should first check the model, and then the index

      int64_t val;
      shared_ptr<DirectoryReader> r;
      {
        lock_guard<mutex> lock(outerInstance);
        val = outerInstance->committedModel[id];
        r = outerInstance->reader;
        r->incRef();
      }

      if (VERBOSE) {
        wcout << L"TEST: " << Thread::currentThread().getName() << L": s id="
              << id << L" val=" << val << L" r=" << r->getVersion() << endl;
      }

      //  sreq = req("wt","json", "q","id:"+Integer.toString(id),
      //  "omitHeader","true");
      shared_ptr<IndexSearcher> searcher;
      if (r == lastReader) {
        // Just re-use lastSearcher, else
        // newSearcher may create too many thread
        // pools (ExecutorService):
        searcher = lastSearcher;
      } else {
        searcher = LuceneTestCase::newSearcher(r);
        lastReader = r;
        lastSearcher = searcher;
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      shared_ptr<Query> q = make_shared<TermQuery>(
          make_shared<Term>(L"id", Integer::toString(id)));
      shared_ptr<TopDocs> results = searcher->search(q, 10);

      if (results->totalHits == 0 && tombstones) {
        // if we couldn't find the doc, look for its tombstone
        // C++ TODO: There is no native C++ equivalent to 'toString':
        q = make_shared<TermQuery>(
            make_shared<Term>(L"id", L"-" + Integer::toString(id)));
        results = searcher->search(q, 1);
        if (results->totalHits == 0) {
          if (val == -1LL) {
            // expected... no doc was added yet
            r->decRef();
            continue;
          }
          fail(L"No documents or tombstones found for id " + to_wstring(id) +
               L", expected at least " + to_wstring(val) + L" reader=" + r);
        }
      }

      if (results->totalHits == 0 && !tombstones) {
        // nothing to do - we can't tell anything from a deleted doc without
        // tombstones
      } else {
        // we should have found the document, or its tombstone
        if (results->totalHits != 1) {
          wcout << L"FAIL: hits id:" << id << L" val=" << val << endl;
          for (auto sd : results->scoreDocs) {
            shared_ptr<Document> *const doc = r->document(sd->doc);
            wcout << L"  docID=" << sd->doc << L" id:" << doc[L"id"]
                  << L" foundVal=" << doc[outerInstance->field] << endl;
          }
          fail(L"id=" + to_wstring(id) + L" reader=" + r + L" totalHits=" +
               to_wstring(results->totalHits));
        }
        shared_ptr<Document> doc = searcher->doc(results->scoreDocs[0]->doc);
        int64_t foundVal = static_cast<int64_t>(doc[outerInstance->field]);
        if (foundVal < abs(val)) {
          fail(L"foundVal=" + to_wstring(foundVal) + L" val=" +
               to_wstring(val) + L" id=" + to_wstring(id) + L" reader=" + r);
        }
      }

      r->decRef();
    }
  } catch (const runtime_error &e) {
    operations->set(-1LL);
    wcout << Thread::currentThread().getName()
          << L": FAILED: unexpected exception" << endl;
    e.printStackTrace(System::out);
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::index