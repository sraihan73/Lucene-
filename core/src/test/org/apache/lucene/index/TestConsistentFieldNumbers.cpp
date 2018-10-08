using namespace std;

#include "TestConsistentFieldNumbers.h"

namespace org::apache::lucene::index
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using TextField = org::apache::lucene::document::TextField;
using Directory = org::apache::lucene::store::Directory;
using FailOnNonBulkMergesInfoStream =
    org::apache::lucene::util::FailOnNonBulkMergesInfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSameFieldNumbersAcrossSegments() throws
// Exception
void TestConsistentFieldNumbers::testSameFieldNumbersAcrossSegments() throw(
    runtime_error)
{
  for (int i = 0; i < 2; i++) {
    shared_ptr<Directory> dir = newDirectory();
    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMergePolicy(NoMergePolicy::INSTANCE));

    shared_ptr<Document> d1 = make_shared<Document>();
    d1->push_back(
        make_shared<StringField>(L"f1", L"first field", Field::Store::YES));
    d1->push_back(
        make_shared<StringField>(L"f2", L"second field", Field::Store::YES));
    writer->addDocument(d1);

    if (i == 1) {
      delete writer;
      writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(NoMergePolicy::INSTANCE));
    } else {
      writer->commit();
    }

    shared_ptr<Document> d2 = make_shared<Document>();
    shared_ptr<FieldType> customType2 =
        make_shared<FieldType>(TextField::TYPE_STORED);
    customType2->setStoreTermVectors(true);
    d2->push_back(
        make_shared<TextField>(L"f2", L"second field", Field::Store::NO));
    d2->push_back(make_shared<Field>(L"f1", L"first field", customType2));
    d2->push_back(
        make_shared<TextField>(L"f3", L"third field", Field::Store::NO));
    d2->push_back(
        make_shared<TextField>(L"f4", L"fourth field", Field::Store::NO));
    writer->addDocument(d2);

    delete writer;

    shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
    assertEquals(2, sis->size());

    shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
    shared_ptr<FieldInfos> fis2 = IndexWriter::readFieldInfos(sis->info(1));

    assertEquals(L"f1", fis1->fieldInfo(0)->name);
    assertEquals(L"f2", fis1->fieldInfo(1)->name);
    assertEquals(L"f1", fis2->fieldInfo(0)->name);
    assertEquals(L"f2", fis2->fieldInfo(1)->name);
    assertEquals(L"f3", fis2->fieldInfo(2)->name);
    assertEquals(L"f4", fis2->fieldInfo(3)->name);

    writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
    writer->forceMerge(1);
    delete writer;

    sis = SegmentInfos::readLatestCommit(dir);
    assertEquals(1, sis->size());

    shared_ptr<FieldInfos> fis3 = IndexWriter::readFieldInfos(sis->info(0));

    assertEquals(L"f1", fis3->fieldInfo(0)->name);
    assertEquals(L"f2", fis3->fieldInfo(1)->name);
    assertEquals(L"f3", fis3->fieldInfo(2)->name);
    assertEquals(L"f4", fis3->fieldInfo(3)->name);

    delete dir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testAddIndexes() throws Exception
void TestConsistentFieldNumbers::testAddIndexes() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergePolicy(NoMergePolicy::INSTANCE));

  shared_ptr<Document> d1 = make_shared<Document>();
  d1->push_back(
      make_shared<TextField>(L"f1", L"first field", Field::Store::YES));
  d1->push_back(
      make_shared<TextField>(L"f2", L"second field", Field::Store::YES));
  writer->addDocument(d1);

  delete writer;
  writer = make_shared<IndexWriter>(
      dir2, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergePolicy(NoMergePolicy::INSTANCE));

  shared_ptr<Document> d2 = make_shared<Document>();
  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setStoreTermVectors(true);
  d2->push_back(
      make_shared<TextField>(L"f2", L"second field", Field::Store::YES));
  d2->push_back(make_shared<Field>(L"f1", L"first field", customType2));
  d2->push_back(
      make_shared<TextField>(L"f3", L"third field", Field::Store::YES));
  d2->push_back(
      make_shared<TextField>(L"f4", L"fourth field", Field::Store::YES));
  writer->addDocument(d2);

  delete writer;

  writer = make_shared<IndexWriter>(
      dir1, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                ->setMergePolicy(NoMergePolicy::INSTANCE));
  writer->addIndexes({dir2});
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir1);
  assertEquals(2, sis->size());

  shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
  shared_ptr<FieldInfos> fis2 = IndexWriter::readFieldInfos(sis->info(1));

  assertEquals(L"f1", fis1->fieldInfo(0)->name);
  assertEquals(L"f2", fis1->fieldInfo(1)->name);
  // make sure the ordering of the "external" segment is preserved
  assertEquals(L"f2", fis2->fieldInfo(0)->name);
  assertEquals(L"f1", fis2->fieldInfo(1)->name);
  assertEquals(L"f3", fis2->fieldInfo(2)->name);
  assertEquals(L"f4", fis2->fieldInfo(3)->name);

  delete dir1;
  delete dir2;
}

