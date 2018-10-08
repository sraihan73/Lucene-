using namespace std;

#include "TestNRTReaderCleanup.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using Constants = org::apache::lucene::util::Constants;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::lucene::util::LuceneTestCase::SuppressFileSystems;

void TestNRTReaderCleanup::testClosingNRTReaderDoesNotCorruptYourIndex() throw(
    IOException)
{

  // Windows disallows deleting & overwriting files still
  // open for reading:
  assumeFalse(L"this test can't run on Windows", Constants::WINDOWS);

  shared_ptr<MockDirectoryWrapper> dir = newMockDirectory();

  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  shared_ptr<LogMergePolicy> lmp = make_shared<LogDocMergePolicy>();
  lmp->setMergeFactor(2);
  iwc->setMergePolicy(lmp);

  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"a", L"foo", Field::Store::NO));
  w->addDocument(doc);
  w->commit();
  w->addDocument(doc);

  // Get a new reader, but this also sets off a merge:
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  // Blow away index and make a new writer:
  for (auto name : dir->listAll()) {
    dir->deleteFile(name);
  }

  w = make_shared<RandomIndexWriter>(random(), dir);
  w->addDocument(doc);
  delete w;
  delete r;
  delete dir;
}
} // namespace org::apache::lucene::index