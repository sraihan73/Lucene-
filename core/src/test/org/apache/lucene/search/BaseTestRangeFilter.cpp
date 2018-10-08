using namespace std;

#include "BaseTestRangeFilter.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::BeforeClass;
using org::junit::Test;

BaseTestRangeFilter::TestIndex::TestIndex(shared_ptr<Random> random, int minR,
                                          int maxR,
                                          bool allowNegativeRandomInts)
{
  this->minR = minR;
  this->maxR = maxR;
  this->allowNegativeRandomInts = allowNegativeRandomInts;
  index = LuceneTestCase::newDirectory(random);
}

shared_ptr<org::apache::lucene::index::IndexReader>
    BaseTestRangeFilter::signedIndexReader;
shared_ptr<org::apache::lucene::index::IndexReader>
    BaseTestRangeFilter::unsignedIndexReader;
shared_ptr<TestIndex> BaseTestRangeFilter::signedIndexDir;
shared_ptr<TestIndex> BaseTestRangeFilter::unsignedIndexDir;
int BaseTestRangeFilter::minId = 0;
int BaseTestRangeFilter::maxId = 0;

wstring BaseTestRangeFilter::pad(int n)
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>(40);
  wstring p = L"0";
  if (n < 0) {
    p = L"-";
    n = numeric_limits<int>::max() + n + 1;
  }
  b->append(p);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = Integer::toString(n);
  for (int i = s.length(); i <= intLength; i++) {
    b->append(L"0");
  }
  b->append(s);

  return b->toString();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void
// beforeClassBaseTestRangeFilter() throws Exception
void BaseTestRangeFilter::beforeClassBaseTestRangeFilter() 
{
  maxId = atLeast(500);
  signedIndexDir = make_shared<TestIndex>(random(), numeric_limits<int>::max(),
                                          numeric_limits<int>::min(), true);
  unsignedIndexDir =
      make_shared<TestIndex>(random(), numeric_limits<int>::max(), 0, false);
  signedIndexReader = build(random(), signedIndexDir);
  unsignedIndexReader = build(random(), unsignedIndexDir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassBaseTestRangeFilter()
// throws Exception
void BaseTestRangeFilter::afterClassBaseTestRangeFilter() 
{
  delete signedIndexReader;
  delete unsignedIndexReader;
  delete signedIndexDir->index;
  delete unsignedIndexDir->index;
  signedIndexReader.reset();
  unsignedIndexReader.reset();
  signedIndexDir.reset();
  unsignedIndexDir.reset();
}

shared_ptr<IndexReader>
BaseTestRangeFilter::build(shared_ptr<Random> random,
                           shared_ptr<TestIndex> index) 
{
  /* build an index */

  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> idField =
      newStringField(random, L"id", L"", Field::Store::YES);
  shared_ptr<Field> idDVField =
      make_shared<SortedDocValuesField>(L"id", make_shared<BytesRef>());
  shared_ptr<Field> intIdField = make_shared<IntPoint>(L"id_int", 0);
  shared_ptr<Field> intDVField =
      make_shared<NumericDocValuesField>(L"id_int", 0);
  shared_ptr<Field> floatIdField = make_shared<FloatPoint>(L"id_float", 0);
  shared_ptr<Field> floatDVField =
      make_shared<NumericDocValuesField>(L"id_float", 0);
  shared_ptr<Field> longIdField = make_shared<LongPoint>(L"id_long", 0);
  shared_ptr<Field> longDVField =
      make_shared<NumericDocValuesField>(L"id_long", 0);
  shared_ptr<Field> doubleIdField = make_shared<DoublePoint>(L"id_double", 0);
  shared_ptr<Field> doubleDVField =
      make_shared<NumericDocValuesField>(L"id_double", 0);
  shared_ptr<Field> randField =
      newStringField(random, L"rand", L"", Field::Store::YES);
  shared_ptr<Field> randDVField =
      make_shared<SortedDocValuesField>(L"rand", make_shared<BytesRef>());
  shared_ptr<Field> bodyField =
      newStringField(random, L"body", L"", Field::Store::NO);
  shared_ptr<Field> bodyDVField =
      make_shared<SortedDocValuesField>(L"body", make_shared<BytesRef>());
  doc->push_back(idField);
  doc->push_back(idDVField);
  doc->push_back(intIdField);
  doc->push_back(intDVField);
  doc->push_back(floatIdField);
  doc->push_back(floatDVField);
  doc->push_back(longIdField);
  doc->push_back(longDVField);
  doc->push_back(doubleIdField);
  doc->push_back(doubleDVField);
  doc->push_back(randField);
  doc->push_back(randDVField);
  doc->push_back(bodyField);
  doc->push_back(bodyDVField);

  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random, index->index,
      newIndexWriterConfig(random, make_shared<MockAnalyzer>(random))
          ->setOpenMode(OpenMode::CREATE)
          ->setMaxBufferedDocs(TestUtil::nextInt(random, 50, 1000))
          ->setMergePolicy(newLogMergePolicy()));
  TestUtil::reduceOpenFiles(writer->w);

  while (true) {

    int minCount = 0;
    int maxCount = 0;

    for (int d = minId; d <= maxId; d++) {
      idField->setStringValue(pad(d));
      idDVField->setBytesValue(make_shared<BytesRef>(pad(d)));
      intIdField->setIntValue(d);
      intDVField->setLongValue(d);
      floatIdField->setFloatValue(d);
      floatDVField->setLongValue(Float::floatToRawIntBits(d));
      longIdField->setLongValue(d);
      longDVField->setLongValue(d);
      doubleIdField->setDoubleValue(d);
      doubleDVField->setLongValue(Double::doubleToRawLongBits(d));
      int r = index->allowNegativeRandomInts
                  ? random->nextInt()
                  : random->nextInt(numeric_limits<int>::max());
      if (index->maxR < r) {
        index->maxR = r;
        maxCount = 1;
      } else if (index->maxR == r) {
        maxCount++;
      }

      if (r < index->minR) {
        index->minR = r;
        minCount = 1;
      } else if (r == index->minR) {
        minCount++;
      }
      randField->setStringValue(pad(r));
      randDVField->setBytesValue(make_shared<BytesRef>(pad(r)));
      bodyField->setStringValue(L"body");
      bodyDVField->setBytesValue(make_shared<BytesRef>(L"body"));
      writer->addDocument(doc);
    }

    if (minCount == 1 && maxCount == 1) {
      // our subclasses rely on only 1 doc having the min or
      // max, so, we loop until we satisfy that.  it should be
      // exceedingly rare (Yonik calculates 1 in ~429,000)
      // times) that this loop requires more than one try:
      shared_ptr<IndexReader> ir = writer->getReader();
      delete writer;
      return ir;
    }

    // try again
    writer->deleteAll();
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPad()
void BaseTestRangeFilter::testPad()
{

  std::deque<int> tests = {-9999999, -99560, -100, -3,   -1,       0,
                            3,        9,      10,   1000, 999999999};
  for (int i = 0; i < tests.size() - 1; i++) {
    int a = tests[i];
    int b = tests[i + 1];
    wstring aa = pad(a);
    wstring bb = pad(b);
    wstring label =
        to_wstring(a) + L":" + aa + L" vs " + to_wstring(b) + L":" + bb;
    assertEquals(L"length of " + label, aa.length(), bb.length());
    assertTrue(L"compare less than " + label, aa.compare(bb) < 0);
  }
}
} // namespace org::apache::lucene::search