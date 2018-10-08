using namespace std;

#include "TestDirectoryReader.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Assume;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

void TestDirectoryReader::testDocument() 
{
  std::deque<std::shared_ptr<SegmentReader>> readers(2);
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc1 = make_shared<Document>();
  shared_ptr<Document> doc2 = make_shared<Document>();
  DocHelper::setupDoc(doc1);
  DocHelper::setupDoc(doc2);
  DocHelper::writeDoc(random(), dir, doc1);
  DocHelper::writeDoc(random(), dir, doc2);
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  assertTrue(reader != nullptr);
  assertTrue(std::dynamic_pointer_cast<StandardDirectoryReader>(reader) !=
             nullptr);

  shared_ptr<Document> newDoc1 = reader->document(0);
  assertTrue(newDoc1->size() > 0);
  assertTrue(DocHelper::numFields(newDoc1) ==
             DocHelper::numFields(doc1) - DocHelper::unstored.size());
  shared_ptr<Document> newDoc2 = reader->document(1);
  assertTrue(newDoc2->size() > 0);
  assertTrue(DocHelper::numFields(newDoc2) ==
             DocHelper::numFields(doc2) - DocHelper::unstored.size());
  shared_ptr<Terms> deque =
      reader->getTermVectors(0)->terms(DocHelper::TEXT_FIELD_2_KEY);
  assertNotNull(deque);

  reader->close();
  if (readers[0] != nullptr) {
    delete readers[0];
  }
  if (readers[1] != nullptr) {
    delete readers[1];
  }
  delete dir;
}

void TestDirectoryReader::testMultiTermDocs() 
{
  shared_ptr<Directory> ramDir1 = newDirectory();
  addDoc(random(), ramDir1, L"test foo", true);
  shared_ptr<Directory> ramDir2 = newDirectory();
  addDoc(random(), ramDir2, L"test blah", true);
  shared_ptr<Directory> ramDir3 = newDirectory();
  addDoc(random(), ramDir3, L"test wow", true);

  std::deque<std::shared_ptr<IndexReader>> readers1 = {
      DirectoryReader::open(ramDir1), DirectoryReader::open(ramDir3)};
  std::deque<std::shared_ptr<IndexReader>> readers2 = {
      DirectoryReader::open(ramDir1), DirectoryReader::open(ramDir2),
      DirectoryReader::open(ramDir3)};
  shared_ptr<MultiReader> mr2 = make_shared<MultiReader>(readers1);
  shared_ptr<MultiReader> mr3 = make_shared<MultiReader>(readers2);

  // test mixing up TermDocs and TermEnums from different readers.
  shared_ptr<TermsEnum> te2 = MultiFields::getTerms(mr2, L"body")->begin();
  te2->seekCeil(make_shared<BytesRef>(L"wow"));
  shared_ptr<PostingsEnum> td =
      TestUtil::docs(random(), mr2, L"body", te2->term(), nullptr, 0);

  shared_ptr<TermsEnum> te3 = MultiFields::getTerms(mr3, L"body")->begin();
  te3->seekCeil(make_shared<BytesRef>(L"wow"));
  td = TestUtil::docs(random(), te3, td, 0);

  int ret = 0;

  // This should blow up if we forget to check that the TermEnum is from the
  // same reader as the TermDocs.
  while (td->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
    ret += td->docID();
  }

  // really a dummy assert to ensure that we got some docs and to ensure that
  // nothing is eliminated by hotspot
  assertTrue(ret > 0);
  delete readers1[0];
  delete readers1[1];
  delete readers2[0];
  delete readers2[1];
  delete readers2[2];
  delete ramDir1;
  delete ramDir2;
  delete ramDir3;
}

void TestDirectoryReader::addDoc(shared_ptr<Random> random,
                                 shared_ptr<Directory> ramDir1,
                                 const wstring &s,
                                 bool create) 
{
  shared_ptr<IndexWriter> iw = make_shared<IndexWriter>(
      ramDir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random))
                   ->setOpenMode(create ? OpenMode::CREATE : OpenMode::APPEND));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"body", s, Field::Store::NO));
  iw->addDocument(doc);
  delete iw;
}

void TestDirectoryReader::testIsCurrent() 
{
  shared_ptr<Directory> d = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  addDocumentWithFields(writer);
  delete writer;
  // set up reader:
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(d);
  assertTrue(reader->isCurrent());
  // modify index by adding another document:
  writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setOpenMode(OpenMode::APPEND));
  addDocumentWithFields(writer);
  delete writer;
  assertFalse(reader->isCurrent());
  // re-create index:
  writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setOpenMode(OpenMode::CREATE));
  addDocumentWithFields(writer);
  delete writer;
  assertFalse(reader->isCurrent());
  reader->close();
  delete d;
}

