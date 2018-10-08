using namespace std;

#include "DocumentValueSourceDictionaryTest.h"

namespace org::apache::lucene::search::suggest
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using LongValues = org::apache::lucene::search::LongValues;
using LongValuesSource = org::apache::lucene::search::LongValuesSource;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
const wstring DocumentValueSourceDictionaryTest::FIELD_NAME = L"f1";
const wstring DocumentValueSourceDictionaryTest::WEIGHT_FIELD_NAME_1 = L"w1";
const wstring DocumentValueSourceDictionaryTest::WEIGHT_FIELD_NAME_2 = L"w2";
const wstring DocumentValueSourceDictionaryTest::WEIGHT_FIELD_NAME_3 = L"w3";
const wstring DocumentValueSourceDictionaryTest::PAYLOAD_FIELD_NAME = L"p1";
const wstring DocumentValueSourceDictionaryTest::CONTEXTS_FIELD_NAME = L"c1";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValueSourceEmptyReader() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testValueSourceEmptyReader() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  // Make sure the index is created?
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, LongValuesSource::constant(10), PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();

  assertNull(inputIterator->next());
  assertEquals(inputIterator->weight(), 0);
  assertNull(inputIterator->payload());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongValuesSourceEmptyReader() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testLongValuesSourceEmptyReader() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  // Make sure the index is created?
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, LongValuesSource::constant(10), PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();

  assertNull(inputIterator->next());
  assertEquals(inputIterator->weight(), 0);
  assertNull(inputIterator->payload());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValueSourceBasic() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testValueSourceBasic() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> s =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, s,
                                                 PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

shared_ptr<LongValuesSource>
DocumentValueSourceDictionaryTest::sum(deque<wstring> &fields)
{
  std::deque<std::shared_ptr<LongValuesSource>> sources(fields->length);
  for (int i = 0; i < fields->length; i++) {
    sources[i] = LongValuesSource::fromLongField(fields[i]);
  }
  return make_shared<LongValuesSourceAnonymousInnerClass>(sources);
}

DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    LongValuesSourceAnonymousInnerClass(
        deque<std::shared_ptr<LongValuesSource>> &sources)
{
  this->sources = sources;
}

shared_ptr<LongValues>
DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    getValues(shared_ptr<LeafReaderContext> ctx,
              shared_ptr<DoubleValues> scores) 
{
  std::deque<std::shared_ptr<LongValues>> values(fields->length);
  for (int i = 0; i < sources.size(); i++) {
    values[i] = sources[i]->getValues(ctx, scores);
  }
  return make_shared<LongValuesAnonymousInnerClass>(shared_from_this(), values);
}

DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    LongValuesAnonymousInnerClass::LongValuesAnonymousInnerClass(
        shared_ptr<LongValuesSourceAnonymousInnerClass> outerInstance,
        deque<std::shared_ptr<LongValues>> &values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

int64_t
DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    LongValuesAnonymousInnerClass::longValue() 
{
  int64_t v = 0;
  for (auto value : values) {
    v += value->longValue();
  }
  return v;
}

bool DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    LongValuesAnonymousInnerClass::advanceExact(int doc) 
{
  bool v = true;
  for (auto value : values) {
    v &= value->advanceExact(doc);
  }
  return v;
}

bool DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

bool DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    needsScores()
{
  return false;
}

int DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    hashCode()
{
  return 0;
}

bool DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    equals(any obj)
{
  return false;
}

wstring DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::
    toString()
{
  return L"";
}

shared_ptr<LongValuesSource>
DocumentValueSourceDictionaryTest::LongValuesSourceAnonymousInnerClass::rewrite(
    shared_ptr<IndexSearcher> searcher) 
{
  return shared_from_this();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongValuesSourceBasic() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testLongValuesSourceBasic() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> sumValueSource =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, sumValueSource,
                                                 PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValueSourceWithContext() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testValueSourceWithContext() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> s =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, s, PAYLOAD_FIELD_NAME, CONTEXTS_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
    shared_ptr<Set<std::shared_ptr<BytesRef>>> originalCtxs =
        unordered_set<std::shared_ptr<BytesRef>>();
    for (auto ctxf : doc->getFields(CONTEXTS_FIELD_NAME)) {
      originalCtxs->add(ctxf->binaryValue());
    }
    assertEquals(originalCtxs, inputIterator->contexts());
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongValuesSourceWithContext() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testLongValuesSourceWithContext() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> sumValues =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, sumValues, PAYLOAD_FIELD_NAME, CONTEXTS_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
    shared_ptr<Set<std::shared_ptr<BytesRef>>> originalCtxs =
        unordered_set<std::shared_ptr<BytesRef>>();
    for (auto ctxf : doc->getFields(CONTEXTS_FIELD_NAME)) {
      originalCtxs->add(ctxf->binaryValue());
    }
    assertEquals(originalCtxs, inputIterator->contexts());
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValueSourceWithoutPayload() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testValueSourceWithoutPayload() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> s =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, s);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    assertNull(inputIterator->payload());
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongValuesSourceWithoutPayload() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::
    testLongValuesSourceWithoutPayload() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<LongValuesSource> sumValues =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2, WEIGHT_FIELD_NAME_3});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, sumValues);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    int64_t w3 =
        doc->getField(WEIGHT_FIELD_NAME_3)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), (w1 + w2 + w3));
    assertNull(inputIterator->payload());
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValueSourceWithDeletions() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testValueSourceWithDeletions() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  shared_ptr<Random> rand = random();
  deque<wstring> termsToDel = deque<wstring>();
  for (auto doc : docs) {
    if (rand->nextBoolean() && termsToDel.size() < docs.size() - 1) {
      termsToDel.push_back(doc->second->get(FIELD_NAME));
    }
    writer->addDocument(doc->second);
  }
  writer->commit();

  std::deque<std::shared_ptr<Term>> delTerms(termsToDel.size());
  for (int i = 0; i < termsToDel.size(); i++) {
    delTerms[i] = make_shared<Term>(FIELD_NAME, termsToDel[i]);
  }

  for (auto delTerm : delTerms) {
    writer->deleteDocuments(delTerm);
  }
  writer->commit();
  delete writer;

  for (auto termToDel : termsToDel) {
    assertTrue(nullptr != docs.erase(termToDel));
  }

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  assertTrue(L"NumDocs should be > 0 but was " + to_wstring(ir->numDocs()),
             ir->numDocs() > 0);
  assertEquals(ir->numDocs(), docs.size());
  shared_ptr<LongValuesSource> s =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, s,
                                                 PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), w2 + w1);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLongValuesSourceWithDeletions() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::
    testLongValuesSourceWithDeletions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  shared_ptr<Random> rand = random();
  deque<wstring> termsToDel = deque<wstring>();
  for (auto doc : docs) {
    if (rand->nextBoolean() && termsToDel.size() < docs.size() - 1) {
      termsToDel.push_back(doc->second->get(FIELD_NAME));
    }
    writer->addDocument(doc->second);
  }
  writer->commit();

  std::deque<std::shared_ptr<Term>> delTerms(termsToDel.size());
  for (int i = 0; i < termsToDel.size(); i++) {
    delTerms[i] = make_shared<Term>(FIELD_NAME, termsToDel[i]);
  }

  for (auto delTerm : delTerms) {
    writer->deleteDocuments(delTerm);
  }
  writer->commit();
  delete writer;

  for (auto termToDel : termsToDel) {
    assertTrue(nullptr != docs.erase(termToDel));
  }

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  assertTrue(L"NumDocs should be > 0 but was " + to_wstring(ir->numDocs()),
             ir->numDocs() > 0);
  assertEquals(ir->numDocs(), docs.size());
  shared_ptr<LongValuesSource> sumValues =
      sum({WEIGHT_FIELD_NAME_1, WEIGHT_FIELD_NAME_2});
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(ir, FIELD_NAME, sumValues,
                                                 PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    int64_t w1 =
        doc->getField(WEIGHT_FIELD_NAME_1)->numericValue()->longValue();
    int64_t w2 =
        doc->getField(WEIGHT_FIELD_NAME_2)->numericValue()->longValue();
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), w2 + w1);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithValueSource() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testWithValueSource() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, LongValuesSource::constant(10), PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), 10);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithLongValuesSource() throws
// java.io.IOException
void DocumentValueSourceDictionaryTest::testWithLongValuesSource() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      generateIndexDocuments(atLeast(100));
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentValueSourceDictionary>(
          ir, FIELD_NAME, LongValuesSource::constant(10), PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    assertEquals(inputIterator->weight(), 10);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }
  assertTrue(docs.empty());
  IOUtils::close({ir, analyzer, dir});
}

unordered_map<wstring, std::shared_ptr<Document>>
DocumentValueSourceDictionaryTest::generateIndexDocuments(int ndocs)
{
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      unordered_map<wstring, std::shared_ptr<Document>>();
  for (int i = 0; i < ndocs; i++) {
    shared_ptr<Field> field = make_shared<TextField>(
        FIELD_NAME, L"field_" + to_wstring(i), Field::Store::YES);
    shared_ptr<Field> weight1 =
        make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME_1, 10 + i);
    shared_ptr<Field> weight2 =
        make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME_2, 20 + i);
    shared_ptr<Field> weight3 =
        make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME_3, 30 + i);
    shared_ptr<Field> contexts = make_shared<StoredField>(
        CONTEXTS_FIELD_NAME,
        make_shared<BytesRef>(L"ctx_" + to_wstring(i) + L"_0"));
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(field);
    // even if payload is not required usually have it
    if (usually()) {
      shared_ptr<Field> payload = make_shared<StoredField>(
          PAYLOAD_FIELD_NAME,
          make_shared<BytesRef>(L"payload_" + to_wstring(i)));
      doc->push_back(payload);
    }
    doc->push_back(weight1);
    doc->push_back(weight2);
    doc->push_back(weight3);
    doc->push_back(contexts);
    for (int j = 1; j < atLeast(3); j++) {
      contexts->setBytesValue(make_shared<BytesRef>(L"ctx_" + to_wstring(i) +
                                                    L"_" + to_wstring(j)));
      doc->push_back(contexts);
    }
    docs.emplace(field->stringValue(), doc);
  }
  return docs;
}
} // namespace org::apache::lucene::search::suggest