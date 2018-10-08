using namespace std;

#include "Test2BSortedDocValuesOrds.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;

void Test2BSortedDocValuesOrds::test2BOrds() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BOrds"));
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
  shared_ptr<SortedDocValuesField> dvField =
      make_shared<SortedDocValuesField>(L"dv", data);
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
  int counter = 0;
  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<LeafReader> reader = context->reader();
    shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
    shared_ptr<BinaryDocValues> dv = DocValues::getBinary(reader, L"dv");
    for (int i = 0; i < reader->maxDoc(); i++) {
      TestUtil::assertEquals(i, dv->nextDoc());
      bytes[0] = static_cast<char>(counter >> 24);
      bytes[1] = static_cast<char>(counter >> 16);
      bytes[2] = static_cast<char>(counter >> 8);
      bytes[3] = static_cast<char>(counter);
      counter++;
      shared_ptr<BytesRef> *const term = dv->binaryValue();
      TestUtil::assertEquals(data, term);
    }
  }

  r->close();
  delete dir;
}
} // namespace org::apache::lucene::index