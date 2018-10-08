using namespace std;

#include "TestICUCollationDocValuesField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/StringField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/MultiDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/MatchAllDocsQuery.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../java/org/apache/lucene/collation/ICUCollationDocValuesField.h"

namespace org::apache::lucene::collation
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiDocValues = org::apache::lucene::index::MultiDocValues;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using MatchAllDocsQuery = org::apache::lucene::search::MatchAllDocsQuery;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::ibm::icu::text::Collator;
using com::ibm::icu::util::ULocale;

void TestICUCollationDocValuesField::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newField(L"field", L"", StringField::TYPE_STORED);
  shared_ptr<ICUCollationDocValuesField> collationField =
      make_shared<ICUCollationDocValuesField>(
          L"collated", Collator::getInstance(ULocale::ENGLISH));
  doc->push_back(field);
  doc->push_back(collationField);

  field->setStringValue(L"ABC");
  collationField->setStringValue(L"ABC");
  iw->addDocument(doc);

  field->setStringValue(L"abc");
  collationField->setStringValue(L"abc");
  iw->addDocument(doc);

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;

  shared_ptr<IndexSearcher> is = newSearcher(ir);

  shared_ptr<SortField> sortField =
      make_shared<SortField>(L"collated", SortField::Type::STRING);

  shared_ptr<TopDocs> td = is->search(make_shared<MatchAllDocsQuery>(), 5,
                                      make_shared<Sort>(sortField));
  TestUtil::assertEquals(L"abc", ir->document(td->scoreDocs[0]->doc)[L"field"]);
  TestUtil::assertEquals(L"ABC", ir->document(td->scoreDocs[1]->doc)[L"field"]);
  delete ir;
  delete dir;
}

void TestICUCollationDocValuesField::testRanges() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field = newField(L"field", L"", StringField::TYPE_STORED);
  shared_ptr<Collator> collator =
      Collator::getInstance(); // uses -Dtests.locale
  if (random()->nextBoolean()) {
    collator->setStrength(Collator::PRIMARY);
  }
  shared_ptr<ICUCollationDocValuesField> collationField =
      make_shared<ICUCollationDocValuesField>(L"collated", collator);
  doc->push_back(field);
  doc->push_back(collationField);

  int numDocs = atLeast(500);
  for (int i = 0; i < numDocs; i++) {
    wstring value = TestUtil::randomSimpleString(random());
    field->setStringValue(value);
    collationField->setStringValue(value);
    iw->addDocument(doc);
  }

  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  int numChecks = atLeast(100);
  for (int i = 0; i < numChecks; i++) {
    wstring start = TestUtil::randomSimpleString(random());
    wstring end = TestUtil::randomSimpleString(random());
    shared_ptr<BytesRef> lowerVal =
        make_shared<BytesRef>(collator->getCollationKey(start).toByteArray());
    shared_ptr<BytesRef> upperVal =
        make_shared<BytesRef>(collator->getCollationKey(end).toByteArray());
    doTestRanges(is, start, end, lowerVal, upperVal, collator);
  }

  delete ir;
  delete dir;
}

void TestICUCollationDocValuesField::doTestRanges(
    shared_ptr<IndexSearcher> is, const wstring &startPoint,
    const wstring &endPoint, shared_ptr<BytesRef> startBR,
    shared_ptr<BytesRef> endBR,
    shared_ptr<Collator> collator) 
{
  shared_ptr<SortedDocValues> dvs =
      MultiDocValues::getSortedValues(is->getIndexReader(), L"collated");
  for (int docID = 0; docID < is->getIndexReader()->maxDoc(); docID++) {
    shared_ptr<Document> doc = is->doc(docID);
    wstring s = doc->getField(L"field")->stringValue();
    bool collatorAccepts = collator->compare(s, startPoint) >= 0 &&
                           collator->compare(s, endPoint) <= 0;
    TestUtil::assertEquals(docID, dvs->nextDoc());
    shared_ptr<BytesRef> br = dvs->binaryValue();
    bool luceneAccepts =
        br->compareTo(startBR) >= 0 && br->compareTo(endBR) <= 0;
    TestUtil::assertEquals(collatorAccepts, luceneAccepts);
  }
}
} // namespace org::apache::lucene::collation