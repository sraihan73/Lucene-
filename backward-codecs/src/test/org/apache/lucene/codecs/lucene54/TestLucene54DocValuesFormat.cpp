using namespace std;

#include "TestLucene54DocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/BinaryDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/NumericDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/SortedDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/SortedNumericDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/SortedSetDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/StoredField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SerialMergeScheduler.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMFile.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMInputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/RAMOutputStream.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../java/org/apache/lucene/codecs/lucene54/Lucene54DocValuesProducer.h"

namespace org::apache::lucene::codecs::lucene54
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using SparseNumericDocValues = org::apache::lucene::codecs::lucene54::
    Lucene54DocValuesProducer::SparseNumericDocValues;
using SparseNumericDocValuesRandomAccessWrapper =
    org::apache::lucene::codecs::lucene54::Lucene54DocValuesProducer::
        SparseNumericDocValuesRandomAccessWrapper;
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
using StoredField = org::apache::lucene::document::StoredField;
using StringField = org::apache::lucene::document::StringField;
using BaseCompressingDocValuesFormatTestCase =
    org::apache::lucene::index::BaseCompressingDocValuesFormatTestCase;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValues = org::apache::lucene::index::DocValues;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using SeekStatus = org::apache::lucene::index::TermsEnum::SeekStatus;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using RAMFile = org::apache::lucene::store::RAMFile;
using RAMInputStream = org::apache::lucene::store::RAMInputStream;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using LongValues = org::apache::lucene::util::LongValues;
using TestUtil = org::apache::lucene::util::TestUtil;

shared_ptr<Codec> TestLucene54DocValuesFormat::getCodec() { return codec; }

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void
// testSortedSetVariableLengthBigVsStoredFields() throws Exception
void TestLucene54DocValuesFormat::
    testSortedSetVariableLengthBigVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(atLeast(300), 1, 32766, 16, 100);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void
// testSortedSetVariableLengthManyVsStoredFields() throws Exception
void TestLucene54DocValuesFormat::
    testSortedSetVariableLengthManyVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedSetVsStoredFields(TestUtil::nextInt(random(), 1024, 2049), 1,
                                  500, 16, 100);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testSortedVariableLengthBigVsStoredFields()
// throws Exception
void TestLucene54DocValuesFormat::
    testSortedVariableLengthBigVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedVsStoredFields(atLeast(300), 1, 1, 32766);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void
// testSortedVariableLengthManyVsStoredFields() throws Exception
void TestLucene54DocValuesFormat::
    testSortedVariableLengthManyVsStoredFields() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSortedVsStoredFields(TestUtil::nextInt(random(), 1024, 2049), 1, 1,
                               500);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testTermsEnumFixedWidth() throws Exception
void TestLucene54DocValuesFormat::testTermsEnumFixedWidth() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestTermsEnumRandom(TestUtil::nextInt(random(), 1025, 5121), 10, 10);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testTermsEnumVariableWidth() throws
// Exception
void TestLucene54DocValuesFormat::testTermsEnumVariableWidth() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestTermsEnumRandom(TestUtil::nextInt(random(), 1025, 5121), 1, 500);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testTermsEnumRandomMany() throws
// Exception
void TestLucene54DocValuesFormat::testTermsEnumRandomMany() 
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestTermsEnumRandom(TestUtil::nextInt(random(), 1025, 8121), 1, 500);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testSparseDocValuesVsStoredFields() throws
// Exception
void TestLucene54DocValuesFormat::testSparseDocValuesVsStoredFields() throw(
    runtime_error)
{
  int numIterations = atLeast(1);
  for (int i = 0; i < numIterations; i++) {
    doTestSparseDocValuesVsStoredFields();
  }
}

