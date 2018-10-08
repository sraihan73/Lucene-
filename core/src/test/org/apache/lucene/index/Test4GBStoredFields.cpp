using namespace std;

#include "Test4GBStoredFields.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using CompressingCodec =
    org::apache::lucene::codecs::compressing::CompressingCodec;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using MMapDirectory = org::apache::lucene::store::MMapDirectory;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TimeUnits = org::apache::lucene::util::TimeUnits;
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void test() throws Exception
void Test4GBStoredFields::test() 
{
  assumeWorkingMMapOnWindows();

  shared_ptr<MockDirectoryWrapper> dir = make_shared<MockDirectoryWrapper>(
      random(), make_shared<MMapDirectory>(createTempDir(L"4GBStoredFields")));
  dir->setThrottling(MockDirectoryWrapper::Throttling::NEVER);

  shared_ptr<IndexWriterConfig> iwc =
      make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random()));
  iwc->setMaxBufferedDocs(IndexWriterConfig::DISABLE_AUTO_FLUSH);
  iwc->setRAMBufferSizeMB(256.0);
  iwc->setMergeScheduler(make_shared<ConcurrentMergeScheduler>());
  iwc->setMergePolicy(newLogMergePolicy(false, 10));
  iwc->setOpenMode(IndexWriterConfig::OpenMode::CREATE);

  // TODO: we disable "Compressing" since it likes to pick very extreme values
  // which will be too slow for this test. maybe we should factor out crazy
  // cases to ExtremeCompressing? then annotations can handle this stuff...
  if (random()->nextBoolean()) {
    iwc->setCodec(CompressingCodec::reasonableInstance(random()));
  }

  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(dir, iwc);

  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  if (std::dynamic_pointer_cast<LogByteSizeMergePolicy>(mp) != nullptr) {
    // 1 petabyte:
    (std::static_pointer_cast<LogByteSizeMergePolicy>(mp))
        ->setMaxMergeMB(1024 * 1024 * 1024);
  }

  shared_ptr<Document> *const doc = make_shared<Document>();
  shared_ptr<FieldType> *const ft = make_shared<FieldType>();
  ft->setStored(true);
  ft->freeze();
  constexpr int valueLength =
      RandomNumbers::randomIntBetween(random(), 1 << 13, 1 << 20);
  const std::deque<char> value = std::deque<char>(valueLength);
  for (int i = 0; i < valueLength; ++i) {
    // random so that even compressing codecs can't compress it
    value[i] = static_cast<char>(random()->nextInt(256));
  }
  shared_ptr<Field> *const f = make_shared<Field>(L"fld", value, ft);
  doc->push_back(f);

  constexpr int numDocs = static_cast<int>((1LL << 32) / valueLength + 100);
  for (int i = 0; i < numDocs; ++i) {
    w->addDocument(doc);
    if (VERBOSE && i % (numDocs / 10) == 0) {
      wcout << i << L" of " << numDocs << L"..." << endl;
    }
  }
  w->forceMerge(1);
  delete w;
  if (VERBOSE) {
    bool found = false;
    for (auto file : dir->listAll()) {
      if (file.endsWith(L".fdt")) {
        constexpr int64_t fileLength = dir->fileLength(file);
        if (fileLength >= 1LL << 32) {
          found = true;
        }
        wcout << L"File length of " << file << L" : " << fileLength << endl;
      }
    }
    if (!found) {
      wcout << L"No .fdt file larger than 4GB, test bug?" << endl;
    }
  }

  shared_ptr<DirectoryReader> rd = DirectoryReader::open(dir);
  shared_ptr<Document> sd = rd->document(numDocs - 1);
  assertNotNull(sd);
  assertEquals(1, sd->getFields().size());
  shared_ptr<BytesRef> valueRef = sd->getBinaryValue(L"fld");
  assertNotNull(valueRef);
  assertEquals(make_shared<BytesRef>(value), valueRef);
  rd->close();

  delete dir;
}
} // namespace org::apache::lucene::index