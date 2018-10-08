using namespace std;

#include "BaseTestCheckIndex.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using LockObtainFailedException =
    org::apache::lucene::store::LockObtainFailedException;
using IOUtils = org::apache::lucene::util::IOUtils;
using LineFileDocs = org::apache::lucene::util::LineFileDocs;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void BaseTestCheckIndex::testDeletedDocs(shared_ptr<Directory> dir) throw(
    IOException)
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  for (int i = 0; i < 19; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType->setStoreTermVectors(true);
    customType->setStoreTermVectorPositions(true);
    customType->setStoreTermVectorOffsets(true);
    doc->push_back(newField(L"field", L"aaa" + to_wstring(i), customType));
    writer->addDocument(doc);
  }
  writer->forceMerge(1);
  writer->commit();
  writer->deleteDocuments({make_shared<Term>(L"field", L"aaa5")});
  delete writer;

  shared_ptr<ByteArrayOutputStream> bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(dir);
  checker->setInfoStream(make_shared<PrintStream>(bos, false, IOUtils::UTF_8));
  if (VERBOSE) {
    checker->setInfoStream(System::out);
  }
  shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex();
  if (indexStatus->clean == false) {
    wcout << L"CheckIndex failed" << endl;
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << bos->toString(IOUtils::UTF_8) << endl;
    fail();
  }

  shared_ptr<CheckIndex::Status::SegmentInfoStatus> *const seg =
      indexStatus->segmentInfos[0];
  assertTrue(seg->openReaderPassed);

  assertNotNull(seg->diagnostics);

  assertNotNull(seg->fieldNormStatus);
  assertNull(seg->fieldNormStatus->error);
  TestUtil::assertEquals(1, seg->fieldNormStatus->totFields);

  assertNotNull(seg->termIndexStatus);
  assertNull(seg->termIndexStatus->error);
  TestUtil::assertEquals(18, seg->termIndexStatus->termCount);
  TestUtil::assertEquals(18, seg->termIndexStatus->totFreq);
  TestUtil::assertEquals(18, seg->termIndexStatus->totPos);

  assertNotNull(seg->storedFieldStatus);
  assertNull(seg->storedFieldStatus->error);
  TestUtil::assertEquals(18, seg->storedFieldStatus->docCount);
  TestUtil::assertEquals(18, seg->storedFieldStatus->totFields);

  assertNotNull(seg->termVectorStatus);
  assertNull(seg->termVectorStatus->error);
  TestUtil::assertEquals(18, seg->termVectorStatus->docCount);
  TestUtil::assertEquals(18, seg->termVectorStatus->totVectors);

  assertNotNull(seg->diagnostics[L"java.vm.version"]);
  assertNotNull(seg->diagnostics[L"java.runtime.version"]);

  assertTrue(seg->diagnostics.size() > 0);
  const deque<wstring> onlySegments = deque<wstring>();
  onlySegments.push_back(L"_0");

  assertTrue(checker->checkIndex(onlySegments)->clean == true);
  delete checker;
}

void BaseTestCheckIndex::testChecksumsOnly(shared_ptr<Directory> dir) throw(
    IOException)
{
  shared_ptr<LineFileDocs> lf = make_shared<LineFileDocs>(random());
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  for (int i = 0; i < 100; i++) {
    iw->addDocument(lf->nextDoc());
  }
  iw->addDocument(make_shared<Document>());
  iw->commit();
  delete iw;
  delete lf;

  shared_ptr<ByteArrayOutputStream> bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(dir);
  checker->setInfoStream(make_shared<PrintStream>(bos, false, IOUtils::UTF_8));
  if (VERBOSE) {
    checker->setInfoStream(System::out);
  }
  shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex();
  assertTrue(indexStatus->clean);
  delete checker;
  delete analyzer;
}

void BaseTestCheckIndex::testChecksumsOnlyVerbose(
    shared_ptr<Directory> dir) 
{
  shared_ptr<LineFileDocs> lf = make_shared<LineFileDocs>(random());
  shared_ptr<MockAnalyzer> analyzer = make_shared<MockAnalyzer>(random());
  analyzer->setMaxTokenLength(
      TestUtil::nextInt(random(), 1, IndexWriter::MAX_TERM_LENGTH));
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  for (int i = 0; i < 100; i++) {
    iw->addDocument(lf->nextDoc());
  }
  iw->addDocument(make_shared<Document>());
  iw->commit();
  delete iw;
  delete lf;

  shared_ptr<ByteArrayOutputStream> bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<CheckIndex> checker = make_shared<CheckIndex>(dir);
  checker->setInfoStream(make_shared<PrintStream>(bos, true, IOUtils::UTF_8));
  if (VERBOSE) {
    checker->setInfoStream(System::out);
  }
  shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex();
  assertTrue(indexStatus->clean);
  delete checker;
  delete analyzer;
}

void BaseTestCheckIndex::testObtainsLock(shared_ptr<Directory> dir) throw(
    IOException)
{
  shared_ptr<IndexWriter> iw =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(nullptr));
  iw->addDocument(make_shared<Document>());
  iw->commit();

  // keep IW open... should not be able to obtain write lock
  expectThrows(LockObtainFailedException::typeid,
               [&]() { make_shared<CheckIndex>(dir); });

  delete iw;
}
} // namespace org::apache::lucene::index