void TestLucene54DocValuesFormat::doTestSparseDocValuesVsStoredFields() throw(
    runtime_error)
{
  const std::deque<int64_t> values =
      std::deque<int64_t>(TestUtil::nextInt(random(), 1, 500));
  for (int i = 0; i < values.size(); ++i) {
    values[i] = random()->nextLong();
  }

  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);

  // sparse compression is only enabled if less than 1% of docs have a value
  constexpr int avgGap = 100;

  constexpr int numDocs = atLeast(200);
  for (int i = random()->nextInt(avgGap * 2); i >= 0; --i) {
    writer->addDocument(make_shared<Document>());
  }
  constexpr int maxNumValuesPerDoc =
      random()->nextBoolean() ? 1 : TestUtil::nextInt(random(), 2, 5);
  for (int i = 0; i < numDocs; ++i) {
    shared_ptr<Document> doc = make_shared<Document>();

    // single-valued
    int64_t docValue = values[random()->nextInt(values.size())];
    doc->push_back(make_shared<NumericDocValuesField>(L"numeric", docValue));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<SortedDocValuesField>(
        L"sorted", make_shared<BytesRef>(Long::toString(docValue))));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(make_shared<BinaryDocValuesField>(
        L"binary", make_shared<BytesRef>(Long::toString(docValue))));
    doc->push_back(make_shared<StoredField>(L"value", docValue));

    // multi-valued
    constexpr int numValues =
        TestUtil::nextInt(random(), 1, maxNumValuesPerDoc);
    for (int j = 0; j < numValues; ++j) {
      docValue = values[random()->nextInt(values.size())];
      doc->push_back(make_shared<SortedNumericDocValuesField>(L"sorted_numeric",
                                                              docValue));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"sorted_set", make_shared<BytesRef>(Long::toString(docValue))));
      doc->push_back(make_shared<StoredField>(L"values", docValue));
    }

    writer->addDocument(doc);

    // add a gap
    for (int j = TestUtil::nextInt(random(), 0, avgGap * 2); j >= 0; --j) {
      writer->addDocument(make_shared<Document>());
    }
  }

  if (random()->nextBoolean()) {
    writer->forceMerge(1);
  }

  shared_ptr<IndexReader> *const indexReader = writer->getReader();
  TestUtil::checkReader(indexReader);
  delete writer;

  for (auto context : indexReader->leaves()) {
    shared_ptr<LeafReader> *const reader = context->reader();
    shared_ptr<NumericDocValues> *const numeric =
        DocValues::getNumeric(reader, L"numeric");

    shared_ptr<SortedDocValues> *const sorted =
        DocValues::getSorted(reader, L"sorted");

    shared_ptr<BinaryDocValues> *const binary =
        DocValues::getBinary(reader, L"binary");

    shared_ptr<SortedNumericDocValues> *const sortedNumeric =
        DocValues::getSortedNumeric(reader, L"sorted_numeric");

    shared_ptr<SortedSetDocValues> *const sortedSet =
        DocValues::getSortedSet(reader, L"sorted_set");

    for (int i = 0; i < reader->maxDoc(); ++i) {
      shared_ptr<Document> *const doc = reader->document(i);
      shared_ptr<IndexableField> *const valueField = doc->getField(L"value");
      const optional<int64_t> value =
          valueField == nullptr ? nullopt
                                : valueField->numericValue()->longValue();

      if (!value) {
        assertTrue(to_wstring(numeric->docID()) + L" vs " + to_wstring(i),
                   numeric->docID() < i);
      } else {
        assertEquals(i, numeric->nextDoc());
        assertEquals(i, binary->nextDoc());
        assertEquals(i, sorted->nextDoc());
        assertEquals(value.value(), numeric->longValue());
        assertTrue(sorted->ordValue() >= 0);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertEquals(make_shared<BytesRef>(Long::toString(value)),
                     sorted->lookupOrd(sorted->ordValue()));
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertEquals(make_shared<BytesRef>(Long::toString(value)),
                     binary->binaryValue());
      }

      std::deque<std::shared_ptr<IndexableField>> valuesFields =
          doc->getFields(L"values");
      if (valuesFields.empty()) {
        assertTrue(to_wstring(sortedNumeric->docID()) + L" vs " + to_wstring(i),
                   sortedNumeric->docID() < i);
      } else {
        shared_ptr<Set<int64_t>> *const valueSet = unordered_set<int64_t>();
        for (auto sf : valuesFields) {
          valueSet->add(sf->numericValue()->longValue());
        }

        assertEquals(i, sortedNumeric->nextDoc());
        assertEquals(valuesFields.size(), sortedNumeric->docValueCount());
        for (int j = 0; j < sortedNumeric->docValueCount(); ++j) {
          assertTrue(valueSet->contains(sortedNumeric->nextValue()));
        }
        assertEquals(i, sortedSet->nextDoc());
        int sortedSetCount = 0;
        while (true) {
          int64_t ord = sortedSet->nextOrd();
          if (ord == SortedSetDocValues::NO_MORE_ORDS) {
            break;
          }
          assertTrue(valueSet->contains(StringHelper::fromString<int64_t>(
              sortedSet->lookupOrd(ord)->utf8ToString())));
          sortedSetCount++;
        }
        assertEquals(valueSet->size(), sortedSetCount);
      }
    }
  }

  delete indexReader;
  delete dir;
}

