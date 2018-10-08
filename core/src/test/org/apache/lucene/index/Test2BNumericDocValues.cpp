using namespace std;

#include "Test2BNumericDocValues.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;

void Test2BNumericDocValues::testNumerics() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BNumerics"));
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
  shared_ptr<NumericDocValuesField> dvField =
      make_shared<NumericDocValuesField>(L"dv", 0);
  doc->push_back(dvField);

  for (int i = 0; i < IndexWriter::MAX_DOCS; i++) {
    dvField->setLongValue(i);
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
  int64_t expectedValue = 0;
  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<LeafReader> reader = context->reader();
    shared_ptr<NumericDocValues> dv = reader->getNumericDocValues(L"dv");
    for (int i = 0; i < reader->maxDoc(); i++) {
      TestUtil::assertEquals(i, dv->nextDoc());
      TestUtil::assertEquals(expectedValue, dv->longValue());
      expectedValue++;
    }
  }

  r->close();
  delete dir;
}
} // namespace org::apache::lucene::index