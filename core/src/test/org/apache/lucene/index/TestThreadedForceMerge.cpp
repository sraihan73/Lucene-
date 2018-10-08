using namespace std;

#include "TestThreadedForceMerge.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Directory = org::apache::lucene::store::Directory;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using StringField = org::apache::lucene::document::StringField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using English = org::apache::lucene::util::English;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::analysis::Analyzer>
    TestThreadedForceMerge::ANALYZER;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void setup()
void TestThreadedForceMerge::setup()
{
  ANALYZER = make_shared<MockAnalyzer>(random(), MockTokenizer::SIMPLE, true);
}

void TestThreadedForceMerge::setFailed() { failed = true; }

void TestThreadedForceMerge::runTest(
    shared_ptr<Random> random,
    shared_ptr<Directory> directory) 
{

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      directory, newIndexWriterConfig(ANALYZER)
                     ->setOpenMode(OpenMode::CREATE)
                     ->setMaxBufferedDocs(2)
                     ->setMergePolicy(newLogMergePolicy()));

  for (int iter = 0; iter < NUM_ITER; iter++) {
    constexpr int iterFinal = iter;

    (std::static_pointer_cast<LogMergePolicy>(
         writer->getConfig()->getMergePolicy()))
        ->setMergeFactor(1000);

    shared_ptr<FieldType> *const customType =
        make_shared<FieldType>(StringField::TYPE_STORED);
    customType->setOmitNorms(true);

    for (int i = 0; i < 200; i++) {
      shared_ptr<Document> d = make_shared<Document>();
      // C++ TODO: There is no native C++ equivalent to 'toString':
      d->push_back(newField(L"id", Integer::toString(i), customType));
      d->push_back(newField(L"contents", English::intToEnglish(i), customType));
      writer->addDocument(d);
    }

    (std::static_pointer_cast<LogMergePolicy>(
         writer->getConfig()->getMergePolicy()))
        ->setMergeFactor(4);

    std::deque<std::shared_ptr<Thread>> threads(NUM_THREADS);

    for (int i = 0; i < NUM_THREADS; i++) {
      constexpr int iFinal = i;
      shared_ptr<IndexWriter> *const writerFinal = writer;
      threads[i] = make_shared<ThreadAnonymousInnerClass>(
          shared_from_this(), iterFinal, customType, iFinal, writerFinal);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i]->start();
    }

    for (int i = 0; i < NUM_THREADS; i++) {
      threads[i]->join();
    }

    assertTrue(!failed);

    constexpr int expectedDocCount = static_cast<int>(
        (1 + iter) *
        (200 + 8 * NUM_ITER2 * (NUM_THREADS / 2.0) * (1 + NUM_THREADS)));

    assertEquals(L"index=" + writer->segString() + L" numDocs=" +
                     to_wstring(writer->numDocs()) + L" maxDoc=" +
                     to_wstring(writer->maxDoc()) + L" config=" +
                     writer->getConfig(),
                 expectedDocCount, writer->numDocs());
    assertEquals(L"index=" + writer->segString() + L" numDocs=" +
                     to_wstring(writer->numDocs()) + L" maxDoc=" +
                     to_wstring(writer->maxDoc()) + L" config=" +
                     writer->getConfig(),
                 expectedDocCount, writer->maxDoc());

    delete writer;
    writer =
        make_shared<IndexWriter>(directory, newIndexWriterConfig(ANALYZER)
                                                ->setOpenMode(OpenMode::APPEND)
                                                ->setMaxBufferedDocs(2));

    shared_ptr<DirectoryReader> reader = DirectoryReader::open(directory);
    assertEquals(L"reader=" + reader, 1, reader->leaves()->size());
    assertEquals(expectedDocCount, reader->numDocs());
    reader->close();
  }
  delete writer;
}

TestThreadedForceMerge::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestThreadedForceMerge> outerInstance, int iterFinal,
    shared_ptr<FieldType> customType, int iFinal,
    shared_ptr<org::apache::lucene::index::IndexWriter> writerFinal)
{
  this->outerInstance = outerInstance;
  this->iterFinal = iterFinal;
  this->customType = customType;
  this->iFinal = iFinal;
  this->writerFinal = writerFinal;
}

void TestThreadedForceMerge::ThreadAnonymousInnerClass::run()
{
  try {
    for (int j = 0; j < NUM_ITER2; j++) {
      writerFinal->forceMerge(1, false);
      for (int k = 0; k < 17 * (1 + iFinal); k++) {
        shared_ptr<Document> d = make_shared<Document>();
        d->push_back(LuceneTestCase::newField(
            L"id",
            to_wstring(iterFinal) + L"_" + to_wstring(iFinal) + L"_" +
                to_wstring(j) + L"_" + to_wstring(k),
            customType));
        d->push_back(LuceneTestCase::newField(
            L"contents", English::intToEnglish(iFinal + k), customType));
        writerFinal->addDocument(d);
      }
      for (int k = 0; k < 9 * (1 + iFinal); k++) {
        writerFinal->deleteDocuments({make_shared<Term>(
            L"id", to_wstring(iterFinal) + L"_" + to_wstring(iFinal) + L"_" +
                       to_wstring(j) + L"_" + to_wstring(k))});
      }
      writerFinal->forceMerge(1);
    }
  } catch (const runtime_error &t) {
    outerInstance->setFailed();
    wcout << Thread::currentThread().getName() << L": hit exception" << endl;
    t.printStackTrace(System::out);
  }
}

void TestThreadedForceMerge::testThreadedForceMerge() 
{
  shared_ptr<Directory> directory = newDirectory();
  runTest(random(), directory);
  delete directory;
}
} // namespace org::apache::lucene::index