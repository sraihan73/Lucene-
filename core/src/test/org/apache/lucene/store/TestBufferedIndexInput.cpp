using namespace std;

#include "TestBufferedIndexInput.h"

namespace org::apache::lucene::store
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TermQuery = org::apache::lucene::search::TermQuery;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBufferedIndexInput::testReadByte() 
{
  shared_ptr<MyBufferedIndexInput> input = make_shared<MyBufferedIndexInput>();
  for (int i = 0; i < BufferedIndexInput::BUFFER_SIZE * 10; i++) {
    assertEquals(input->readByte(), byten(i));
  }
}

void TestBufferedIndexInput::testReadBytes() 
{
  shared_ptr<MyBufferedIndexInput> input = make_shared<MyBufferedIndexInput>();
  runReadBytes(input, BufferedIndexInput::BUFFER_SIZE, random());
}

void TestBufferedIndexInput::runReadBytesAndClose(
    shared_ptr<IndexInput> input, int bufferSize,
    shared_ptr<Random> r) 
{
  try {
    runReadBytes(input, bufferSize, r);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete input;
  }
}

void TestBufferedIndexInput::runReadBytes(
    shared_ptr<IndexInput> input, int bufferSize,
    shared_ptr<Random> r) 
{

  int pos = 0;
  // gradually increasing size:
  for (int size = 1; size < bufferSize * 10; size = size + size / 200 + 1) {
    checkReadBytes(input, size, pos);
    pos += size;
    if (pos >= TEST_FILE_LENGTH) {
      // wrap
      pos = 0;
      input->seek(0LL);
    }
  }
  // wildly fluctuating size:
  for (int64_t i = 0; i < 100; i++) {
    constexpr int size = r->nextInt(10000);
    checkReadBytes(input, 1 + size, pos);
    pos += 1 + size;
    if (pos >= TEST_FILE_LENGTH) {
      // wrap
      pos = 0;
      input->seek(0LL);
    }
  }
  // constant small size (7 bytes):
  for (int i = 0; i < bufferSize; i++) {
    checkReadBytes(input, 7, pos);
    pos += 7;
    if (pos >= TEST_FILE_LENGTH) {
      // wrap
      pos = 0;
      input->seek(0LL);
    }
  }
}

void TestBufferedIndexInput::checkReadBytes(shared_ptr<IndexInput> input,
                                            int size,
                                            int pos) 
{
  // Just to see that "offset" is treated properly in readBytes(), we
  // add an arbitrary offset at the beginning of the array
  int offset = size % 10; // arbitrary
  buffer = ArrayUtil::grow(buffer, offset + size);
  assertEquals(pos, input->getFilePointer());
  int64_t left = TEST_FILE_LENGTH - input->getFilePointer();
  if (left <= 0) {
    return;
  } else if (left < size) {
    size = static_cast<int>(left);
  }
  input->readBytes(buffer, offset, size);
  assertEquals(pos + size, input->getFilePointer());
  for (int i = 0; i < size; i++) {
    assertEquals(L"pos=" + to_wstring(i) + L" filepos=" + to_wstring(pos + i),
                 byten(pos + i), buffer[offset + i]);
  }
}

void TestBufferedIndexInput::testEOF() 
{
  shared_ptr<MyBufferedIndexInput> input =
      make_shared<MyBufferedIndexInput>(1024);
  // see that we can read all the bytes at one go:
  checkReadBytes(input, static_cast<int>(input->length()), 0);
  // go back and see that we can't read more than that, for small and
  // large overflows:
  int pos = static_cast<int>(input->length()) - 10;
  input->seek(pos);
  checkReadBytes(input, 10, pos);
  input->seek(pos);
  // block read past end of file
  expectThrows(IOException::typeid, [&]() { checkReadBytes(input, 11, pos); });

  input->seek(pos);

  // block read past end of file
  expectThrows(IOException::typeid, [&]() { checkReadBytes(input, 50, pos); });

  input->seek(pos);

  // block read past end of file
  expectThrows(IOException::typeid,
               [&]() { checkReadBytes(input, 100000, pos); });
}

char TestBufferedIndexInput::byten(int64_t n)
{
  return static_cast<char>(n * n % 256);
}

TestBufferedIndexInput::MyBufferedIndexInput::MyBufferedIndexInput(
    int64_t len)
    : BufferedIndexInput(L"MyBufferedIndexInput(len=" + len + L")",
                         BufferedIndexInput::BUFFER_SIZE)
{
  this->len = len;
  this->pos = 0;
}