void TestDirectoryReader::testGetFieldNames() 
{
  shared_ptr<Directory> d = newDirectory();
  // set up writer
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType3 = make_shared<FieldType>();
  customType3->setStored(true);

  doc->push_back(
      make_shared<StringField>(L"keyword", L"test1", Field::Store::YES));
  doc->push_back(make_shared<TextField>(L"text", L"test1", Field::Store::YES));
  doc->push_back(make_shared<Field>(L"unindexed", L"test1", customType3));
  doc->push_back(
      make_shared<TextField>(L"unstored", L"test1", Field::Store::NO));
  writer->addDocument(doc);

  delete writer;
  // set up reader
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(d);
  shared_ptr<FieldInfos> fieldInfos = MultiFields::getMergedFieldInfos(reader);
  assertNotNull(fieldInfos->fieldInfo(L"keyword"));
  assertNotNull(fieldInfos->fieldInfo(L"text"));
  assertNotNull(fieldInfos->fieldInfo(L"unindexed"));
  assertNotNull(fieldInfos->fieldInfo(L"unstored"));
  reader->close();
  // add more documents
  writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setOpenMode(OpenMode::APPEND)
             ->setMergePolicy(newLogMergePolicy()));
  // want to get some more segments here
  int mergeFactor = (std::static_pointer_cast<LogMergePolicy>(
                         writer->getConfig()->getMergePolicy()))
                        ->getMergeFactor();
  for (int i = 0; i < 5 * mergeFactor; i++) {
    doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"keyword", L"test1", Field::Store::YES));
    doc->push_back(
        make_shared<TextField>(L"text", L"test1", Field::Store::YES));
    doc->push_back(make_shared<Field>(L"unindexed", L"test1", customType3));
    doc->push_back(
        make_shared<TextField>(L"unstored", L"test1", Field::Store::NO));
    writer->addDocument(doc);
  }
  // new fields are in some different segments (we hope)
  for (int i = 0; i < 5 * mergeFactor; i++) {
    doc = make_shared<Document>();
    doc->push_back(
        make_shared<StringField>(L"keyword2", L"test1", Field::Store::YES));
    doc->push_back(
        make_shared<TextField>(L"text2", L"test1", Field::Store::YES));
    doc->push_back(make_shared<Field>(L"unindexed2", L"test1", customType3));
    doc->push_back(
        make_shared<TextField>(L"unstored2", L"test1", Field::Store::NO));
    writer->addDocument(doc);
  }
  // new termvector fields

  shared_ptr<FieldType> customType5 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType5->setStoreTermVectors(true);
  shared_ptr<FieldType> customType6 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType6->setStoreTermVectors(true);
  customType6->setStoreTermVectorOffsets(true);
  shared_ptr<FieldType> customType7 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType7->setStoreTermVectors(true);
  customType7->setStoreTermVectorPositions(true);
  shared_ptr<FieldType> customType8 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType8->setStoreTermVectors(true);
  customType8->setStoreTermVectorOffsets(true);
  customType8->setStoreTermVectorPositions(true);

  for (int i = 0; i < 5 * mergeFactor; i++) {
    doc = make_shared<Document>();
    doc->push_back(
        make_shared<TextField>(L"tvnot", L"tvnot", Field::Store::YES));
    doc->push_back(
        make_shared<Field>(L"termvector", L"termvector", customType5));
    doc->push_back(make_shared<Field>(L"tvoffset", L"tvoffset", customType6));
    doc->push_back(
        make_shared<Field>(L"tvposition", L"tvposition", customType7));
    doc->push_back(make_shared<Field>(L"tvpositionoffset", L"tvpositionoffset",
                                      customType8));
    writer->addDocument(doc);
  }

  delete writer;

  // verify fields again
  reader = DirectoryReader::open(d);
  fieldInfos = MultiFields::getMergedFieldInfos(reader);

  shared_ptr<deque<wstring>> allFieldNames = unordered_set<wstring>();
  shared_ptr<deque<wstring>> indexedFieldNames = unordered_set<wstring>();
  shared_ptr<deque<wstring>> notIndexedFieldNames =
      unordered_set<wstring>();
  shared_ptr<deque<wstring>> tvFieldNames = unordered_set<wstring>();

  for (auto fieldInfo : fieldInfos) {
    const wstring name = fieldInfo->name;
    allFieldNames->add(name);
    if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      indexedFieldNames->add(name);
    } else {
      notIndexedFieldNames->add(name);
    }
    if (fieldInfo->hasVectors()) {
      tvFieldNames->add(name);
    }
  }

  assertTrue(allFieldNames->contains(L"keyword"));
  assertTrue(allFieldNames->contains(L"text"));
  assertTrue(allFieldNames->contains(L"unindexed"));
  assertTrue(allFieldNames->contains(L"unstored"));
  assertTrue(allFieldNames->contains(L"keyword2"));
  assertTrue(allFieldNames->contains(L"text2"));
  assertTrue(allFieldNames->contains(L"unindexed2"));
  assertTrue(allFieldNames->contains(L"unstored2"));
  assertTrue(allFieldNames->contains(L"tvnot"));
  assertTrue(allFieldNames->contains(L"termvector"));
  assertTrue(allFieldNames->contains(L"tvposition"));
  assertTrue(allFieldNames->contains(L"tvoffset"));
  assertTrue(allFieldNames->contains(L"tvpositionoffset"));

  // verify that only indexed fields were returned
  TestUtil::assertEquals(
      11, indexedFieldNames->size()); // 6 original + the 5 termvector fields
  assertTrue(indexedFieldNames->contains(L"keyword"));
  assertTrue(indexedFieldNames->contains(L"text"));
  assertTrue(indexedFieldNames->contains(L"unstored"));
  assertTrue(indexedFieldNames->contains(L"keyword2"));
  assertTrue(indexedFieldNames->contains(L"text2"));
  assertTrue(indexedFieldNames->contains(L"unstored2"));
  assertTrue(indexedFieldNames->contains(L"tvnot"));
  assertTrue(indexedFieldNames->contains(L"termvector"));
  assertTrue(indexedFieldNames->contains(L"tvposition"));
  assertTrue(indexedFieldNames->contains(L"tvoffset"));
  assertTrue(indexedFieldNames->contains(L"tvpositionoffset"));

  // verify that only unindexed fields were returned
  TestUtil::assertEquals(2,
                         notIndexedFieldNames->size()); // the following fields
  assertTrue(notIndexedFieldNames->contains(L"unindexed"));
  assertTrue(notIndexedFieldNames->contains(L"unindexed2"));

  // verify index term deque fields
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(tvFieldNames->toString(), 4,
               tvFieldNames->size()); // 4 field has term deque only
  assertTrue(tvFieldNames->contains(L"termvector"));

  reader->close();
  delete d;
}

