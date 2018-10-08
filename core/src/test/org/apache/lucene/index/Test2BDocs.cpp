using namespace std;

#include "Test2BDocs.h"

namespace org::apache::lucene::index
{
using com::carrotsearch::randomizedtesting::annotations::TimeoutSuite;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::Monster;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::apache::lucene::util::LuceneTestCase::SuppressSysoutChecks;
using TestUtil = org::apache::lucene::util::TestUtil;
using TimeUnits = org::apache::lucene::util::TimeUnits;

void Test2BDocs::test2BDocs() 
{
  shared_ptr<BaseDirectoryWrapper> dir =
      newFSDirectory(createTempDir(L"2BDocs"));
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
  shared_ptr<Field> field =
      make_shared<Field>(L"f1", L"a", StringField::TYPE_NOT_STORED);
  doc->push_back(field);

  for (int i = 0; i < IndexWriter::MAX_DOCS; i++) {
    w->addDocument(doc);
    if (i % (10 * 1000 * 1000) == 0) {
      wcout << L"indexed: " << i << endl;
      System::out::flush();
    }
  }

  w->forceMerge(1);
  delete w;

  wcout << L"verifying..." << endl;
  System::out::flush();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);

  shared_ptr<BytesRef> term = make_shared<BytesRef>(1);
  term->bytes[0] = static_cast<char>(L'a');
  term->length = 1;

  int64_t skips = 0;

  shared_ptr<Random> rnd = random();

  int64_t start = System::nanoTime();

  for (shared_ptr<LeafReaderContext> context : r->leaves()) {
    shared_ptr<LeafReader> reader = context->reader();
    int lim = context->reader()->maxDoc();

    shared_ptr<Terms> terms = reader->terms(L"f1");
    for (int i = 0; i < 10000; i++) {
      shared_ptr<TermsEnum> te = terms->begin();
      assertTrue(te->seekExact(term));
      shared_ptr<PostingsEnum> docs = te->postings(nullptr);

      // skip randomly through the term
      for (int target = -1;;) {
        int maxSkipSize = lim - target + 1;
        // do a smaller skip half of the time
        if (rnd->nextBoolean()) {
          maxSkipSize = min(256, maxSkipSize);
        }
        int newTarget = target + rnd->nextInt(maxSkipSize) + 1;
        if (newTarget >= lim) {
          if (target + 1 >= lim) {
            break; // we already skipped to end, so break.
          }
          newTarget = lim - 1; // skip to end
        }
        target = newTarget;

        int res = docs->advance(target);
        if (res == PostingsEnum::NO_MORE_DOCS) {
          break;
        }

        assertTrue(res >= target);

        skips++;
        target = res;
      }
    }
  }

  r->close();
  delete dir;

  int64_t end = System::nanoTime();

  wcout << L"Skip count=" << skips << L" seconds="
        << TimeUnit::NANOSECONDS::toSeconds(end - start) << endl;
  assert(skips > 0);
}
} // namespace org::apache::lucene::index