TestBufferedIndexInput::MyBufferedIndexInput::MyBufferedIndexInput()
    : MyBufferedIndexInput(numeric_limits<long>::max())
{
  // an infinite file
}

void TestBufferedIndexInput::MyBufferedIndexInput::readInternal(
    std::deque<char> &b, int offset, int length) 
{
  for (int i = offset; i < offset + length; i++) {
    b[i] = byten(pos++);
  }
}

void TestBufferedIndexInput::MyBufferedIndexInput::seekInternal(
    int64_t pos) 
{
  this->pos = pos;
}

TestBufferedIndexInput::MyBufferedIndexInput::~MyBufferedIndexInput() {}

int64_t TestBufferedIndexInput::MyBufferedIndexInput::length() { return len; }

shared_ptr<IndexInput> TestBufferedIndexInput::MyBufferedIndexInput::slice(
    const wstring &sliceDescription, int64_t offset,
    int64_t length) 
{
  throw make_shared<UnsupportedOperationException>();
}

void TestBufferedIndexInput::testSetBufferSize() 
{
  shared_ptr<Path> indexDir = createTempDir(L"testSetBufferSize");
  shared_ptr<MockFSDirectory> dir =
      make_shared<MockFSDirectory>(indexDir, random());
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
               ->setOpenMode(IndexWriterConfig::OpenMode::CREATE)
               ->setMergePolicy(newLogMergePolicy(false)));
  for (int i = 0; i < 37; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newTextField(L"content", L"aaa bbb ccc ddd" + to_wstring(i),
                                Field::Store::YES));
    doc->push_back(newTextField(L"id", L"" + to_wstring(i), Field::Store::YES));
    writer->addDocument(doc);
  }

  dir->allIndexInputs.clear();

  shared_ptr<IndexReader> reader = DirectoryReader::open(writer);
  shared_ptr<Term> aaa = make_shared<Term>(L"content", L"aaa");
  shared_ptr<Term> bbb = make_shared<Term>(L"content", L"bbb");

  delete reader;

  dir->tweakBufferSizes();
  writer->deleteDocuments({make_shared<Term>(L"id", L"0")});
  reader = DirectoryReader::open(writer);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(make_shared<TermQuery>(bbb), 1000)->scoreDocs;
  dir->tweakBufferSizes();
  assertEquals(36, hits.size());

  delete reader;

  dir->tweakBufferSizes();
  writer->deleteDocuments({make_shared<Term>(L"id", L"4")});
  reader = DirectoryReader::open(writer);
  searcher = newSearcher(reader);

  hits = searcher->search(make_shared<TermQuery>(bbb), 1000)->scoreDocs;
  dir->tweakBufferSizes();
  assertEquals(35, hits.size());
  dir->tweakBufferSizes();
  hits = searcher
             ->search(make_shared<TermQuery>(make_shared<Term>(L"id", L"33")),
                      1000)
             ->scoreDocs;
  dir->tweakBufferSizes();
  assertEquals(1, hits.size());
  hits = searcher->search(make_shared<TermQuery>(aaa), 1000)->scoreDocs;
  dir->tweakBufferSizes();
  assertEquals(35, hits.size());
  delete writer;
  delete reader;
}

TestBufferedIndexInput::MockFSDirectory::MockFSDirectory(
    shared_ptr<Path> path, shared_ptr<Random> rand) 
    : FilterDirectory(new SimpleFSDirectory(path)), rand(rand)
{
}

void TestBufferedIndexInput::MockFSDirectory::tweakBufferSizes()
{
  // int count = 0;
  for (auto ip : allIndexInputs) {
    shared_ptr<BufferedIndexInput> bii =
        std::static_pointer_cast<BufferedIndexInput>(ip);
    int bufferSize = 1024 + abs(rand->nextInt() % 32768);
    bii->setBufferSize(bufferSize);
    // count++;
  }
  // System.out.println("tweak'd " + count + " buffer sizes");
}

shared_ptr<IndexInput> TestBufferedIndexInput::MockFSDirectory::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  // Make random changes to buffer size
  // bufferSize = 1+Math.abs(rand.nextInt() % 10);
  shared_ptr<IndexInput> f = FilterDirectory::openInput(name, context);
  allIndexInputs.push_back(f);
  return f;
}
} // namespace org::apache::lucene::store