void TestDirectoryReader::testTermVectors() 
{
  shared_ptr<Directory> d = newDirectory();
  // set up writer
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setMergePolicy(newLogMergePolicy()));
  // want to get some more segments here
  // new termvector fields
  int mergeFactor = (std::static_pointer_cast<LogMergePolicy>(
                         writer->getConfig()->getMergePolicy()))
                        ->getMergeFactor();
  shared_ptr<FieldType> customType5 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType5->setStoreTermVectors(true);
  shared_ptr<FieldType> customType6 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType6->setStoreTermVectors(true);
  customType6->setStoreTermVectorOffsets(true);
  shared_ptr<FieldType> customType7 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType7->setStoreTermVectors(true);
  customType7->setStoreTermVectorPositions(true);
  shared_ptr<FieldType> customType8 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType8->setStoreTermVectors(true);
  customType8->setStoreTermVectorOffsets(true);
  customType8->setStoreTermVectorPositions(true);
  for (int i = 0; i < 5 * mergeFactor; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<TextField>(
        L"tvnot", L"one two two three three three", Field::Store::YES));
    doc->push_back(make_shared<Field>(
        L"termvector", L"one two two three three three", customType5));
    doc->push_back(make_shared<Field>(
        L"tvoffset", L"one two two three three three", customType6));
    doc->push_back(make_shared<Field>(
        L"tvposition", L"one two two three three three", customType7));
    doc->push_back(make_shared<Field>(
        L"tvpositionoffset", L"one two two three three three", customType8));

    writer->addDocument(doc);
  }
  delete writer;
  delete d;
}

void TestDirectoryReader::assertTermDocsCount(const wstring &msg,
                                              shared_ptr<IndexReader> reader,
                                              shared_ptr<Term> term,
                                              int expected) 
{
  shared_ptr<PostingsEnum> tdocs =
      TestUtil::docs(random(), reader, term->field(),
                     make_shared<BytesRef>(term->text()), nullptr, 0);
  int count = 0;
  if (tdocs != nullptr) {
    while (tdocs->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      count++;
    }
  }
  assertEquals(msg + L", count mismatch", expected, count);
}

