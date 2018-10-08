using namespace std;

#include "DocumentDictionaryTest.h"

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
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
const wstring DocumentDictionaryTest::FIELD_NAME = L"f1";
const wstring DocumentDictionaryTest::WEIGHT_FIELD_NAME = L"w1";
const wstring DocumentDictionaryTest::PAYLOAD_FIELD_NAME = L"p1";
const wstring DocumentDictionaryTest::CONTEXT_FIELD_NAME = L"c1";

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyReader() throws java.io.IOException
void DocumentDictionaryTest::testEmptyReader() 
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
  shared_ptr<Dictionary> dictionary = make_shared<DocumentDictionary>(
      ir, FIELD_NAME, WEIGHT_FIELD_NAME, PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();

  assertNull(inputIterator->next());
  assertEquals(inputIterator->weight(), 0);
  assertNull(inputIterator->payload());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasic() throws java.io.IOException
void DocumentDictionaryTest::testBasic() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map::Entry<deque<wstring>,
                       unordered_map<wstring, std::shared_ptr<Document>>>
      res = generateIndexDocuments(atLeast(1000), false);
  unordered_map<wstring, std::shared_ptr<Document>> docs = res.getValue();
  deque<wstring> invalidDocTerms = res.getKey();
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary = make_shared<DocumentDictionary>(
      ir, FIELD_NAME, WEIGHT_FIELD_NAME, PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    shared_ptr<IndexableField> weightField = doc->getField(WEIGHT_FIELD_NAME);
    assertEquals(inputIterator->weight(),
                 (weightField != nullptr)
                     ? weightField->numericValue()->longValue()
                     : 0);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
  }

  for (auto invalidTerm : invalidDocTerms) {
    assertNotNull(docs.erase(invalidTerm));
  }
  assertTrue(docs.empty());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithOptionalPayload() throws
// java.io.IOException
void DocumentDictionaryTest::testWithOptionalPayload() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  // Create a document that is missing the payload field
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> field =
      make_shared<TextField>(FIELD_NAME, L"some field", Field::Store::YES);
  doc->push_back(field);
  // do not store the payload or the contexts
  shared_ptr<Field> weight =
      make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME, 100);
  doc->push_back(weight);
  writer->addDocument(doc);
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);

  // Even though the payload field is missing, the dictionary iterator should
  // not skip the document because the payload field is optional.
  shared_ptr<Dictionary> dictionaryOptionalPayload =
      make_shared<DocumentDictionary>(ir, FIELD_NAME, WEIGHT_FIELD_NAME,
                                      PAYLOAD_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator =
      dictionaryOptionalPayload->getEntryIterator();
  shared_ptr<BytesRef> f = inputIterator->next();
  assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
  shared_ptr<IndexableField> weightField = doc->getField(WEIGHT_FIELD_NAME);
  assertEquals(inputIterator->weight(),
               weightField->numericValue()->longValue());
  shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
  assertNull(payloadField);
  assertTrue(inputIterator->payload()->length == 0);
  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithoutPayload() throws
// java.io.IOException
void DocumentDictionaryTest::testWithoutPayload() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map::Entry<deque<wstring>,
                       unordered_map<wstring, std::shared_ptr<Document>>>
      res = generateIndexDocuments(atLeast(1000), false);
  unordered_map<wstring, std::shared_ptr<Document>> docs = res.getValue();
  deque<wstring> invalidDocTerms = res.getKey();
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentDictionary>(ir, FIELD_NAME, WEIGHT_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    shared_ptr<IndexableField> weightField = doc->getField(WEIGHT_FIELD_NAME);
    assertEquals(inputIterator->weight(),
                 (weightField != nullptr)
                     ? weightField->numericValue()->longValue()
                     : 0);
    assertNull(inputIterator->payload());
  }

  for (auto invalidTerm : invalidDocTerms) {
    assertNotNull(docs.erase(invalidTerm));
  }

  assertTrue(docs.empty());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithContexts() throws
// java.io.IOException
void DocumentDictionaryTest::testWithContexts() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map::Entry<deque<wstring>,
                       unordered_map<wstring, std::shared_ptr<Document>>>
      res = generateIndexDocuments(atLeast(1000), true);
  unordered_map<wstring, std::shared_ptr<Document>> docs = res.getValue();
  deque<wstring> invalidDocTerms = res.getKey();
  for (auto doc : docs) {
    writer->addDocument(doc->second);
  }
  writer->commit();
  delete writer;
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentDictionary>(ir, FIELD_NAME, WEIGHT_FIELD_NAME,
                                      PAYLOAD_FIELD_NAME, CONTEXT_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    shared_ptr<IndexableField> weightField = doc->getField(WEIGHT_FIELD_NAME);
    assertEquals(inputIterator->weight(),
                 (weightField != nullptr)
                     ? weightField->numericValue()->longValue()
                     : 0);
    shared_ptr<IndexableField> payloadField = doc->getField(PAYLOAD_FIELD_NAME);
    if (payloadField == nullptr) {
      assertTrue(inputIterator->payload()->length == 0);
    } else {
      assertEquals(inputIterator->payload(), payloadField->binaryValue());
    }
    shared_ptr<Set<std::shared_ptr<BytesRef>>> oriCtxs =
        unordered_set<std::shared_ptr<BytesRef>>();
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contextSet =
        inputIterator->contexts();
    for (auto ctxf : doc->getFields(CONTEXT_FIELD_NAME)) {
      oriCtxs->add(ctxf->binaryValue());
    }
    assertEquals(oriCtxs->size(), contextSet->size());
  }

  for (auto invalidTerm : invalidDocTerms) {
    assertNotNull(docs.erase(invalidTerm));
  }
  assertTrue(docs.empty());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithDeletions() throws
// java.io.IOException
void DocumentDictionaryTest::testWithDeletions() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  unordered_map::Entry<deque<wstring>,
                       unordered_map<wstring, std::shared_ptr<Document>>>
      res = generateIndexDocuments(atLeast(1000), false);
  unordered_map<wstring, std::shared_ptr<Document>> docs = res.getValue();
  deque<wstring> invalidDocTerms = res.getKey();
  shared_ptr<Random> rand = random();
  deque<wstring> termsToDel = deque<wstring>();
  for (auto doc : docs) {
    shared_ptr<IndexableField> f = doc->second.getField(FIELD_NAME);
    if (rand->nextBoolean() && f != nullptr &&
        !find(invalidDocTerms.begin(), invalidDocTerms.end(),
              f->stringValue()) != invalidDocTerms.end()))
        {
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
  assertEquals(ir->numDocs(), docs.size());
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentDictionary>(ir, FIELD_NAME, WEIGHT_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  while ((f = inputIterator->next()) != nullptr) {
    shared_ptr<Document> doc = docs.erase(f->utf8ToString());
    assertTrue(f->equals(make_shared<BytesRef>(doc[FIELD_NAME])));
    shared_ptr<IndexableField> weightField = doc->getField(WEIGHT_FIELD_NAME);
    assertEquals(inputIterator->weight(),
                 (weightField != nullptr)
                     ? weightField->numericValue()->longValue()
                     : 0);
    assertNull(inputIterator->payload());
  }

  for (auto invalidTerm : invalidDocTerms) {
    assertNotNull(docs.erase(invalidTerm));
  }
  assertTrue(docs.empty());

  IOUtils::close({ir, analyzer, dir});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testMultiValuedField() throws
// java.io.IOException
void DocumentDictionaryTest::testMultiValuedField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc = newIndexWriterConfig(random(), analyzer);
  iwc->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir, iwc);

  deque<std::shared_ptr<Suggestion>> suggestions =
      indexMultiValuedDocuments(atLeast(1000), writer);
  writer->commit();
  delete writer;

  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  shared_ptr<Dictionary> dictionary =
      make_shared<DocumentDictionary>(ir, FIELD_NAME, WEIGHT_FIELD_NAME,
                                      PAYLOAD_FIELD_NAME, CONTEXT_FIELD_NAME);
  shared_ptr<InputIterator> inputIterator = dictionary->getEntryIterator();
  shared_ptr<BytesRef> f;
  deque<std::shared_ptr<Suggestion>>::const_iterator suggestionsIter =
      suggestions.begin();
  while ((f = inputIterator->next()) != nullptr) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    shared_ptr<Suggestion> nextSuggestion = suggestionsIter.next();
    assertTrue(f->equals(nextSuggestion->term));
    int64_t weight = nextSuggestion->weight;
    assertEquals(inputIterator->weight(), (weight != -1) ? weight : 0);
    assertEquals(inputIterator->payload(), nextSuggestion->payload);
    assertTrue(inputIterator->contexts()->equals(nextSuggestion->contexts));
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  assertFalse(suggestionsIter.hasNext());
  IOUtils::close({ir, analyzer, dir});
}

unordered_map::Entry<deque<wstring>,
                     unordered_map<wstring, std::shared_ptr<Document>>>
DocumentDictionaryTest::generateIndexDocuments(int ndocs, bool requiresContexts)
{
  unordered_map<wstring, std::shared_ptr<Document>> docs =
      unordered_map<wstring, std::shared_ptr<Document>>();
  deque<wstring> invalidDocTerms = deque<wstring>();
  for (int i = 0; i < ndocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    bool invalidDoc = false;
    shared_ptr<Field> field = nullptr;
    // usually have valid term field in document
    if (usually()) {
      field = make_shared<TextField>(FIELD_NAME, L"field_" + to_wstring(i),
                                     Field::Store::YES);
      doc->push_back(field);
    } else {
      invalidDoc = true;
    }

    // even if payload is not required usually have it
    if (usually()) {
      shared_ptr<Field> payload = make_shared<StoredField>(
          PAYLOAD_FIELD_NAME,
          make_shared<BytesRef>(L"payload_" + to_wstring(i)));
      doc->push_back(payload);
    }

    if (requiresContexts || usually()) {
      if (usually()) {
        for (int j = 0; j < atLeast(2); j++) {
          doc->push_back(make_shared<StoredField>(
              CONTEXT_FIELD_NAME,
              make_shared<BytesRef>(L"context_" + to_wstring(i) + L"_" +
                                    to_wstring(j))));
        }
      }
      // we should allow entries without context
    }

    // usually have valid weight field in document
    if (usually()) {
      shared_ptr<Field> weight =
          (rarely())
              ? make_shared<StoredField>(WEIGHT_FIELD_NAME, 100 + i)
              : make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME, 100 + i);
      doc->push_back(weight);
    }

    wstring term = L"";
    if (invalidDoc) {
      term = (field != nullptr) ? field->stringValue()
                                : L"invalid_" + to_wstring(i);
      invalidDocTerms.push_back(term);
    } else {
      term = field->stringValue();
    }

    docs.emplace(term, doc);
  }
  return make_shared<SimpleEntry<>>(invalidDocTerms, docs);
}