void TestConsistentFieldNumbers::testFieldNumberGaps() 
{
  int numIters = atLeast(13);
  for (int i = 0; i < numIters; i++) {
    shared_ptr<Directory> dir = newDirectory();
    {
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(NoMergePolicy::INSTANCE));
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(
          make_shared<TextField>(L"f1", L"d1 first field", Field::Store::YES));
      d->push_back(
          make_shared<TextField>(L"f2", L"d1 second field", Field::Store::YES));
      writer->addDocument(d);
      delete writer;
      shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
      assertEquals(1, sis->size());
      shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
      assertEquals(L"f1", fis1->fieldInfo(0)->name);
      assertEquals(L"f2", fis1->fieldInfo(1)->name);
    }

    {
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(NoMergePolicy::INSTANCE));
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(
          make_shared<TextField>(L"f1", L"d2 first field", Field::Store::YES));
      d->push_back(make_shared<StoredField>(L"f3", std::deque<char>{1, 2, 3}));
      writer->addDocument(d);
      delete writer;
      shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
      assertEquals(2, sis->size());
      shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
      shared_ptr<FieldInfos> fis2 = IndexWriter::readFieldInfos(sis->info(1));
      assertEquals(L"f1", fis1->fieldInfo(0)->name);
      assertEquals(L"f2", fis1->fieldInfo(1)->name);
      assertEquals(L"f1", fis2->fieldInfo(0)->name);
      assertNull(fis2->fieldInfo(1));
      assertEquals(L"f3", fis2->fieldInfo(2)->name);
    }

    {
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(NoMergePolicy::INSTANCE));
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(
          make_shared<TextField>(L"f1", L"d3 first field", Field::Store::YES));
      d->push_back(
          make_shared<TextField>(L"f2", L"d3 second field", Field::Store::YES));
      d->push_back(
          make_shared<StoredField>(L"f3", std::deque<char>{1, 2, 3, 4, 5}));
      writer->addDocument(d);
      delete writer;
      shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
      assertEquals(3, sis->size());
      shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
      shared_ptr<FieldInfos> fis2 = IndexWriter::readFieldInfos(sis->info(1));
      shared_ptr<FieldInfos> fis3 = IndexWriter::readFieldInfos(sis->info(2));
      assertEquals(L"f1", fis1->fieldInfo(0)->name);
      assertEquals(L"f2", fis1->fieldInfo(1)->name);
      assertEquals(L"f1", fis2->fieldInfo(0)->name);
      assertNull(fis2->fieldInfo(1));
      assertEquals(L"f3", fis2->fieldInfo(2)->name);
      assertEquals(L"f1", fis3->fieldInfo(0)->name);
      assertEquals(L"f2", fis3->fieldInfo(1)->name);
      assertEquals(L"f3", fis3->fieldInfo(2)->name);
    }

    {
      shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
          dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                   ->setMergePolicy(NoMergePolicy::INSTANCE));
      writer->deleteDocuments({make_shared<Term>(L"f1", L"d1")});
      // nuke the first segment entirely so that the segment with gaps is
      // loaded first!
      writer->forceMergeDeletes();
      delete writer;
    }

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
        dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
                 ->setMergePolicy(make_shared<LogByteSizeMergePolicy>())
                 ->setInfoStream(make_shared<FailOnNonBulkMergesInfoStream>()));
    writer->forceMerge(1);
    delete writer;

    shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
    assertEquals(1, sis->size());
    shared_ptr<FieldInfos> fis1 = IndexWriter::readFieldInfos(sis->info(0));
    assertEquals(L"f1", fis1->fieldInfo(0)->name);
    assertEquals(L"f2", fis1->fieldInfo(1)->name);
    assertEquals(L"f3", fis1->fieldInfo(2)->name);
    delete dir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testManyFields() throws Exception