void TestDirectoryReader::testBinaryFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  std::deque<char> bin = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy()));

  for (int i = 0; i < 10; i++) {
    addDoc(writer, L"document number " + to_wstring(i + 1));
    addDocumentWithFields(writer);
    addDocumentWithDifferentFields(writer);
    addDocumentWithTermVectorFields(writer);
  }
  delete writer;
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(newLogMergePolicy()));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StoredField>(L"bin1", bin));
  doc->push_back(
      make_shared<TextField>(L"junk", L"junk text", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;
  shared_ptr<DirectoryReader> reader = DirectoryReader::open(dir);
  shared_ptr<Document> doc2 = reader->document(reader->maxDoc() - 1);
  std::deque<std::shared_ptr<IndexableField>> fields =
      doc2->getFields(L"bin1");
  assertNotNull(fields);
  TestUtil::assertEquals(1, fields.size());
  shared_ptr<IndexableField> b1 = fields[0];
  assertTrue(b1->binaryValue() != nullptr);
  shared_ptr<BytesRef> bytesRef = b1->binaryValue();
  TestUtil::assertEquals(bin.size(), bytesRef->length);
  for (int i = 0; i < bin.size(); i++) {
    TestUtil::assertEquals(bin[i], bytesRef->bytes[i + bytesRef->offset]);
  }
  reader->close();
  // force merge

  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::APPEND)
               ->setMergePolicy(newLogMergePolicy()));
  writer->forceMerge(1);
  delete writer;
  reader = DirectoryReader::open(dir);
  doc2 = reader->document(reader->maxDoc() - 1);
  fields = doc2->getFields(L"bin1");
  assertNotNull(fields);
  TestUtil::assertEquals(1, fields.size());
  b1 = fields[0];
  assertTrue(b1->binaryValue() != nullptr);
  bytesRef = b1->binaryValue();
  TestUtil::assertEquals(bin.size(), bytesRef->length);
  for (int i = 0; i < bin.size(); i++) {
    TestUtil::assertEquals(bin[i], bytesRef->bytes[i + bytesRef->offset]);
  }
  reader->close();
  delete dir;
}

void TestDirectoryReader::testFilesOpenClose() 
{
  // Create initial data set
  shared_ptr<Path> dirFile =
      createTempDir(L"TestIndexReader.testFilesOpenClose");
  shared_ptr<Directory> dir = newFSDirectory(dirFile);

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  addDoc(writer, L"test");
  delete writer;
  delete dir;

  // Try to erase the data - this ensures that the writer closed all files
  IOUtils::rm({dirFile});
  dir = newFSDirectory(dirFile);

  // Now create the data set again, just as before
  writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setOpenMode(OpenMode::CREATE));
  addDoc(writer, L"test");
  delete writer;
  delete dir;

  // Now open existing directory and test that reader closes all files
  dir = newFSDirectory(dirFile);
  shared_ptr<DirectoryReader> reader1 = DirectoryReader::open(dir);
  reader1->close();
  delete dir;

  // The following will fail if reader did not close
  // all files
  IOUtils::rm({dirFile});
}

void TestDirectoryReader::testOpenReaderAfterDelete() 
{
  shared_ptr<Path> dirFile = createTempDir(L"deletetest");
  shared_ptr<Directory> dir = newFSDirectory(dirFile);
  if (std::dynamic_pointer_cast<BaseDirectoryWrapper>(dir) != nullptr) {
    (std::static_pointer_cast<BaseDirectoryWrapper>(dir))
        ->setCheckIndexOnClose(
            false); // we will hit NoSuchFileException in MDW since we nuked it!
  }
  try {
    DirectoryReader::open(dir);
    fail(L"expected FileNotFoundException/NoSuchFileException");
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileNotFoundException | NoSuchFileException e) {
    // expected
  }

  IOUtils::rm({dirFile});

  // Make sure we still get a CorruptIndexException (not NPE):
  try {
    DirectoryReader::open(dir);
    fail(L"expected FileNotFoundException/NoSuchFileException");
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (FileNotFoundException | NoSuchFileException e) {
    // expected
  }

  delete dir;
}

void TestDirectoryReader::addDocumentWithFields(
    shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType3 = make_shared<FieldType>();
  customType3->setStored(true);
  doc->push_back(newStringField(L"keyword", L"test1", Field::Store::YES));
  doc->push_back(newTextField(L"text", L"test1", Field::Store::YES));
  doc->push_back(newField(L"unindexed", L"test1", customType3));
  doc->push_back(
      make_shared<TextField>(L"unstored", L"test1", Field::Store::NO));
  writer->addDocument(doc);
}

void TestDirectoryReader::addDocumentWithDifferentFields(
    shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();

  shared_ptr<FieldType> customType3 = make_shared<FieldType>();
  customType3->setStored(true);
  doc->push_back(newStringField(L"keyword2", L"test1", Field::Store::YES));
  doc->push_back(newTextField(L"text2", L"test1", Field::Store::YES));
  doc->push_back(newField(L"unindexed2", L"test1", customType3));
  doc->push_back(
      make_shared<TextField>(L"unstored2", L"test1", Field::Store::NO));
  writer->addDocument(doc);
}

void TestDirectoryReader::addDocumentWithTermVectorFields(
    shared_ptr<IndexWriter> writer) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType5 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType5->setStoreTermVectors(true);
  shared_ptr<FieldType> customType6 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType6->setStoreTermVectors(true);
  customType6->setStoreTermVectorOffsets(true);
  shared_ptr<FieldType> customType7 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType7->setStoreTermVectors(true);
  customType7->setStoreTermVectorPositions(true);
  shared_ptr<FieldType> customType8 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType8->setStoreTermVectors(true);
  customType8->setStoreTermVectorOffsets(true);
  customType8->setStoreTermVectorPositions(true);
  doc->push_back(newTextField(L"tvnot", L"tvnot", Field::Store::YES));
  doc->push_back(newField(L"termvector", L"termvector", customType5));
  doc->push_back(newField(L"tvoffset", L"tvoffset", customType6));
  doc->push_back(newField(L"tvposition", L"tvposition", customType7));
  doc->push_back(
      newField(L"tvpositionoffset", L"tvpositionoffset", customType8));

  writer->addDocument(doc);
}

