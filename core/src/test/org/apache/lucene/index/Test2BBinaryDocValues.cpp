using namespace std;

#include "Test2BBinaryDocValues.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;

void Test2BBinaryDocValues::testFixedBinary() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BFixedBinary"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
               .setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               .setRAMBufferSizeMB(256.0)
               .setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               .setMergePolicy(newLogMergePolicy(false, 10))
               .setOpenMode(IndexWriterConfig::OpenMode::CREATE)
               .setCodec(TestUtil::getDefaultCodec()));

  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> bytes(4);
  shared_ptr<BytesRef> data = make_shared<BytesRef>(bytes);
  shared_ptr<BinaryDocValuesField> dvField =
      make_shared<BinaryDocValuesField>(L"dv", data);
  doc->push_back(dvField);

  for (int i = 0; i < IndexWriter::MAX_DOCS; i++) {
    bytes[0] = static_cast<char>(i >> 24);
    bytes[1] = static_cast<char>(i >> 16);
    bytes[2] = static_cast<char>(i >> 8);
    bytes[3] = static_cast<char>(i);
    w->addDocument(doc);
    if (i % 100000 == 0) {
      wcout << L"indexed: " << i << endl;
      System::out::flush();
    }
  }

  w->forceMerge(1);
  delete w;

  wcout << L"verifying..." << endl;
  System::out::flush();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  int expectedValue = 0;
  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<LeafReader> reader = context->reader();
    shared_ptr<BinaryDocValues> dv = reader->getBinaryDocValues(L"dv");
    for (int i = 0; i < reader->maxDoc(); i++) {
      bytes[0] = static_cast<char>(expectedValue >> 24);
      bytes[1] = static_cast<char>(expectedValue >> 16);
      bytes[2] = static_cast<char>(expectedValue >> 8);
      bytes[3] = static_cast<char>(expectedValue);
      TestUtil::assertEquals(i, dv->nextDoc());
      shared_ptr<BytesRef> *const term = dv->binaryValue();
      TestUtil::assertEquals(data, term);
      expectedValue++;
    }
  }

  r->close();
  delete dir;
}

void Test2BBinaryDocValues::testVariableBinary() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BVariableBinary"));
  if (std::dynamic_pointer_cast<MockDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<MockDirectoryWrapper>(dir))
        ->setThrottling(MockDirectoryWrapper::Throttling::NEVER);
  }

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()))
               .setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH)
               .setRAMBufferSizeMB(256.0)
               .setMergeScheduler(make_shared<ConcurrentMergeScheduler>())
               .setMergePolicy(newLogMergePolicy(false, 10))
               .setOpenMode(IndexWriterConfig::OpenMode::CREATE)
               .setCodec(TestUtil::getDefaultCodec()));

  shared_ptr<Document> doc = make_shared<Document>();
  std::deque<char> bytes(4);
  shared_ptr<ByteArrayDataOutput> encoder =
      make_shared<ByteArrayDataOutput>(bytes);
  shared_ptr<BytesRef> data = make_shared<BytesRef>(bytes);
  shared_ptr<BinaryDocValuesField> dvField =
      make_shared<BinaryDocValuesField>(L"dv", data);
  doc->push_back(dvField);

  for (int i = 0; i < IndexWriter::MAX_DOCS; i++) {
    encoder->reset(bytes);
    encoder->writeVInt(i % 65535); // 1, 2, or 3 bytes
    data->length = encoder->getPosition();
    w->addDocument(doc);
    if (i % 100000 == 0) {
      wcout << L"indexed: " << i << endl;
      System::out::flush();
    }
  }

  w->forceMerge(1);
  delete w;

  wcout << L"verifying..." << endl;
  System::out::flush();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  int expectedValue = 0;
  shared_ptr<ByteArrayDataInput> input = make_shared<ByteArrayDataInput>();
  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<LeafReader> reader = context->reader();
    shared_ptr<BinaryDocValues> dv = reader->getBinaryDocValues(L"dv");
    for (int i = 0; i < reader->maxDoc(); i++) {
      TestUtil::assertEquals(i, dv->nextDoc());
      shared_ptr<BytesRef> *const term = dv->binaryValue();
      input->reset(term->bytes, term->offset, term->length);
      TestUtil::assertEquals(expectedValue % 65535, input->readVInt());
      assertTrue(input->eof());
      expectedValue++;
    }
  }

  r->close();
  delete dir;
}
} // namespace org::apache::lucene::index