void TestLucene54DocValuesFormat::doTestTermsEnumRandom(
    int numDocs, int minLength, int maxLength) 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()));
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // set to duel against a codec which has ordinals:
  shared_ptr<PostingsFormat> *const pf =
      TestUtil::getPostingsFormatWithOrds(random());
  shared_ptr<DocValuesFormat> *const dv =
      make_shared<Lucene54DocValuesFormat>();
  conf->setCodec(make_shared<AssertingCodecAnonymousInnerClass>(
      shared_from_this(), pf, dv));
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, conf);

  // index some docs
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<Field> idField =
        make_shared<StringField>(L"id", Integer::toString(i), Field::Store::NO);
    doc->push_back(idField);
    constexpr int length = TestUtil::nextInt(random(), minLength, maxLength);
    int numValues = random()->nextInt(17);
    // create a random deque of strings
    deque<wstring> values = deque<wstring>();
    for (int v = 0; v < numValues; v++) {
      values.push_back(
          TestUtil::randomSimpleString(random(), minLength, length));
    }

    // add in any order to the indexed field
    deque<wstring> unordered = deque<wstring>(values);
    Collections::shuffle(unordered, random());
    for (auto v : values) {
      doc->push_back(newStringField(L"indexed", v, Field::Store::NO));
    }

    // add in any order to the dv field
    deque<wstring> unordered2 = deque<wstring>(values);
    Collections::shuffle(unordered2, random());
    for (auto v : unordered2) {
      doc->push_back(make_shared<SortedSetDocValuesField>(
          L"dv", make_shared<BytesRef>(v)));
    }

    writer->addDocument(doc);
    if (random()->nextInt(31) == 0) {
      writer->commit();
    }
  }

  // delete some docs
  int numDeletions = random()->nextInt(numDocs / 10);
  for (int i = 0; i < numDeletions; i++) {
    int id = random()->nextInt(numDocs);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    writer->deleteDocuments(make_shared<Term>(L"id", Integer::toString(id)));
  }

  // compare per-segment
  shared_ptr<DirectoryReader> ir = writer->getReader();
  for (shared_ptr<LeafReaderContext> context : ir->leaves()) {
    shared_ptr<LeafReader> r = context->reader();
    shared_ptr<Terms> terms = r->terms(L"indexed");
    if (terms != nullptr) {
      shared_ptr<SortedSetDocValues> ssdv = r->getSortedSetDocValues(L"dv");
      assertEquals(terms->size(), ssdv->getValueCount());
      shared_ptr<TermsEnum> expected = terms->begin();
      shared_ptr<TermsEnum> actual =
          r->getSortedSetDocValues(L"dv")->termsEnum();
      assertEquals(terms->size(), expected, actual);

      doTestSortedSetEnumAdvanceIndependently(ssdv);
    }
  }
  ir->close();

  writer->forceMerge(1);

  // now compare again after the merge
  ir = writer->getReader();
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  shared_ptr<Terms> terms = ar->terms(L"indexed");
  if (terms != nullptr) {
    assertEquals(terms->size(),
                 ar->getSortedSetDocValues(L"dv")->getValueCount());
    shared_ptr<TermsEnum> expected = terms->begin();
    shared_ptr<TermsEnum> actual =
        ar->getSortedSetDocValues(L"dv")->termsEnum();
    assertEquals(terms->size(), expected, actual);
  }
  ir->close();

  delete writer;
  delete dir;
}

TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestLucene54DocValuesFormat> outerInstance,
        shared_ptr<PostingsFormat> pf, shared_ptr<DocValuesFormat> dv)
{
  this->outerInstance = outerInstance;
  this->pf = pf;
  this->dv = dv;
}

shared_ptr<PostingsFormat>
TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return pf;
}

shared_ptr<DocValuesFormat>
TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return dv;
}

void TestLucene54DocValuesFormat::assertEquals(
    int64_t numOrds, shared_ptr<TermsEnum> expected,
    shared_ptr<TermsEnum> actual) 
{
  shared_ptr<BytesRef> ref;

  // sequential next() through all terms
  while ((ref = expected->next()) != nullptr) {
    assertEquals(ref, actual->next());
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }
  assertNull(actual->next());

  // sequential seekExact(ord) through all terms
  for (int64_t i = 0; i < numOrds; i++) {
    expected->seekExact(i);
    actual->seekExact(i);
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }

  // sequential seekExact(BytesRef) through all terms
  for (int64_t i = 0; i < numOrds; i++) {
    expected->seekExact(i);
    assertTrue(actual->seekExact(expected->term()));
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }

  // sequential seekCeil(BytesRef) through all terms
  for (int64_t i = 0; i < numOrds; i++) {
    expected->seekExact(i);
    assertEquals(SeekStatus::FOUND, actual->seekCeil(expected->term()));
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }

  // random seekExact(ord)
  for (int64_t i = 0; i < numOrds; i++) {
    int64_t randomOrd = TestUtil::nextLong(random(), 0, numOrds - 1);
    expected->seekExact(randomOrd);
    actual->seekExact(randomOrd);
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }

  // random seekExact(BytesRef)
  for (int64_t i = 0; i < numOrds; i++) {
    int64_t randomOrd = TestUtil::nextLong(random(), 0, numOrds - 1);
    expected->seekExact(randomOrd);
    actual->seekExact(expected->term());
    assertEquals(expected->ord(), actual->ord());
    assertEquals(expected->term(), actual->term());
  }

  // random seekCeil(BytesRef)
  for (int64_t i = 0; i < numOrds; i++) {
    shared_ptr<BytesRef> target =
        make_shared<BytesRef>(TestUtil::randomUnicodeString(random()));
    SeekStatus expectedStatus = expected->seekCeil(target);
    assertEquals(expectedStatus, actual->seekCeil(target));
    if (expectedStatus != SeekStatus::END) {
      assertEquals(expected->ord(), actual->ord());
      assertEquals(expected->term(), actual->term());
    }
  }
}