void TestDirectoryReader::addDoc(shared_ptr<IndexWriter> writer,
                                 const wstring &value) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"content", value, Field::Store::NO));
  writer->addDocument(doc);
}

void TestDirectoryReader::assertIndexEquals(
    shared_ptr<DirectoryReader> index1,
    shared_ptr<DirectoryReader> index2) 
{
  assertEquals(L"IndexReaders have different values for numDocs.",
               index1->numDocs(), index2->numDocs());
  assertEquals(L"IndexReaders have different values for maxDoc.",
               index1->maxDoc(), index2->maxDoc());
  assertEquals(L"Only one IndexReader has deletions.", index1->hasDeletions(),
               index2->hasDeletions());
  assertEquals(L"Single segment test differs.", index1->leaves()->size() == 1,
               index2->leaves()->size() == 1);

  // check field names
  shared_ptr<FieldInfos> fieldInfos1 = MultiFields::getMergedFieldInfos(index1);
  shared_ptr<FieldInfos> fieldInfos2 = MultiFields::getMergedFieldInfos(index2);
  assertEquals(L"IndexReaders have different numbers of fields.",
               fieldInfos1->size(), fieldInfos2->size());
  constexpr int numFields = fieldInfos1->size();
  for (int fieldID = 0; fieldID < numFields; fieldID++) {
    shared_ptr<FieldInfo> *const fieldInfo1 = fieldInfos1->fieldInfo(fieldID);
    shared_ptr<FieldInfo> *const fieldInfo2 = fieldInfos2->fieldInfo(fieldID);
    assertEquals(L"Different field names.", fieldInfo1->name, fieldInfo2->name);
  }

  // check norms
  for (auto fieldInfo : fieldInfos1) {
    wstring curField = fieldInfo->name;
    shared_ptr<NumericDocValues> norms1 =
        MultiDocValues::getNormValues(index1, curField);
    shared_ptr<NumericDocValues> norms2 =
        MultiDocValues::getNormValues(index2, curField);
    if (norms1 != nullptr && norms2 != nullptr) {
      // todo: generalize this (like TestDuelingCodecs assert)
      while (true) {
        int docID = norms1->nextDoc();
        TestUtil::assertEquals(docID, norms2->nextDoc());
        if (docID == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        assertEquals(L"Norm different for doc " + to_wstring(docID) +
                         L" and field '" + curField + L"'.",
                     norms1->longValue(), norms2->longValue());
      }
    } else {
      assertNull(norms1);
      assertNull(norms2);
    }
  }

  // check deletions
  shared_ptr<Bits> *const liveDocs1 = MultiFields::getLiveDocs(index1);
  shared_ptr<Bits> *const liveDocs2 = MultiFields::getLiveDocs(index2);
  for (int i = 0; i < index1->maxDoc(); i++) {
    assertEquals(L"Doc " + to_wstring(i) + L" only deleted in one index.",
                 liveDocs1 == nullptr || !liveDocs1->get(i),
                 liveDocs2 == nullptr || !liveDocs2->get(i));
  }

  // check stored fields
  for (int i = 0; i < index1->maxDoc(); i++) {
    if (liveDocs1 == nullptr || liveDocs1->get(i)) {
      shared_ptr<Document> doc1 = index1->document(i);
      shared_ptr<Document> doc2 = index2->document(i);
      deque<std::shared_ptr<IndexableField>> field1 = doc1->getFields();
      deque<std::shared_ptr<IndexableField>> field2 = doc2->getFields();
      assertEquals(L"Different numbers of fields for doc " + to_wstring(i) +
                       L".",
                   field1.size(), field2.size());
      deque<std::shared_ptr<IndexableField>>::const_iterator itField1 =
          field1.begin();
      deque<std::shared_ptr<IndexableField>>::const_iterator itField2 =
          field2.begin();
      while (itField1 != field1.end()) {
        shared_ptr<Field> curField1 = (Field)*itField1;
        // C++ TODO: Java iterators are only converted within the context of
        // 'while' and 'for' loops:
        shared_ptr<Field> curField2 =
            std::static_pointer_cast<Field>(itField2.next());
        assertEquals(L"Different fields names for doc " + to_wstring(i) + L".",
                     curField1->name(), curField2->name());
        assertEquals(L"Different field values for doc " + to_wstring(i) + L".",
                     curField1->stringValue(), curField2->stringValue());
        itField1++;
      }
    }
  }

  // check dictionary and posting lists
  shared_ptr<Fields> fields1 = MultiFields::getFields(index1);
  shared_ptr<Fields> fields2 = MultiFields::getFields(index2);
  org::apache::lucene::index::Fields::const_iterator fenum2 = fields2->begin();
  for (auto field1 : fields1) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    assertEquals(L"Different fields", field1, fenum2->next());
    shared_ptr<Terms> terms1 = fields1->terms(field1);
    if (terms1 == nullptr) {
      assertNull(fields2->terms(field1));
      continue;
    }
    shared_ptr<TermsEnum> enum1 = terms1->begin();

    shared_ptr<Terms> terms2 = fields2->terms(field1);
    assertNotNull(terms2);
    shared_ptr<TermsEnum> enum2 = terms2->begin();

    while (enum1->next() != nullptr) {
      assertEquals(L"Different terms", enum1->term(), enum2->next());
      shared_ptr<PostingsEnum> tp1 =
          enum1->postings(nullptr, PostingsEnum::ALL);
      shared_ptr<PostingsEnum> tp2 =
          enum2->postings(nullptr, PostingsEnum::ALL);

      while (tp1->nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
        assertTrue(tp2->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
        assertEquals(L"Different doc id in postinglist of term " +
                         enum1->term() + L".",
                     tp1->docID(), tp2->docID());
        assertEquals(L"Different term frequence in postinglist of term " +
                         enum1->term() + L".",
                     tp1->freq(), tp2->freq());
        for (int i = 0; i < tp1->freq(); i++) {
          assertEquals(L"Different positions in postinglist of term " +
                           enum1->term() + L".",
                       tp1->nextPosition(), tp2->nextPosition());
        }
      }
    }
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(fenum2->hasNext());
}

void TestDirectoryReader::testGetIndexCommit() 
{

  shared_ptr<Directory> d = newDirectory();

  // set up writer
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setMaxBufferedDocs(2)
             ->setMergePolicy(newLogMergePolicy(10)));
  for (int i = 0; i < 27; i++) {
    addDocumentWithFields(writer);
  }
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(d);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(d);
  shared_ptr<IndexCommit> c = r->getIndexCommit();

  TestUtil::assertEquals(sis->getSegmentsFileName(), c->getSegmentsFileName());

  assertTrue(c->equals(r->getIndexCommit()));

  // Change the index
  writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setOpenMode(OpenMode::APPEND)
             ->setMaxBufferedDocs(2)
             ->setMergePolicy(newLogMergePolicy(10)));
  for (int i = 0; i < 7; i++) {
    addDocumentWithFields(writer);
  }
  delete writer;

  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNotNull(r2);
  assertFalse(c->equals(r2->getIndexCommit()));
  assertFalse(r2->getIndexCommit()->getSegmentCount() == 1);
  r2->close();

  writer = make_shared<IndexWriter>(
      d, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
             ->setOpenMode(OpenMode::APPEND));
  writer->forceMerge(1);
  delete writer;

  r2 = DirectoryReader::openIfChanged(r);
  assertNotNull(r2);
  assertNull(DirectoryReader::openIfChanged(r2));
  TestUtil::assertEquals(1, r2->getIndexCommit()->getSegmentCount());

  r->close();
  r2->close();
  delete d;
}

