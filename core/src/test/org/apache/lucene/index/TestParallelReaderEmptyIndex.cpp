using namespace std;

#include "TestParallelReaderEmptyIndex.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestParallelReaderEmptyIndex::testEmptyIndex() 
{
  shared_ptr<Directory> rd1 = newDirectory();
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      rd1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  delete iw;
  // create a copy:
  shared_ptr<Directory> rd2 = newDirectory(rd1);

  shared_ptr<Directory> rdOut = newDirectory();

  shared_ptr<IndexWriter> iwOut = make_shared<IndexWriter>(
      rdOut, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  // add a readerless parallel reader
  iwOut->addIndexes(
      {SlowCodecReaderWrapper::wrap(make_shared<ParallelLeafReader>())});
  iwOut->forceMerge(1);

  shared_ptr<ParallelCompositeReader> cpr =
      make_shared<ParallelCompositeReader>(DirectoryReader::open(rd1),
                                           DirectoryReader::open(rd2));

  // When unpatched, Lucene crashes here with a NoSuchElementException (caused
  // by ParallelTermEnum)
  deque<std::shared_ptr<CodecReader>> leaves =
      deque<std::shared_ptr<CodecReader>>();
  for (shared_ptr<LeafReaderContext> leaf : cpr->leaves()) {
    leaves.push_back(SlowCodecReaderWrapper::wrap(leaf->reader()));
  }
  iwOut->addIndexes(
      {leaves.toArray(std::deque<std::shared_ptr<CodecReader>>(0))});
  iwOut->forceMerge(1);

  delete iwOut;
  delete rdOut;
  delete rd1;
  delete rd2;
}

void TestParallelReaderEmptyIndex::testEmptyIndexWithVectors() throw(
    IOException)
{
  shared_ptr<Directory> rd1 = newDirectory();
  {
    if (VERBOSE) {
      wcout << L"\nTEST: make 1st writer" << endl;
    }
    shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
        rd1, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> idField = newTextField(L"id", L"", Field::Store::NO);
    doc->push_back(idField);
    shared_ptr<FieldType> customType =
        make_shared<FieldType>(TextField::TYPE_NOT_STORED);
    customType->setStoreTermVectors(true);
    doc->push_back(newField(L"test", L"", customType));
    idField->setStringValue(L"1");
    iw->addDocument(doc);
    doc->push_back(newField(L"test", L"", customType));
    idField->setStringValue(L"2");
    iw->addDocument(doc);
    delete iw;

    shared_ptr<IndexWriterConfig> dontMergeConfig =
        (make_shared<IndexWriterConfig>(make_shared<MockAnalyzer>(random())))
            ->setMergePolicy(NoMergePolicy::INSTANCE);
    if (VERBOSE) {
      wcout << L"\nTEST: make 2nd writer" << endl;
    }
    shared_ptr<IndexWriter> writer =
        make_shared<IndexWriter>(rd1, dontMergeConfig);

    writer->deleteDocuments({make_shared<Term>(L"id", L"1")});
    delete writer;
    shared_ptr<IndexReader> ir = DirectoryReader::open(rd1);
    assertEquals(2, ir->maxDoc());
    assertEquals(1, ir->numDocs());
    delete ir;

    iw = make_shared<IndexWriter>(
        rd1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setOpenMode(OpenMode::APPEND));
    iw->forceMerge(1);
    delete iw;
  }

  shared_ptr<Directory> rd2 = newDirectory();
  {
    shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
        rd2, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    shared_ptr<Document> doc = make_shared<Document>();
    iw->addDocument(doc);
    delete iw;
  }

  shared_ptr<Directory> rdOut = newDirectory();

  shared_ptr<IndexWriter> iwOut = make_shared<IndexWriter>(
      rdOut, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<DirectoryReader> reader1 = DirectoryReader::open(rd1);
  shared_ptr<DirectoryReader> reader2 = DirectoryReader::open(rd2);
  shared_ptr<ParallelLeafReader> pr = make_shared<ParallelLeafReader>(
      false, getOnlyLeafReader(reader1), getOnlyLeafReader(reader2));

  // When unpatched, Lucene crashes here with an ArrayIndexOutOfBoundsException
  // (caused by TermVectorsWriter)
  iwOut->addIndexes({SlowCodecReaderWrapper::wrap(pr)});

  delete pr;
  reader1->close();
  reader2->close();

  // assert subreaders were closed
  assertEquals(0, reader1->getRefCount());
  assertEquals(0, reader2->getRefCount());

  delete rd1;
  delete rd2;

  iwOut->forceMerge(1);
  delete iwOut;

  delete rdOut;
}
} // namespace org::apache::lucene::index