void TestLucene54DocValuesFormat::testSparseLongValues() 
{
  constexpr int iters = atLeast(5);
  for (int iter = 0; iter < iters; ++iter) {
    constexpr int numDocs = TestUtil::nextInt(random(), 0, 100);
    const std::deque<int> docIds = std::deque<int>(numDocs);
    const std::deque<int64_t> values = std::deque<int64_t>(numDocs);
    constexpr int maxDoc;
    if (numDocs == 0) {
      maxDoc = 1 + random()->nextInt(10);
    } else {
      docIds[0] = random()->nextInt(10);
      for (int i = 1; i < docIds.size(); ++i) {
        docIds[i] = docIds[i - 1] + 1 + random()->nextInt(100);
      }
      maxDoc = docIds[numDocs - 1] + 1 + random()->nextInt(10);
    }
    for (int i = 0; i < values.size(); ++i) {
      values[i] = random()->nextLong();
    }
    constexpr int64_t missingValue = random()->nextLong();
    shared_ptr<LongValues> *const docIdsValues =
        make_shared<LongValuesAnonymousInnerClass>(shared_from_this(), docIds);
    shared_ptr<LongValues> *const valuesValues =
        make_shared<LongValuesAnonymousInnerClass2>(shared_from_this(), values);
    shared_ptr<SparseNumericDocValues> *const sparseValues =
        make_shared<SparseNumericDocValues>(numDocs, docIdsValues,
                                            valuesValues);

    // sequential access
    assertEquals(-1, sparseValues->docID());
    for (int i = 0; i < docIds.size(); ++i) {
      assertEquals(docIds[i], sparseValues->nextDoc());
    }
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, sparseValues->nextDoc());

    // advance
    for (int i = 0; i < 2000; ++i) {
      constexpr int target = TestUtil::nextInt(random(), 0, maxDoc);
      int index = Arrays::binarySearch(docIds, target);
      if (index < 0) {
        index = -1 - index;
      }
      sparseValues->reset();
      if (index > 0) {
        assertEquals(docIds[index - 1], sparseValues->advance(Math::toIntExact(
                                            docIds[index - 1])));
      }
      if (index == docIds.size()) {
        assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                     sparseValues->advance(target));
      } else {
        assertEquals(docIds[index], sparseValues->advance(target));
      }
    }

    // advanceExact
    for (int i = 0; i < 2000; ++i) {
      sparseValues->reset();
      if (random()->nextBoolean() && docIds.size() > 0) {
        sparseValues->advance(
            docIds[TestUtil::nextInt(random(), 0, docIds.size() - 1)]);
      }

      constexpr int target = TestUtil::nextInt(
          random(), max(0, sparseValues->docID()), maxDoc - 1);
      constexpr bool exists = sparseValues->advanceExact(target);

      constexpr int index = Arrays::binarySearch(docIds, target);
      assertEquals(index >= 0, exists);
      assertEquals(target, sparseValues->docID());

      constexpr bool exists2 = sparseValues->advanceExact(target);
      assertEquals(index >= 0, exists2);
      assertEquals(target, sparseValues->docID());

      constexpr int nextIndex = index >= 0 ? index + 1 : -1 - index;
      if (nextIndex >= docIds.size()) {
        assertEquals(DocIdSetIterator::NO_MORE_DOCS, sparseValues->nextDoc());
      } else {
        assertEquals(docIds[nextIndex], sparseValues->nextDoc());
      }
    }

    shared_ptr<SparseNumericDocValuesRandomAccessWrapper> *const raWrapper =
        make_shared<SparseNumericDocValuesRandomAccessWrapper>(sparseValues,
                                                               missingValue);

    // random-access
    for (int i = 0; i < 2000; ++i) {
      constexpr int docId = TestUtil::nextInt(random(), 0, maxDoc - 1);
      constexpr int idx = Arrays::binarySearch(docIds, docId);
      constexpr int64_t value = raWrapper->get(docId);
      if (idx >= 0) {
        assertEquals(values[idx], value);
      } else {
        assertEquals(missingValue, value);
      }
    }

    // sequential access
    for (int docId = 0; docId < maxDoc; docId += random()->nextInt(3)) {
      constexpr int idx = Arrays::binarySearch(docIds, docId);
      constexpr int64_t value = raWrapper->get(docId);
      if (idx >= 0) {
        assertEquals(values[idx], value);
      } else {
        assertEquals(missingValue, value);
      }
    }
  }
}

TestLucene54DocValuesFormat::LongValuesAnonymousInnerClass::
    LongValuesAnonymousInnerClass(
        shared_ptr<TestLucene54DocValuesFormat> outerInstance,
        deque<int> &docIds)
{
  this->outerInstance = outerInstance;
  this->docIds = docIds;
}

int64_t
TestLucene54DocValuesFormat::LongValuesAnonymousInnerClass::get(int64_t index)
{
  return docIds[Math::toIntExact(index)];
}