shared_ptr<Document> TestDirectoryReader::createDocument(const wstring &id)
{
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setTokenized(false);
  customType->setOmitNorms(true);

  doc->push_back(newField(L"id", id, customType));
  return doc;
}

void TestDirectoryReader::testNoDir() 
{
  shared_ptr<Path> tempDir = createTempDir(L"doesnotexist");
  shared_ptr<Directory> dir = newFSDirectory(tempDir);
  expectThrows(IndexNotFoundException::typeid,
               [&]() { DirectoryReader::open(dir); });
  delete dir;
}

void TestDirectoryReader::testNoDupCommitFileNames() 
{

  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMaxBufferedDocs(2));
  writer->addDocument(createDocument(L"a"));
  writer->addDocument(createDocument(L"a"));
  writer->addDocument(createDocument(L"a"));
  delete writer;

  shared_ptr<deque<std::shared_ptr<IndexCommit>>> commits =
      DirectoryReader::listCommits(dir);
  for (auto commit : commits) {
    shared_ptr<deque<wstring>> files = commit->getFileNames();
    unordered_set<wstring> seen = unordered_set<wstring>();
    for (auto fileName : files) {
      assertTrue(L"file " + fileName + L" was duplicated",
                 !find(seen.begin(), seen.end(), fileName) != seen.end());
      seen.insert(fileName);
    }
  }

  delete dir;
}