deque<std::shared_ptr<Suggestion>>
DocumentDictionaryTest::indexMultiValuedDocuments(
    int numDocs, shared_ptr<RandomIndexWriter> writer) 
{
  deque<std::shared_ptr<Suggestion>> suggestionList =
      deque<std::shared_ptr<Suggestion>>(numDocs);

  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<Field> field;
    shared_ptr<BytesRef> payloadValue;
    shared_ptr<Set<std::shared_ptr<BytesRef>>> contextValues =
        unordered_set<std::shared_ptr<BytesRef>>();
    int64_t numericValue = -1; //-1 for missing weight
    shared_ptr<BytesRef> term;

    payloadValue = make_shared<BytesRef>(L"payload_" + to_wstring(i));
    field = make_shared<StoredField>(PAYLOAD_FIELD_NAME, payloadValue);
    doc->push_back(field);

    if (usually()) {
      numericValue = 100 + i;
      field =
          make_shared<NumericDocValuesField>(WEIGHT_FIELD_NAME, numericValue);
      doc->push_back(field);
    }

    int numContexts = atLeast(1);
    for (int j = 0; j < numContexts; j++) {
      shared_ptr<BytesRef> contextValue = make_shared<BytesRef>(
          L"context_" + to_wstring(i) + L"_" + to_wstring(j));
      field = make_shared<StoredField>(CONTEXT_FIELD_NAME, contextValue);
      doc->push_back(field);
      contextValues->add(contextValue);
    }

    int numSuggestions = atLeast(2);
    for (int j = 0; j < numSuggestions; j++) {
      term = make_shared<BytesRef>(L"field_" + to_wstring(i) + L"_" +
                                   to_wstring(j));
      field = make_shared<StoredField>(FIELD_NAME, term);
      doc->push_back(field);

      shared_ptr<Suggestion> suggestionValue = make_shared<Suggestion>();
      suggestionValue->payload = payloadValue;
      suggestionValue->contexts = contextValues;
      suggestionValue->weight = numericValue;
      suggestionValue->term = term;
      suggestionList.push_back(suggestionValue);
    }
    writer->addDocument(doc);
  }
  return suggestionList;
}
} // namespace org::apache::lucene::search::suggest