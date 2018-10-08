using namespace std;

#include "TestRAMDirectory.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using English = org::apache::lucene::util::English;
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<Directory> TestRAMDirectory::getDirectory(shared_ptr<Path> path)
{
  return make_shared<RAMDirectory>();
}

shared_ptr<Path> TestRAMDirectory::buildIndex() 
{
  shared_ptr<Path> path = createTempDir(L"buildIndex");

  shared_ptr<Directory> dir = newFSDirectory(path);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setOpenMode(OpenMode::CREATE));
  // add some documents
  shared_ptr<Document> doc;
  for (int i = 0; i < DOCS_TO_ADD; i++) {
    doc = make_shared<Document>();
    doc->push_back(newStringField(L"content",
                                  StringHelper::trim(English::intToEnglish(i)),
                                  Field::Store::YES));
    writer->addDocument(doc);
  }
  assertEquals(DOCS_TO_ADD, writer->maxDoc());
  delete writer;
  delete dir;

  return path;
}

void TestRAMDirectory::testCopySubdir() 
{
  shared_ptr<Path> path = createTempDir(L"testsubdir");
  shared_ptr<FSDirectory> fsDir = nullptr;
  try {
    Files::createDirectory(path->resolve(L"subdir"));
    fsDir = make_shared<SimpleFSDirectory>(path);
    shared_ptr<RAMDirectory> ramDir =
        make_shared<RAMDirectory>(fsDir, newIOContext(random()));
    deque<wstring> files = Arrays::asList(ramDir->listAll());
    assertFalse(find(files.begin(), files.end(), L"subdir") != files.end());
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({fsDir});
  }
}

void TestRAMDirectory::testRAMDirectory() 
{
  shared_ptr<Path> indexDir = buildIndex();

  shared_ptr<FSDirectory> dir = make_shared<SimpleFSDirectory>(indexDir);
  shared_ptr<MockDirectoryWrapper> ramDir = make_shared<MockDirectoryWrapper>(
      random(), make_shared<RAMDirectory>(dir, newIOContext(random())));

  // close the underlaying directory
  delete dir;

  // Check size
  assertEquals(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

  // open reader to test document count
  shared_ptr<IndexReader> reader = DirectoryReader::open(ramDir);
  assertEquals(DOCS_TO_ADD, reader->numDocs());

  // open search zo check if all doc's are there
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  // search for all documents
  for (int i = 0; i < DOCS_TO_ADD; i++) {
    shared_ptr<Document> doc = searcher->doc(i);
    assertTrue(doc->getField(L"content") != nullptr);
  }

  // cleanup
  delete reader;
}

void TestRAMDirectory::testRAMDirectorySize() throw(IOException,
                                                    InterruptedException)
{

  shared_ptr<Path> indexDir = buildIndex();

  shared_ptr<FSDirectory> dir = make_shared<SimpleFSDirectory>(indexDir);
  shared_ptr<MockDirectoryWrapper> *const ramDir =
      make_shared<MockDirectoryWrapper>(
          random(), make_shared<RAMDirectory>(dir, newIOContext(random())));
  delete dir;

  shared_ptr<IndexWriter> *const writer = make_shared<IndexWriter>(
      ramDir,
      (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
          ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);

  assertEquals(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

  std::deque<std::shared_ptr<Thread>> threads(NUM_THREADS);
  for (int i = 0; i < NUM_THREADS; i++) {
    constexpr int num = i;
    threads[i] =
        make_shared<ThreadAnonymousInnerClass>(shared_from_this(), writer, num);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->start();
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    threads[i]->join();
  }

  writer->forceMerge(1);
  assertEquals(ramDir->sizeInBytes(), ramDir->getRecomputedSizeInBytes());

  delete writer;
}

TestRAMDirectory::ThreadAnonymousInnerClass::ThreadAnonymousInnerClass(
    shared_ptr<TestRAMDirectory> outerInstance, shared_ptr<IndexWriter> writer,
    int num)
{
  this->outerInstance = outerInstance;
  this->writer = writer;
  this->num = num;
}

void TestRAMDirectory::ThreadAnonymousInnerClass::run()
{
  for (int j = 1; j < DOCS_PER_THREAD; j++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(
        L"sizeContent",
        StringHelper::trim(English::intToEnglish(num * DOCS_PER_THREAD + j)),
        Field::Store::YES));
    try {
      writer->addDocument(doc);
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  }
}

void TestRAMDirectory::testShouldThrowEOFException() 
{
  shared_ptr<Random> *const random = TestRAMDirectory::random();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (Directory dir = newDirectory())
  {
    Directory dir = newDirectory();
    constexpr int len =
        16 + TestRAMDirectory::random()->nextInt(2048) / 16 * 16;
    const std::deque<char> bytes = std::deque<char>(len);

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexOutput os = dir.createOutput("foo",
    // newIOContext(random)))
    {
      IndexOutput os = dir->createOutput(L"foo", newIOContext(random));
      os->writeBytes(bytes, bytes.size());
    }

    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (IndexInput is = dir.openInput("foo",
    // newIOContext(random)))
    {
      IndexInput is = dir->openInput(L"foo", newIOContext(random));
      expectThrows(EOFException::typeid, [&]() {
        is->seek(0);
        is->seek(len + TestRAMDirectory::random()->nextInt(2048));
        is->readBytes(bytes, 0, 16);
      });
    }
  }
}
} // namespace org::apache::lucene::store