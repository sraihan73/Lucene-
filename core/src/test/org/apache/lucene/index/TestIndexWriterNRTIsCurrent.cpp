using namespace std;

#include "TestIndexWriterNRTIsCurrent.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexWriterNRTIsCurrent::testIsCurrentWithThreads() throw(
    IOException, InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);
  shared_ptr<ReaderHolder> holder = make_shared<ReaderHolder>();
  std::deque<std::shared_ptr<ReaderThread>> threads(atLeast(3));
  shared_ptr<CountDownLatch> *const latch = make_shared<CountDownLatch>(1);
  shared_ptr<WriterThread> writerThread =
      make_shared<WriterThread>(holder, writer, atLeast(500), random(), latch);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<ReaderThread>(holder, latch);
    threads[i]->start();
  }
  writerThread->start();

  writerThread->join();
  bool failed = writerThread->failed != nullptr;
  if (failed) {
    writerThread->failed.printStackTrace();
  }
  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
    if (threads[i]->failed != nullptr) {
      threads[i]->failed.printStackTrace();
      failed = true;
    }
  }
  assertFalse(failed);
  delete writer;
  delete dir;
}

TestIndexWriterNRTIsCurrent::WriterThread::WriterThread(
    shared_ptr<ReaderHolder> holder, shared_ptr<IndexWriter> writer, int numOps,
    shared_ptr<Random> random, shared_ptr<CountDownLatch> latch)
    : Thread(), holder(holder), writer(writer), numOps(numOps), latch(latch)
{
}

void TestIndexWriterNRTIsCurrent::WriterThread::run()
{
  shared_ptr<DirectoryReader> currentReader = nullptr;
  shared_ptr<Random> random = LuceneTestCase::random();
  try {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<TextField>(L"id", L"1", Field::Store::NO));
    writer->addDocument(doc);
    holder->reader = currentReader = writer->getReader();
    shared_ptr<Term> term = make_shared<Term>(L"id");
    for (int i = 0; i < numOps && !holder->stop; i++) {
      float nextOp = random->nextFloat();
      if (nextOp < 0.3) {
        term->set(L"id", make_shared<BytesRef>(L"1"));
        writer->updateDocument(term, doc);
      } else if (nextOp < 0.5) {
        writer->addDocument(doc);
      } else {
        term->set(L"id", make_shared<BytesRef>(L"1"));
        writer->deleteDocuments({term});
      }
      if (holder->reader != currentReader) {
        holder->reader = currentReader;
        if (countdown) {
          countdown = false;
          latch->countDown();
        }
      }
      if (random->nextBoolean()) {
        writer->commit();
        shared_ptr<DirectoryReader> *const newReader =
            DirectoryReader::openIfChanged(currentReader);
        if (newReader != nullptr) {
          currentReader->decRef();
          currentReader = newReader;
        }
        if (currentReader->numDocs() == 0) {
          writer->addDocument(doc);
        }
      }
    }
  } catch (const runtime_error &e) {
    failed = e;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    holder->reader.reset();
    if (countdown) {
      latch->countDown();
    }
    if (currentReader != nullptr) {
      try {
        currentReader->decRef();
      } catch (const IOException &e) {
      }
    }
  }
  if (VERBOSE) {
    wcout << L"writer stopped - forced by reader: " << holder->stop << endl;
  }
}

TestIndexWriterNRTIsCurrent::ReaderThread::ReaderThread(
    shared_ptr<ReaderHolder> holder, shared_ptr<CountDownLatch> latch)
    : Thread(), holder(holder), latch(latch)
{
}

void TestIndexWriterNRTIsCurrent::ReaderThread::run()
{
  try {
    latch->await();
  } catch (const InterruptedException &e) {
    failed = e;
    return;
  }
  shared_ptr<DirectoryReader> reader;
  while ((reader = holder->reader) != nullptr) {
    if (reader->tryIncRef()) {
      try {
        bool current = reader->isCurrent();
        if (VERBOSE) {
          wcout << L"Thread: " << Thread::currentThread() << L" Reader: "
                << reader << L" isCurrent:" << current << endl;
        }

        assertFalse(current);
      } catch (const runtime_error &e) {
        if (VERBOSE) {
          wcout << L"FAILED Thread: " << Thread::currentThread() << L" Reader: "
                << reader << L" isCurrent: false" << endl;
        }
        failed = e;
        holder->stop = true;
        return;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        try {
          reader->decRef();
        } catch (const IOException &e) {
          if (failed == nullptr) {
            failed = e;
          }
        }
      }
    }
  }
}
} // namespace org::apache::lucene::index