void TestDirectoryReader::testUniqueTermCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(
      L"field", L"a b c d e f g h i j k l m n o p q r s t u v w x y z",
      Field::Store::NO));
  doc->push_back(
      newTextField(L"number", L"0 1 2 3 4 5 6 7 8 9", Field::Store::NO));
  writer->addDocument(doc);
  writer->addDocument(doc);
  writer->commit();

  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  shared_ptr<LeafReader> r1 = getOnlyLeafReader(r);
  TestUtil::assertEquals(26, r1->terms(L"field")->size());
  TestUtil::assertEquals(10, r1->terms(L"number")->size());
  writer->addDocument(doc);
  writer->commit();
  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNotNull(r2);
  r->close();

  for (shared_ptr<LeafReaderContext> s : r2->leaves()) {
    TestUtil::assertEquals(26, s->reader()->terms(L"field")->size());
    TestUtil::assertEquals(10, s->reader()->terms(L"number")->size());
  }
  r2->close();
  delete writer;
  delete dir;
}

void TestDirectoryReader::testPrepareCommitIsCurrent() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->commit();
  shared_ptr<Document> doc = make_shared<Document>();
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  assertTrue(r->isCurrent());
  writer->addDocument(doc);
  writer->prepareCommit();
  assertTrue(r->isCurrent());
  shared_ptr<DirectoryReader> r2 = DirectoryReader::openIfChanged(r);
  assertNull(r2);
  writer->commit();
  assertFalse(r->isCurrent());
  delete writer;
  r->close();
  delete dir;
}

void TestDirectoryReader::testListCommits() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(nullptr)->setIndexDeletionPolicy(
               make_shared<SnapshotDeletionPolicy>(
                   make_shared<KeepOnlyLastCommitDeletionPolicy>())));
  shared_ptr<SnapshotDeletionPolicy> sdp =
      std::static_pointer_cast<SnapshotDeletionPolicy>(
          writer->getConfig()->getIndexDeletionPolicy());
  writer->addDocument(make_shared<Document>());
  writer->commit();
  sdp->snapshot();
  writer->addDocument(make_shared<Document>());
  writer->commit();
  sdp->snapshot();
  writer->addDocument(make_shared<Document>());
  writer->commit();
  sdp->snapshot();
  delete writer;
  int64_t currentGen = 0;
  for (auto ic : DirectoryReader::listCommits(dir)) {
    assertTrue(L"currentGen=" + to_wstring(currentGen) + L" commitGen=" +
                   to_wstring(ic->getGeneration()),
               currentGen < ic->getGeneration());
    currentGen = ic->getGeneration();
  }
  delete dir;
}

void TestDirectoryReader::testTotalTermFreqCached() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a a b", Field::Store::NO));
  writer->addDocument(d);
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  try {
    // Make sure codec impls totalTermFreq (eg PreFlex doesn't)
    Assume::assumeTrue(r->totalTermFreq(make_shared<Term>(
                           L"f", make_shared<BytesRef>(L"b"))) != -1);
    TestUtil::assertEquals(1, r->totalTermFreq(make_shared<Term>(
                                  L"f", make_shared<BytesRef>(L"b"))));
    TestUtil::assertEquals(2, r->totalTermFreq(make_shared<Term>(
                                  L"f", make_shared<BytesRef>(L"a"))));
    TestUtil::assertEquals(1, r->totalTermFreq(make_shared<Term>(
                                  L"f", make_shared<BytesRef>(L"b"))));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    r->close();
    delete dir;
  }
}

void TestDirectoryReader::testGetSumDocFreq() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a", Field::Store::NO));
  writer->addDocument(d);
  d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"b", Field::Store::NO));
  writer->addDocument(d);
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  try {
    // Make sure codec impls getSumDocFreq (eg PreFlex doesn't)
    Assume::assumeTrue(r->getSumDocFreq(L"f") != -1);
    TestUtil::assertEquals(2, r->getSumDocFreq(L"f"));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    r->close();
    delete dir;
  }
}

void TestDirectoryReader::testGetDocCount() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a", Field::Store::NO));
  writer->addDocument(d);
  d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a", Field::Store::NO));
  writer->addDocument(d);
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  try {
    // Make sure codec impls getSumDocFreq (eg PreFlex doesn't)
    Assume::assumeTrue(r->getDocCount(L"f") != -1);
    TestUtil::assertEquals(2, r->getDocCount(L"f"));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    r->close();
    delete dir;
  }
}