void TestConsistentFieldNumbers::testManyFields() 
{
  constexpr int NUM_DOCS = atLeast(200);
  constexpr int MAX_FIELDS = atLeast(50);

  // C++ NOTE: The following call to the 'RectangularVectors' helper class
  // reproduces the rectangular array initialization that is automatic in Java:
  // ORIGINAL LINE: int[][] docs = new int[NUM_DOCS][4];
  std::deque<std::deque<int>> docs =
      RectangularVectors::ReturnRectangularIntVector(NUM_DOCS, 4);
  for (int i = 0; i < docs.size(); i++) {
    for (int j = 0; j < docs[i].size(); j++) {
      docs[i][j] = random()->nextInt(MAX_FIELDS);
    }
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  for (int i = 0; i < NUM_DOCS; i++) {
    shared_ptr<Document> d = make_shared<Document>();
    for (int j = 0; j < docs[i].size(); j++) {
      d->push_back(getField(docs[i][j]));
    }

    writer->addDocument(d);
  }

  writer->forceMerge(1);
  delete writer;

  shared_ptr<SegmentInfos> sis = SegmentInfos::readLatestCommit(dir);
  for (auto si : sis) {
    shared_ptr<FieldInfos> fis = IndexWriter::readFieldInfos(si);

    for (auto fi : fis) {
      shared_ptr<Field> expected = getField(stoi(fi->name));
      assertEquals(expected->fieldType()->indexOptions(),
                   fi->getIndexOptions());
      assertEquals(expected->fieldType()->storeTermVectors(), fi->hasVectors());
    }
  }

  delete dir;
}

shared_ptr<Field> TestConsistentFieldNumbers::getField(int number)
{
  int mode = number % 16;
  wstring fieldName = L"" + to_wstring(number);
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);

  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType2->setTokenized(false);

  shared_ptr<FieldType> customType3 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType3->setTokenized(false);

  shared_ptr<FieldType> customType4 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType4->setTokenized(false);
  customType4->setStoreTermVectors(true);
  customType4->setStoreTermVectorOffsets(true);

  shared_ptr<FieldType> customType5 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType5->setStoreTermVectors(true);
  customType5->setStoreTermVectorOffsets(true);

  shared_ptr<FieldType> customType6 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType6->setTokenized(false);
  customType6->setStoreTermVectors(true);
  customType6->setStoreTermVectorOffsets(true);

  shared_ptr<FieldType> customType7 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType7->setTokenized(false);
  customType7->setStoreTermVectors(true);
  customType7->setStoreTermVectorOffsets(true);

  shared_ptr<FieldType> customType8 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType8->setTokenized(false);
  customType8->setStoreTermVectors(true);
  customType8->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType9 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType9->setStoreTermVectors(true);
  customType9->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType10 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType10->setTokenized(false);
  customType10->setStoreTermVectors(true);
  customType10->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType11 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType11->setTokenized(false);
  customType11->setStoreTermVectors(true);
  customType11->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType12 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType12->setStoreTermVectors(true);
  customType12->setStoreTermVectorOffsets(true);
  customType12->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType13 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType13->setStoreTermVectors(true);
  customType13->setStoreTermVectorOffsets(true);
  customType13->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType14 =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType14->setTokenized(false);
  customType14->setStoreTermVectors(true);
  customType14->setStoreTermVectorOffsets(true);
  customType14->setStoreTermVectorPositions(true);

  shared_ptr<FieldType> customType15 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType15->setTokenized(false);
  customType15->setStoreTermVectors(true);
  customType15->setStoreTermVectorOffsets(true);
  customType15->setStoreTermVectorPositions(true);

  switch (mode) {
  case 0:
    return make_shared<Field>(fieldName, L"some text", customType);
  case 1:
    return make_shared<TextField>(fieldName, L"some text", Field::Store::NO);
  case 2:
    return make_shared<Field>(fieldName, L"some text", customType2);
  case 3:
    return make_shared<Field>(fieldName, L"some text", customType3);
  case 4:
    return make_shared<Field>(fieldName, L"some text", customType4);
  case 5:
    return make_shared<Field>(fieldName, L"some text", customType5);
  case 6:
    return make_shared<Field>(fieldName, L"some text", customType6);
  case 7:
    return make_shared<Field>(fieldName, L"some text", customType7);
  case 8:
    return make_shared<Field>(fieldName, L"some text", customType8);
  case 9:
    return make_shared<Field>(fieldName, L"some text", customType9);
  case 10:
    return make_shared<Field>(fieldName, L"some text", customType10);
  case 11:
    return make_shared<Field>(fieldName, L"some text", customType11);
  case 12:
    return make_shared<Field>(fieldName, L"some text", customType12);
  case 13:
    return make_shared<Field>(fieldName, L"some text", customType13);
  case 14:
    return make_shared<Field>(fieldName, L"some text", customType14);
  case 15:
    return make_shared<Field>(fieldName, L"some text", customType15);
  default:
    return nullptr;
  }
}
} // namespace org::apache::lucene::index