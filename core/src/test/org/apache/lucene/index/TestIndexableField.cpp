using namespace std;

#include "TestIndexableField.h"

namespace org::apache::lucene::index
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using StoredField = org::apache::lucene::document::StoredField;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    IndexableFieldTypeAnonymousInnerClass()
{
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    stored()
{
  return (outerInstance->counter & 1) == 0 ||
         (outerInstance->counter % 10) == 3;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    tokenized()
{
  return true;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    storeTermVectors()
{
  return indexOptions() != IndexOptions::NONE &&
         outerInstance->counter % 2 == 1 && outerInstance->counter % 10 != 9;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    storeTermVectorOffsets()
{
  return storeTermVectors() && outerInstance->counter % 10 != 9;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    storeTermVectorPositions()
{
  return storeTermVectors() && outerInstance->counter % 10 != 9;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    storeTermVectorPayloads()
{
  return storeTermVectors() && outerInstance->counter % 10 != 9;
}

bool TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    omitNorms()
{
  return false;
}

IndexOptions TestIndexableField::MyField::
    IndexableFieldTypeAnonymousInnerClass::indexOptions()
{
  return outerInstance->counter % 10 == 3
             ? IndexOptions::NONE
             : IndexOptions::DOCS_AND_FREQS_AND_POSITIONS;
}

DocValuesType TestIndexableField::MyField::
    IndexableFieldTypeAnonymousInnerClass::docValuesType()
{
  return DocValuesType::NONE;
}

int TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    pointDimensionCount()
{
  return 0;
}

int TestIndexableField::MyField::IndexableFieldTypeAnonymousInnerClass::
    pointNumBytes()
{
  return 0;
}

TestIndexableField::MyField::MyField(int counter) : counter(counter) {}

wstring TestIndexableField::MyField::name()
{
  return L"f" + to_wstring(counter);
}

shared_ptr<BytesRef> TestIndexableField::MyField::binaryValue()
{
  if ((counter % 10) == 3) {
    const std::deque<char> bytes = std::deque<char>(10);
    for (int idx = 0; idx < bytes.size(); idx++) {
      bytes[idx] = static_cast<char>(counter + idx);
    }
    return make_shared<BytesRef>(bytes, 0, bytes.size());
  } else {
    return nullptr;
  }
}

wstring TestIndexableField::MyField::stringValue()
{
  constexpr int fieldID = counter % 10;
  if (fieldID != 3 && fieldID != 7) {
    return L"text " + to_wstring(counter);
  } else {
    return L"";
  }
}

shared_ptr<Reader> TestIndexableField::MyField::readerValue()
{
  if (counter % 10 == 7) {
    return make_shared<StringReader>(L"text " + to_wstring(counter));
  } else {
    return nullptr;
  }
}

shared_ptr<Number> TestIndexableField::MyField::numericValue()
{
  return nullptr;
}

shared_ptr<IndexableFieldType> TestIndexableField::MyField::fieldType()
{
  return fieldType;
}

shared_ptr<TokenStream>
TestIndexableField::MyField::tokenStream(shared_ptr<Analyzer> analyzer,
                                         shared_ptr<TokenStream> previous)
{
  return readerValue() != nullptr
             ? analyzer->tokenStream(name(), readerValue())
             : analyzer->tokenStream(name(),
                                     make_shared<StringReader>(stringValue()));
}

void TestIndexableField::testArbitraryFields() 
{

  shared_ptr<Directory> *const dir = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), dir);

  constexpr int NUM_DOCS = atLeast(27);
  if (VERBOSE) {
    wcout << L"TEST: " << NUM_DOCS << L" docs" << endl;
  }
  const std::deque<int> fieldsPerDoc = std::deque<int>(NUM_DOCS);
  int baseCount = 0;

  for (int docCount = 0; docCount < NUM_DOCS; docCount++) {
    constexpr int fieldCount = TestUtil::nextInt(random(), 1, 17);
    fieldsPerDoc[docCount] = fieldCount - 1;

    constexpr int finalDocCount = docCount;
    if (VERBOSE) {
      wcout << L"TEST: " << fieldCount << L" fields in doc " << docCount
            << endl;
    }

    constexpr int finalBaseCount = baseCount;
    baseCount += fieldCount - 1;

    deque<std::shared_ptr<IndexableField>> d =
        make_shared<IterableAnonymousInnerClass>(shared_from_this(), fieldCount,
                                                 finalDocCount, finalBaseCount);
    w->addDocument(d);
  }

  shared_ptr<IndexReader> *const r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  int counter = 0;
  for (int id = 0; id < NUM_DOCS; id++) {
    if (VERBOSE) {
      wcout << L"TEST: verify doc id=" << id << L" (" << fieldsPerDoc[id]
            << L" fields) counter=" << counter << endl;
    }

    shared_ptr<TopDocs> *const hits = s->search(
        make_shared<TermQuery>(make_shared<Term>(L"id", L"" + to_wstring(id))),
        1);
    TestUtil::assertEquals(1, hits->totalHits);
    constexpr int docID = hits->scoreDocs[0]->doc;
    shared_ptr<Document> *const doc = s->doc(docID);
    constexpr int endCounter = counter + fieldsPerDoc[id];
    while (counter < endCounter) {
      const wstring name = L"f" + to_wstring(counter);
      constexpr int fieldID = counter % 10;

      constexpr bool stored = (counter & 1) == 0 || fieldID == 3;
      constexpr bool binary = fieldID == 3;
      constexpr bool indexed = fieldID != 3;

      const wstring stringValue;
      if (fieldID != 3 && fieldID != 9) {
        stringValue = L"text " + to_wstring(counter);
      } else {
        stringValue = L"";
      }

      // stored:
      if (stored) {
        shared_ptr<IndexableField> f = doc->getField(name);
        assertNotNull(L"doc " + to_wstring(id) + L" doesn't have field f" +
                          to_wstring(counter),
                      f);
        if (binary) {
          assertNotNull(L"doc " + to_wstring(id) + L" doesn't have field f" +
                            to_wstring(counter),
                        f);
          shared_ptr<BytesRef> *const b = f->binaryValue();
          assertNotNull(b);
          TestUtil::assertEquals(10, b->length);
          for (int idx = 0; idx < 10; idx++) {
            TestUtil::assertEquals(static_cast<char>(idx + counter),
                                   b->bytes[b->offset + idx]);
          }
        } else {
          assert(stringValue != L"");
          TestUtil::assertEquals(stringValue, f->stringValue());
        }
      }

      if (indexed) {
        constexpr bool tv = counter % 2 == 1 && fieldID != 9;
        if (tv) {
          shared_ptr<Terms> *const tfv = r->getTermVectors(docID)->terms(name);
          assertNotNull(tfv);
          shared_ptr<TermsEnum> termsEnum = tfv->begin();
          TestUtil::assertEquals(
              make_shared<BytesRef>(L"" + to_wstring(counter)),
              termsEnum->next());
          TestUtil::assertEquals(1, termsEnum->totalTermFreq());
          shared_ptr<PostingsEnum> dpEnum =
              termsEnum->postings(nullptr, PostingsEnum::ALL);
          assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
          TestUtil::assertEquals(1, dpEnum->freq());
          TestUtil::assertEquals(1, dpEnum->nextPosition());

          TestUtil::assertEquals(make_shared<BytesRef>(L"text"),
                                 termsEnum->next());
          TestUtil::assertEquals(1, termsEnum->totalTermFreq());
          dpEnum = termsEnum->postings(dpEnum, PostingsEnum::ALL);
          assertTrue(dpEnum->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
          TestUtil::assertEquals(1, dpEnum->freq());
          TestUtil::assertEquals(0, dpEnum->nextPosition());

          assertNull(termsEnum->next());

          // TODO: offsets

        } else {
          shared_ptr<Fields> vectors = r->getTermVectors(docID);
          assertTrue(vectors->empty() || vectors->terms(name) == nullptr);
        }

        shared_ptr<BooleanQuery::Builder> bq =
            make_shared<BooleanQuery::Builder>();
        bq->add(make_shared<TermQuery>(
                    make_shared<Term>(L"id", L"" + to_wstring(id))),
                BooleanClause::Occur::MUST);
        bq->add(make_shared<TermQuery>(make_shared<Term>(name, L"text")),
                BooleanClause::Occur::MUST);
        shared_ptr<TopDocs> *const hits2 = s->search(bq->build(), 1);
        TestUtil::assertEquals(1, hits2->totalHits);
        TestUtil::assertEquals(docID, hits2->scoreDocs[0]->doc);

        bq = make_shared<BooleanQuery::Builder>();
        bq->add(make_shared<TermQuery>(
                    make_shared<Term>(L"id", L"" + to_wstring(id))),
                BooleanClause::Occur::MUST);
        bq->add(make_shared<TermQuery>(
                    make_shared<Term>(name, L"" + to_wstring(counter))),
                BooleanClause::Occur::MUST);
        shared_ptr<TopDocs> *const hits3 = s->search(bq->build(), 1);
        TestUtil::assertEquals(1, hits3->totalHits);
        TestUtil::assertEquals(docID, hits3->scoreDocs[0]->doc);
      }

      counter++;
    }
  }

  delete r;
  delete dir;
}

TestIndexableField::IterableAnonymousInnerClass::IterableAnonymousInnerClass(
    shared_ptr<TestIndexableField> outerInstance, int fieldCount,
    int finalDocCount, int finalBaseCount)
{
  this->outerInstance = outerInstance;
  this->fieldCount = fieldCount;
  this->finalDocCount = finalDocCount;
  this->finalBaseCount = finalBaseCount;
}

shared_ptr<Iterator<std::shared_ptr<IndexableField>>>
TestIndexableField::IterableAnonymousInnerClass::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

TestIndexableField::IterableAnonymousInnerClass::IteratorAnonymousInnerClass::
    IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool TestIndexableField::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  return fieldUpto < outerInstance->fieldCount;
}

shared_ptr<IndexableField> TestIndexableField::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  assert(fieldUpto < outerInstance->fieldCount);
  if (fieldUpto == 0) {
    fieldUpto = 1;
    return LuceneTestCase::newStringField(
        L"id", L"" + outerInstance->finalDocCount, Field::Store::YES);
  } else {
    return make_shared<MyField>(outerInstance->finalBaseCount +
                                (fieldUpto++ - 1));
  }
}

void TestIndexableField::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::remove()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef> TestIndexableField::CustomField::binaryValue()
{
  return nullptr;
}

wstring TestIndexableField::CustomField::stringValue() { return L"foobar"; }

shared_ptr<Reader> TestIndexableField::CustomField::readerValue()
{
  return nullptr;
}

shared_ptr<Number> TestIndexableField::CustomField::numericValue()
{
  return nullptr;
}

wstring TestIndexableField::CustomField::name() { return L"field"; }

shared_ptr<TokenStream>
TestIndexableField::CustomField::tokenStream(shared_ptr<Analyzer> a,
                                             shared_ptr<TokenStream> reuse)
{
  return nullptr;
}

shared_ptr<IndexableFieldType> TestIndexableField::CustomField::fieldType()
{
  shared_ptr<FieldType> ft = make_shared<FieldType>(StoredField::TYPE);
  ft->setStoreTermVectors(true);
  ft->freeze();
  return ft;
}

void TestIndexableField::testNotIndexedTermVectors() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  try {
    w->addDocument(Collections::singletonList<std::shared_ptr<IndexableField>>(
        make_shared<CustomField>()));
    fail(L"didn't hit exception");
  } catch (const invalid_argument &iae) {
    // expected
  }
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::index