TestLucene54DocValuesFormat::LongValuesAnonymousInnerClass2::
    LongValuesAnonymousInnerClass2(
        shared_ptr<TestLucene54DocValuesFormat> outerInstance,
        deque<int64_t> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t TestLucene54DocValuesFormat::LongValuesAnonymousInnerClass2::get(
    int64_t index)
{
  return values[Math::toIntExact(index)];
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testSortedSetAroundBlockSize() throws
// java.io.IOException
void TestLucene54DocValuesFormat::testSortedSetAroundBlockSize() throw(
    IOException)
{
  constexpr int frontier =
      1 << Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT;
  for (int maxDoc = frontier - 1; maxDoc <= frontier + 1; ++maxDoc) {
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir, newIndexWriterConfig()->setMergePolicy(newLogMergePolicy()));
    shared_ptr<RAMFile> buffer = make_shared<RAMFile>();
    shared_ptr<RAMOutputStream> out =
        make_shared<RAMOutputStream>(buffer, false);
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<SortedSetDocValuesField> field1 =
        make_shared<SortedSetDocValuesField>(L"sset", make_shared<BytesRef>());
    doc->push_back(field1);
    shared_ptr<SortedSetDocValuesField> field2 =
        make_shared<SortedSetDocValuesField>(L"sset", make_shared<BytesRef>());
    doc->push_back(field2);
    for (int i = 0; i < maxDoc; ++i) {
      shared_ptr<BytesRef> s1 =
          make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2));
      shared_ptr<BytesRef> s2 =
          make_shared<BytesRef>(TestUtil::randomSimpleString(random(), 2));
      field1->setBytesValue(s1);
      field2->setBytesValue(s2);
      w->addDocument(doc);
      shared_ptr<Set<std::shared_ptr<BytesRef>>> set =
          set<std::shared_ptr<BytesRef>>(Arrays::asList(s1, s2));
      out->writeVInt(set->size());
      for (auto ref : set) {
        out->writeVInt(ref->length);
        out->writeBytes(ref->bytes, ref->offset, ref->length);
      }
    }
    delete out;
    w->forceMerge(1);
    shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
    delete w;
    shared_ptr<LeafReader> sr = getOnlyLeafReader(r);
    assertEquals(maxDoc, sr->maxDoc());
    shared_ptr<SortedSetDocValues> values = sr->getSortedSetDocValues(L"sset");
    assertNotNull(values);
    shared_ptr<RAMInputStream> in_ = make_shared<RAMInputStream>(L"", buffer);
    shared_ptr<BytesRefBuilder> b = make_shared<BytesRefBuilder>();
    for (int i = 0; i < maxDoc; ++i) {
      assertEquals(i, values->nextDoc());
      constexpr int numValues = in_->readVInt();

      for (int j = 0; j < numValues; ++j) {
        b->setLength(in_->readVInt());
        b->grow(b->length());
        in_->readBytes(b->bytes(), 0, b->length());
        assertEquals(b->get(), values->lookupOrd(values->nextOrd()));
      }

      assertEquals(SortedSetDocValues::NO_MORE_ORDS, values->nextOrd());
    }
    r->close();
    delete dir;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public void testSortedNumericAroundBlockSize() throws
// java.io.IOException
void TestLucene54DocValuesFormat::testSortedNumericAroundBlockSize() throw(
    IOException)
{
  constexpr int frontier =
      1 << Lucene54DocValuesFormat::DIRECT_MONOTONIC_BLOCK_SHIFT;
  for (int maxDoc = frontier - 1; maxDoc <= frontier + 1; ++maxDoc) {
    shared_ptr<Directory> *const dir = newDirectory();
    shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
        dir, newIndexWriterConfig()->setMergePolicy(newLogMergePolicy()));
    shared_ptr<RAMFile> buffer = make_shared<RAMFile>();
    shared_ptr<RAMOutputStream> out =
        make_shared<RAMOutputStream>(buffer, false);
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<SortedNumericDocValuesField> field1 =
        make_shared<SortedNumericDocValuesField>(L"snum", 0LL);
    doc->push_back(field1);
    shared_ptr<SortedNumericDocValuesField> field2 =
        make_shared<SortedNumericDocValuesField>(L"snum", 0LL);
    doc->push_back(field2);
    for (int i = 0; i < maxDoc; ++i) {
      int64_t s1 = random()->nextInt(100);
      int64_t s2 = random()->nextInt(100);
      field1->setLongValue(s1);
      field2->setLongValue(s2);
      w->addDocument(doc);
      out->writeVLong(min(s1, s2));
      out->writeVLong(max(s1, s2));
    }
    delete out;
    w->forceMerge(1);
    shared_ptr<DirectoryReader> r = DirectoryReader::open(w);
    delete w;
    shared_ptr<LeafReader> sr = getOnlyLeafReader(r);
    assertEquals(maxDoc, sr->maxDoc());
    shared_ptr<SortedNumericDocValues> values =
        sr->getSortedNumericDocValues(L"snum");
    assertNotNull(values);
    shared_ptr<RAMInputStream> in_ = make_shared<RAMInputStream>(L"", buffer);
    for (int i = 0; i < maxDoc; ++i) {
      assertEquals(i, values->nextDoc());
      assertEquals(2, values->docValueCount());
      assertEquals(in_->readVLong(), values->nextValue());
      assertEquals(in_->readVLong(), values->nextValue());
    }
    r->close();
    delete dir;
  }
}

