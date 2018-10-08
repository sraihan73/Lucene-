using namespace std;

#include "DataSplitterTest.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/SortedDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/classification/utils/DatasetSplitter.h"

namespace org::apache::lucene::classification::utils
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using BaseDirectoryWrapper = org::apache::lucene::store::BaseDirectoryWrapper;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;
const wstring DataSplitterTest::textFieldName = L"text";
const wstring DataSplitterTest::classFieldName = L"class";
const wstring DataSplitterTest::idFieldName = L"id";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void DataSplitterTest::setUp() 
{
  LuceneTestCase::setUp();
  dir = newDirectory();
  indexWriter = make_shared<RandomIndexWriter>(random(), dir);

  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(true);
  ft->setStoreTermVectorOffsets(true);
  ft->setStoreTermVectorPositions(true);

  shared_ptr<Document> doc;
  shared_ptr<Random> rnd = random();
  for (int i = 0; i < 1000; i++) {
    doc = make_shared<Document>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(
        make_shared<Field>(idFieldName, L"id" + Integer::toString(i), ft));
    doc->push_back(make_shared<Field>(
        textFieldName, TestUtil::randomUnicodeString(rnd, 1024), ft));
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring className = Integer::toString(rnd->nextInt(10));
    doc->push_back(make_shared<Field>(classFieldName, className, ft));
    doc->push_back(make_shared<SortedDocValuesField>(
        classFieldName, make_shared<BytesRef>(className)));
    indexWriter->addDocument(doc);
  }

  indexWriter->commit();
  indexWriter->forceMerge(1);

  originalIndex = getOnlyLeafReader(indexWriter->getReader());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @After public void tearDown() throws Exception
void DataSplitterTest::tearDown() 
{
  delete originalIndex;
  delete indexWriter;
  delete dir;
  LuceneTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSplitOnAllFields() throws Exception
void DataSplitterTest::testSplitOnAllFields() 
{
  assertSplit(originalIndex, 0.1, 0.1);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSplitOnSomeFields() throws Exception
void DataSplitterTest::testSplitOnSomeFields() 
{
  assertSplit(originalIndex, 0.2, 0.35, {idFieldName, textFieldName});
}

void DataSplitterTest::assertSplit(
    shared_ptr<LeafReader> originalIndex, double testRatio,
    double crossValidationRatio,
    deque<wstring> &fieldNames) 
{

  shared_ptr<BaseDirectoryWrapper> trainingIndex = newDirectory();
  shared_ptr<BaseDirectoryWrapper> testIndex = newDirectory();
  shared_ptr<BaseDirectoryWrapper> crossValidationIndex = newDirectory();

  try {
    shared_ptr<DatasetSplitter> datasetSplitter =
        make_shared<DatasetSplitter>(testRatio, crossValidationRatio);
    datasetSplitter->split(originalIndex, trainingIndex, testIndex,
                           crossValidationIndex,
                           make_shared<MockAnalyzer>(random()), true,
                           classFieldName, {fieldNames});

    assertNotNull(trainingIndex);
    assertNotNull(testIndex);
    assertNotNull(crossValidationIndex);

    shared_ptr<DirectoryReader> trainingReader =
        DirectoryReader::open(trainingIndex);
    assertEquals(static_cast<int>(originalIndex->maxDoc() *
                                  (1 - testRatio - crossValidationRatio)),
                 trainingReader->maxDoc(), 20);
    shared_ptr<DirectoryReader> testReader = DirectoryReader::open(testIndex);
    assertEquals(static_cast<int>(originalIndex->maxDoc() * testRatio),
                 testReader->maxDoc(), 20);
    shared_ptr<DirectoryReader> cvReader =
        DirectoryReader::open(crossValidationIndex);
    assertEquals(
        static_cast<int>(originalIndex->maxDoc() * crossValidationRatio),
        cvReader->maxDoc(), 20);

    trainingReader->close();
    testReader->close();
    cvReader->close();
    closeQuietly(trainingReader);
    closeQuietly(testReader);
    closeQuietly(cvReader);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (trainingIndex != nullptr) {
      delete trainingIndex;
    }
    if (testIndex != nullptr) {
      delete testIndex;
    }
    if (crossValidationIndex != nullptr) {
      delete crossValidationIndex;
    }
  }
}

void DataSplitterTest::closeQuietly(shared_ptr<IndexReader> reader) throw(
    IOException)
{
  try {
    if (reader != nullptr) {
      delete reader;
    }
  } catch (const runtime_error &e) {
    // do nothing
  }
}
} // namespace org::apache::lucene::classification::utils