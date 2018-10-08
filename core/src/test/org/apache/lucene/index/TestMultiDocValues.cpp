using namespace std;

#include "TestMultiDocValues.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

void TestMultiDocValues::testNumerics() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = make_shared<NumericDocValuesField>(L"numbers", 0);
  doc->push_back(field);

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    field->setLongValue(random()->nextLong());
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<NumericDocValues> multi =
      MultiDocValues::getNumericValues(ir, L"numbers");
  shared_ptr<NumericDocValues> single = merged->getNumericDocValues(L"numbers");
  for (int i = 0; i < numDocs; i++) {
    TestUtil::assertEquals(i, multi->nextDoc());
    TestUtil::assertEquals(i, single->nextDoc());
    TestUtil::assertEquals(single->longValue(), multi->longValue());
  }
  testRandomAdvance(merged->getNumericDocValues(L"numbers"),
                    MultiDocValues::getNumericValues(ir, L"numbers"));
  testRandomAdvanceExact(merged->getNumericDocValues(L"numbers"),
                         MultiDocValues::getNumericValues(ir, L"numbers"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testBinary() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<BinaryDocValuesField>(L"bytes", make_shared<BytesRef>());
  doc->push_back(field);

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<BytesRef> ref =
        make_shared<BytesRef>(TestUtil::randomUnicodeString(random()));
    field->setBytesValue(ref);
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<BinaryDocValues> multi =
      MultiDocValues::getBinaryValues(ir, L"bytes");
  shared_ptr<BinaryDocValues> single = merged->getBinaryDocValues(L"bytes");
  for (int i = 0; i < numDocs; i++) {
    TestUtil::assertEquals(i, multi->nextDoc());
    TestUtil::assertEquals(i, single->nextDoc());
    shared_ptr<BytesRef> *const expected =
        BytesRef::deepCopyOf(single->binaryValue());
    shared_ptr<BytesRef> *const actual = multi->binaryValue();
    TestUtil::assertEquals(expected, actual);
  }
  testRandomAdvance(merged->getBinaryDocValues(L"bytes"),
                    MultiDocValues::getBinaryValues(ir, L"bytes"));
  testRandomAdvanceExact(merged->getBinaryDocValues(L"bytes"),
                         MultiDocValues::getBinaryValues(ir, L"bytes"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testSorted() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<SortedDocValuesField>(L"bytes", make_shared<BytesRef>());
  doc->push_back(field);

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<BytesRef> ref =
        make_shared<BytesRef>(TestUtil::randomUnicodeString(random()));
    field->setBytesValue(ref);
    if (random()->nextInt(7) == 0) {
      iw->addDocument(make_shared<Document>());
    }
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;
  shared_ptr<SortedDocValues> multi =
      MultiDocValues::getSortedValues(ir, L"bytes");
  shared_ptr<SortedDocValues> single = merged->getSortedDocValues(L"bytes");
  TestUtil::assertEquals(single->getValueCount(), multi->getValueCount());
  while (true) {
    TestUtil::assertEquals(single->nextDoc(), multi->nextDoc());
    if (single->docID() == DocIdSetIterator::NO_MORE_DOCS) {
      break;
    }

    // check value
    shared_ptr<BytesRef> *const expected =
        BytesRef::deepCopyOf(single->binaryValue());
    shared_ptr<BytesRef> *const actual = multi->binaryValue();
    TestUtil::assertEquals(expected, actual);

    // check ord
    TestUtil::assertEquals(single->ordValue(), multi->ordValue());
  }
  testRandomAdvance(merged->getSortedDocValues(L"bytes"),
                    MultiDocValues::getSortedValues(ir, L"bytes"));
  testRandomAdvanceExact(merged->getSortedDocValues(L"bytes"),
                         MultiDocValues::getSortedValues(ir, L"bytes"),
                         merged->maxDoc());
  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testSortedWithLotsOfDups() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<SortedDocValuesField>(L"bytes", make_shared<BytesRef>());
  doc->push_back(field);

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<BytesRef> ref =
        make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2));
    field->setBytesValue(ref);
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<SortedDocValues> multi =
      MultiDocValues::getSortedValues(ir, L"bytes");
  shared_ptr<SortedDocValues> single = merged->getSortedDocValues(L"bytes");
  TestUtil::assertEquals(single->getValueCount(), multi->getValueCount());
  for (int i = 0; i < numDocs; i++) {
    TestUtil::assertEquals(i, multi->nextDoc());
    TestUtil::assertEquals(i, single->nextDoc());
    // check ord
    TestUtil::assertEquals(single->ordValue(), multi->ordValue());
    // check ord value
    shared_ptr<BytesRef> *const expected =
        BytesRef::deepCopyOf(single->binaryValue());
    shared_ptr<BytesRef> *const actual = multi->binaryValue();
    TestUtil::assertEquals(expected, actual);
  }
  testRandomAdvance(merged->getSortedDocValues(L"bytes"),
                    MultiDocValues::getSortedValues(ir, L"bytes"));
  testRandomAdvanceExact(merged->getSortedDocValues(L"bytes"),
                         MultiDocValues::getSortedValues(ir, L"bytes"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testSortedSet() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numValues = random()->nextInt(5);
    for (int j = 0; j < numValues; j++) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"bytes",
          make_shared<BytesRef>(TestUtil::randomUnicodeString(random()))));
    }
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<SortedSetDocValues> multi =
      MultiDocValues::getSortedSetValues(ir, L"bytes");
  shared_ptr<SortedSetDocValues> single =
      merged->getSortedSetDocValues(L"bytes");
  if (multi == nullptr) {
    assertNull(single);
  } else {
    TestUtil::assertEquals(single->getValueCount(), multi->getValueCount());
    // check values
    for (int64_t i = 0; i < single->getValueCount(); i++) {
      shared_ptr<BytesRef> *const expected =
          BytesRef::deepCopyOf(single->lookupOrd(i));
      shared_ptr<BytesRef> *const actual = multi->lookupOrd(i);
      TestUtil::assertEquals(expected, actual);
    }
    // check ord deque
    while (true) {
      int docID = single->nextDoc();
      TestUtil::assertEquals(docID, multi->nextDoc());
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }

      deque<int64_t> expectedList = deque<int64_t>();
      int64_t ord;
      while ((ord = single->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
        expectedList.push_back(ord);
      }

      int upto = 0;
      while ((ord = multi->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
        TestUtil::assertEquals(expectedList[upto], ord);
        upto++;
      }
      TestUtil::assertEquals(expectedList.size(), upto);
    }
  }
  testRandomAdvance(merged->getSortedSetDocValues(L"bytes"),
                    MultiDocValues::getSortedSetValues(ir, L"bytes"));
  testRandomAdvanceExact(merged->getSortedSetDocValues(L"bytes"),
                         MultiDocValues::getSortedSetValues(ir, L"bytes"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testSortedSetWithDups() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numValues = random()->nextInt(5);
    for (int j = 0; j < numValues; j++) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"bytes",
          make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2))));
    }
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<SortedSetDocValues> multi =
      MultiDocValues::getSortedSetValues(ir, L"bytes");
  shared_ptr<SortedSetDocValues> single =
      merged->getSortedSetDocValues(L"bytes");
  if (multi == nullptr) {
    assertNull(single);
  } else {
    TestUtil::assertEquals(single->getValueCount(), multi->getValueCount());
    // check values
    for (int64_t i = 0; i < single->getValueCount(); i++) {
      shared_ptr<BytesRef> *const expected =
          BytesRef::deepCopyOf(single->lookupOrd(i));
      shared_ptr<BytesRef> *const actual = multi->lookupOrd(i);
      TestUtil::assertEquals(expected, actual);
    }
    // check ord deque
    while (true) {
      int docID = single->nextDoc();
      TestUtil::assertEquals(docID, multi->nextDoc());
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      deque<int64_t> expectedList = deque<int64_t>();
      int64_t ord;
      while ((ord = single->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
        expectedList.push_back(ord);
      }

      int upto = 0;
      while ((ord = multi->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
        TestUtil::assertEquals(expectedList[upto], ord);
        upto++;
      }
      TestUtil::assertEquals(expectedList.size(), upto);
    }
  }
  testRandomAdvance(merged->getSortedSetDocValues(L"bytes"),
                    MultiDocValues::getSortedSetValues(ir, L"bytes"));
  testRandomAdvanceExact(merged->getSortedSetDocValues(L"bytes"),
                         MultiDocValues::getSortedSetValues(ir, L"bytes"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testSortedNumeric() 
{
  shared_ptr<Directory> dir = newDirectory();

  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), nullptr);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  int numDocs = TEST_NIGHTLY ? atLeast(500) : atLeast(50);
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numValues = random()->nextInt(5);
    for (int j = 0; j < numValues; j++) {
      doc->push_back(make_shared<SortedNumericDocValuesField>(
          L"nums",
          TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                             numeric_limits<int64_t>::max())));
    }
    iw->addDocument(doc);
    if (random()->nextInt(17) == 0) {
      iw->commit();
    }
  }
  shared_ptr<DirectoryReader> ir = iw->getReader();
  iw->forceMerge(1);
  shared_ptr<DirectoryReader> ir2 = iw->getReader();
  shared_ptr<LeafReader> merged = getOnlyLeafReader(ir2);
  delete iw;

  shared_ptr<SortedNumericDocValues> multi =
      MultiDocValues::getSortedNumericValues(ir, L"nums");
  shared_ptr<SortedNumericDocValues> single =
      merged->getSortedNumericDocValues(L"nums");
  if (multi == nullptr) {
    assertNull(single);
  } else {
    // check values
    for (int i = 0; i < numDocs; i++) {
      if (i > single->docID()) {
        TestUtil::assertEquals(single->nextDoc(), multi->nextDoc());
      }
      if (i == single->docID()) {
        TestUtil::assertEquals(single->docValueCount(), multi->docValueCount());
        for (int j = 0; j < single->docValueCount(); j++) {
          TestUtil::assertEquals(single->nextValue(), multi->nextValue());
        }
      }
    }
  }
  testRandomAdvance(merged->getSortedNumericDocValues(L"nums"),
                    MultiDocValues::getSortedNumericValues(ir, L"nums"));
  testRandomAdvanceExact(merged->getSortedNumericDocValues(L"nums"),
                         MultiDocValues::getSortedNumericValues(ir, L"nums"),
                         merged->maxDoc());

  ir->close();
  ir2->close();
  delete dir;
}

void TestMultiDocValues::testRandomAdvance(
    shared_ptr<DocIdSetIterator> iter1,
    shared_ptr<DocIdSetIterator> iter2) 
{
  TestUtil::assertEquals(-1, iter1->docID());
  TestUtil::assertEquals(-1, iter2->docID());

  while (iter1->docID() != DocIdSetIterator::NO_MORE_DOCS) {
    if (random()->nextBoolean()) {
      TestUtil::assertEquals(iter1->nextDoc(), iter2->nextDoc());
    } else {
      int target = iter1->docID() + TestUtil::nextInt(random(), 1, 100);
      TestUtil::assertEquals(iter1->advance(target), iter2->advance(target));
    }
  }
}

void TestMultiDocValues::testRandomAdvanceExact(
    shared_ptr<DocValuesIterator> iter1, shared_ptr<DocValuesIterator> iter2,
    int maxDoc) 
{
  for (int target = random()->nextInt(min(maxDoc, 10)); target < maxDoc;
       target += random()->nextInt(10)) {
    constexpr bool exists1 = iter1->advanceExact(target);
    constexpr bool exists2 = iter2->advanceExact(target);
    TestUtil::assertEquals(exists1, exists2);
  }
}
} // namespace org::apache::lucene::index