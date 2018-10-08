using namespace std;

#include "TestIndexTooManyDocs.h"

namespace org::apache::lucene::index
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestIndexTooManyDocs::testIndexTooManyDocs() throw(IOException,
                                                        InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  int numMaxDoc = 25;
  shared_ptr<IndexWriterConfig> config = make_shared<IndexWriterConfig>();
  config->setRAMBufferSizeMB(
      0.000001); // force lots of small segments and logs of concurrent deletes
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, config);
  try {
    IndexWriter::setMaxDocs(numMaxDoc);
    int numThreads = 5 + random()->nextInt(5);
    std::deque<std::shared_ptr<Thread>> threads(numThreads);
    shared_ptr<CountDownLatch> latch = make_shared<CountDownLatch>(numThreads);
    shared_ptr<CountDownLatch> indexingDone =
        make_shared<CountDownLatch>(numThreads - 2);
    shared_ptr<AtomicBoolean> done = make_shared<AtomicBoolean>(false);
    for (int i = 0; i < numThreads; i++) {
      if (i >= 2) {
        threads[i] = make_shared<Thread>([&]() {
          latch->countDown();
          try {
            latch->await();
          } catch (const InterruptedException &e) {
            throw make_shared<AssertionError>(e);
          }
          for (int d = 0; d < 100; d++) {
            shared_ptr<Document> doc = make_shared<Document>();
            wstring id = Integer::toString(random()->nextInt(numMaxDoc * 2));
            doc->add(make_shared<StringField>(L"id", id, Field::Store::NO));
            try {
              shared_ptr<Term> t = make_shared<Term>(L"id", id);
              if (random()->nextInt(5) == 0) {
                writer->deleteDocuments({make_shared<TermQuery>(t)});
              }
              writer->updateDocument(t, doc);
            } catch (const IOException &e) {
              throw make_shared<AssertionError>(e);
            } catch (const invalid_argument &e) {
              assertEquals(L"number of documents in the index cannot exceed " +
                               to_wstring(IndexWriter::getActualMaxDocs()),
                           e.what());
            }
          }
          indexingDone->countDown();
        });
      } else {
        threads[i] = make_shared<Thread>([&]() {
          try {
            latch->countDown();
            latch->await();
            shared_ptr<DirectoryReader> open =
                DirectoryReader::open(writer, true, true);
            while (done->get() == false) {
              shared_ptr<DirectoryReader> directoryReader =
                  DirectoryReader::openIfChanged(open);
              if (directoryReader != nullptr) {
                open->close();
                open = directoryReader;
              }
            }
            IOUtils::closeWhileHandlingException({open});
          } catch (const runtime_error &e) {
            throw make_shared<AssertionError>(e);
          }
        });
      }
      threads[i]->start();
    }

    indexingDone->await();
    done->set(true);

    for (int i = 0; i < numThreads; i++) {
      threads[i]->join();
    }
    delete writer;
    delete dir;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IndexWriter::setMaxDocs(IndexWriter::MAX_DOCS);
  }
}
} // namespace org::apache::lucene::index