void TestLucene54DocValuesFormat::testSortedNumericAdvanceExact() throw(
    runtime_error)
{
  // test SORTED_WITH_ADDRESSES
  doTestSortedNumericAdvanceExact(TestUtil::nextInt(random(), 100, 200), 17,
                                  nullptr);
  // test SORTED_SET_TABLE
  std::deque<int64_t> values(255);
  for (int i = 0; i < values.size(); i++) {
    values[i] = random()->nextLong();
  }
  doTestSortedNumericAdvanceExact(TestUtil::nextInt(random(), 50, 100), 4,
                                  values);
}

void TestLucene54DocValuesFormat::doTestSortedNumericAdvanceExact(
    int numDocs, int maxValuesPerDoc,
    std::deque<int64_t> &possibleValues) 
{
  shared_ptr<Directory> dir = newFSDirectory(createTempDir());
  shared_ptr<IndexWriterConfig> conf =
      newIndexWriterConfig()->setMergePolicy(newLogMergePolicy());
  conf->setMergeScheduler(make_shared<SerialMergeScheduler>());
  // set to duel against a codec which has ordinals:
  shared_ptr<PostingsFormat> *const pf =
      TestUtil::getPostingsFormatWithOrds(random());
  shared_ptr<DocValuesFormat> *const dv =
      make_shared<Lucene54DocValuesFormat>();
  conf->setCodec(make_shared<AssertingCodecAnonymousInnerClass2>(
      shared_from_this(), pf, dv));
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(dir, conf);

  deque<std::deque<int64_t>> expected = deque<std::deque<int64_t>>();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int numValues = random()->nextInt(maxValuesPerDoc);
    // create a random deque of strings
    std::deque<int64_t> values(numValues);
    for (int v = 0; v < numValues; v++) {
      constexpr int64_t value;
      if (possibleValues.empty()) {
        value = random()->nextLong();
      } else {
        value = possibleValues[TestUtil::nextInt(random(), 0,
                                                 possibleValues.size() - 1)];
      }
      values[v] = value;
      doc->push_back(
          make_shared<SortedNumericDocValuesField>(L"numeric", value));
    }
    Arrays::sort(values);
    expected.push_back(values);
    writer->addDocument(doc);
  }
  writer->commit();
  writer->forceMerge(1);
  shared_ptr<DirectoryReader> ir = DirectoryReader::open(writer);
  delete writer;
  shared_ptr<LeafReader> ar = getOnlyLeafReader(ir);
  for (int i = 0; i < numDocs; i++) {
    int doc = TestUtil::nextInt(random(), 0, numDocs - 1);
    shared_ptr<SortedNumericDocValues> values =
        ar->getSortedNumericDocValues(L"numeric");
    std::deque<int64_t> array_ = expected[doc];
    if (array_.empty()) {
      assertFalse(values->advanceExact(doc));
    } else {
      assertTrue(values->advanceExact(doc));
      assertEquals(array_.size(), values->docValueCount());
      for (int j = 0; j < values->docValueCount(); j++) {
        assertEquals(array_[j], values->nextValue());
      }
    }
  }
  ir->close();
  delete dir;
}

TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(
        shared_ptr<TestLucene54DocValuesFormat> outerInstance,
        shared_ptr<PostingsFormat> pf, shared_ptr<DocValuesFormat> dv)
{
  this->outerInstance = outerInstance;
  this->pf = pf;
  this->dv = dv;
}

shared_ptr<PostingsFormat>
TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass2::
    getPostingsFormatForField(const wstring &field)
{
  return pf;
}

shared_ptr<DocValuesFormat>
TestLucene54DocValuesFormat::AssertingCodecAnonymousInnerClass2::
    getDocValuesFormatForField(const wstring &field)
{
  return dv;
}
} // namespace org::apache::lucene::codecs::lucene54