void TestDirectoryReader::testGetSumTotalTermFreq() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a b b", Field::Store::NO));
  writer->addDocument(d);
  d = make_shared<Document>();
  d->push_back(newTextField(L"f", L"a a b", Field::Store::NO));
  writer->addDocument(d);
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  try {
    // Make sure codec impls getSumDocFreq (eg PreFlex doesn't)
    Assume::assumeTrue(r->getSumTotalTermFreq(L"f") != -1);
    TestUtil::assertEquals(6, r->getSumTotalTermFreq(L"f"));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    r->close();
    delete dir;
  }
}

void TestDirectoryReader::testReaderFinishedListener() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
               ->setMergePolicy(newLogMergePolicy()));
  (std::static_pointer_cast<LogMergePolicy>(
       writer->getConfig()->getMergePolicy()))
      ->setMergeFactor(3);
  writer->addDocument(make_shared<Document>());
  writer->commit();
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<DirectoryReader> *const reader = writer->getReader();
  const std::deque<int> closeCount = std::deque<int>(1);
  constexpr IndexReader::ClosedListener listener =
      [&](IndexReader::CacheKey key) { closeCount[0]++; };

  reader->getReaderCacheHelper().addClosedListener(listener);

  reader->close();

  // Close the top reader, it's the only one that should be closed
  TestUtil::assertEquals(1, closeCount[0]);
  delete writer;

  shared_ptr<DirectoryReader> reader2 = DirectoryReader::open(dir);
  reader2->getReaderCacheHelper().addClosedListener(listener);

  closeCount[0] = 0;
  reader2->close();
  TestUtil::assertEquals(1, closeCount[0]);
  delete dir;
}

void TestDirectoryReader::testOOBDocID() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(make_shared<Document>());
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  r->document(0);
  expectThrows(invalid_argument::typeid, [&]() { r->document(1); });
  r->close();
  delete dir;
}

void TestDirectoryReader::testTryIncRef() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  assertTrue(r->tryIncRef());
  r->decRef();
  r->close();
  assertFalse(r->tryIncRef());
  delete writer;
  delete dir;
}

void TestDirectoryReader::testStressTryIncRef() throw(IOException,
                                                      InterruptedException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  writer->addDocument(make_shared<Document>());
  writer->commit();
  shared_ptr<DirectoryReader> r = DirectoryReader::open(dir);
  int numThreads = atLeast(2);

  std::deque<std::shared_ptr<IncThread>> threads(numThreads);
  for (int i = 0; i < threads.size(); i++) {
    threads[i] = make_shared<IncThread>(r, random());
    threads[i]->start();
  }
  delay(100);

  assertTrue(r->tryIncRef());
  r->decRef();
  r->close();

  for (int i = 0; i < threads.size(); i++) {
    threads[i]->join();
    assertNull(threads[i]->failed);
  }
  assertFalse(r->tryIncRef());
  delete writer;
  delete dir;
}

TestDirectoryReader::IncThread::IncThread(shared_ptr<IndexReader> toInc,
                                          shared_ptr<Random> random)
    : toInc(toInc), random(random)
{
}

void TestDirectoryReader::IncThread::run()
{
  try {
    while (toInc->tryIncRef()) {
      assertFalse(toInc->hasDeletions());
      toInc->decRef();
    }
    assertFalse(toInc->tryIncRef());
  } catch (const runtime_error &e) {
    failed = e;
  }
}

void TestDirectoryReader::testLoadCertainFields() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newStringField(L"field1", L"foobar", Field::Store::YES));
  doc->push_back(newStringField(L"field2", L"foobaz", Field::Store::YES));
  writer->addDocument(doc);
  shared_ptr<DirectoryReader> r = writer->getReader();
  delete writer;
  shared_ptr<Set<wstring>> fieldsToLoad = unordered_set<wstring>();
  TestUtil::assertEquals(0, r->document(0, fieldsToLoad).getFields()->size());
  fieldsToLoad->add(L"field1");
  shared_ptr<Document> doc2 = r->document(0, fieldsToLoad);
  TestUtil::assertEquals(1, doc2->getFields().size());
  TestUtil::assertEquals(L"foobar", doc2[L"field1"]);
  r->close();
  delete dir;
}

void TestDirectoryReader::testIndexExistsOnNonExistentDirectory() throw(
    runtime_error)
{
  shared_ptr<Path> tempDir =
      createTempDir(L"testIndexExistsOnNonExistentDirectory");
  shared_ptr<Directory> dir = newFSDirectory(tempDir);
  assertFalse(DirectoryReader::indexExists(dir));
  delete dir;
}
} // namespace org::apache::lucene::index