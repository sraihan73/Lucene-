using namespace std;

#include "TestLazyDocument.h"

namespace org::apache::lucene::document
{
using namespace org::apache::lucene::analysis;
using namespace org::apache::lucene::index;
using namespace org::apache::lucene::search;
using namespace org::apache::lucene::store;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::After;
using org::junit::Before;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @After public void removeIndex()
void TestLazyDocument::removeIndex()
{
  if (nullptr != dir) {
    try {
      delete dir;
      dir.reset();
    } catch (const runtime_error &e) {
    }
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Before public void createIndex() throws Exception
void TestLazyDocument::createIndex() 
{

  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, newIndexWriterConfig(analyzer));
  try {
    for (int docid = 0; docid < NUM_DOCS; docid++) {
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(
          newStringField(L"docid", L"" + to_wstring(docid), Field::Store::YES));
      d->push_back(newStringField(L"never_load", L"fail", Field::Store::YES));
      for (auto f : FIELDS) {
        for (int val = 0; val < NUM_VALUES; val++) {
          d->push_back(newStringField(
              f, to_wstring(docid) + L"_" + f + L"_" + to_wstring(val),
              Field::Store::YES));
        }
      }
      d->push_back(newStringField(L"load_later", L"yes", Field::Store::YES));
      writer->addDocument(d);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete writer;
  }
}

void TestLazyDocument::testLazy() 
{
  constexpr int id = random()->nextInt(NUM_DOCS);
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  try {
    shared_ptr<Query> q = make_shared<TermQuery>(
        make_shared<Term>(L"docid", L"" + to_wstring(id)));
    shared_ptr<IndexSearcher> searcher = newSearcher(reader);
    std::deque<std::shared_ptr<ScoreDoc>> hits =
        searcher->search(q, 100)->scoreDocs;
    assertEquals(L"Too many docs", 1, hits.size());
    shared_ptr<LazyTestingStoredFieldVisitor> visitor =
        make_shared<LazyTestingStoredFieldVisitor>(
            make_shared<LazyDocument>(reader, hits[0]->doc), FIELDS);
    reader->document(hits[0]->doc, visitor);
    shared_ptr<Document> d = visitor->doc;

    int numFieldValues = 0;
    unordered_map<wstring, int> fieldValueCounts =
        unordered_map<wstring, int>();

    // at this point, all FIELDS should be Lazy and unrealized
    for (auto f : d) {
      numFieldValues++;
      if (f->name() == L"never_load") {
        fail(L"never_load was loaded");
      }
      if (f->name() == L"load_later") {
        fail(L"load_later was loaded on first pass");
      }
      if (f->name() == L"docid") {
        assertFalse(f->name(),
                    std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                        nullptr);
      } else {
        int count = fieldValueCounts.find(f->name()) != fieldValueCounts.end()
                        ? fieldValueCounts[f->name()]
                        : 0;
        count++;
        fieldValueCounts.emplace(f->name(), count);
        assertTrue(f->name() + L" is " + f->getClass(),
                   std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                       nullptr);
        shared_ptr<LazyDocument::LazyField> lf =
            std::static_pointer_cast<LazyDocument::LazyField>(f);
        assertFalse(f->name() + L" is loaded", lf->hasBeenLoaded());
      }
    }
    if (VERBOSE) {
      wcout << L"numFieldValues == " << numFieldValues << endl;
    }
    assertEquals(L"numFieldValues", 1 + (NUM_VALUES * FIELDS.size()),
                 numFieldValues);

    for (auto fieldName : fieldValueCounts) {
      assertEquals(L"fieldName count: " + fieldName.first, NUM_VALUES,
                   static_cast<int>(fieldValueCounts[fieldName.first]));
    }

    // pick a single field name to load a single value
    const wstring fieldName = FIELDS[random()->nextInt(FIELDS.size())];
    std::deque<std::shared_ptr<IndexableField>> fieldValues =
        d->getFields(fieldName);
    assertEquals(L"#vals in field: " + fieldName, NUM_VALUES,
                 fieldValues.size());
    constexpr int valNum = random()->nextInt(fieldValues.size());
    assertEquals(to_wstring(id) + L"_" + fieldName + L"_" + to_wstring(valNum),
                 fieldValues[valNum]->stringValue());

    // now every value of fieldName should be loaded
    for (auto f : d) {
      if (f->name() == L"never_load") {
        fail(L"never_load was loaded");
      }
      if (f->name() == L"load_later") {
        fail(L"load_later was loaded too soon");
      }
      if (f->name() == L"docid") {
        assertFalse(f->name(),
                    std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                        nullptr);
      } else {
        assertTrue(f->name() + L" is " + f->getClass(),
                   std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                       nullptr);
        shared_ptr<LazyDocument::LazyField> lf =
            std::static_pointer_cast<LazyDocument::LazyField>(f);
        assertEquals(f->name() + L" is loaded?", lf->name() == fieldName,
                     lf->hasBeenLoaded());
      }
    }

    // use the same LazyDoc to ask for one more lazy field
    visitor = make_shared<LazyTestingStoredFieldVisitor>(
        make_shared<LazyDocument>(reader, hits[0]->doc), L"load_later");
    reader->document(hits[0]->doc, visitor);
    d = visitor->doc;

    // ensure we have all the values we expect now, and that
    // adding one more lazy field didn't "unload" the existing LazyField's
    // we already loaded.
    for (auto f : d) {
      if (f->name() == L"never_load") {
        fail(L"never_load was loaded");
      }
      if (f->name() == L"docid") {
        assertFalse(f->name(),
                    std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                        nullptr);
      } else {
        assertTrue(f->name() + L" is " + f->getClass(),
                   std::dynamic_pointer_cast<LazyDocument::LazyField>(f) !=
                       nullptr);
        shared_ptr<LazyDocument::LazyField> lf =
            std::static_pointer_cast<LazyDocument::LazyField>(f);
        assertEquals(f->name() + L" is loaded?", lf->name() == fieldName,
                     lf->hasBeenLoaded());
      }
    }

    // even the underlying doc shouldn't have never_load
    assertNull(L"never_load was loaded in wrapped doc",
               visitor->lazyDoc->getDocument()->getField(L"never_load"));

  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
  }
}

TestLazyDocument::LazyTestingStoredFieldVisitor::LazyTestingStoredFieldVisitor(
    shared_ptr<LazyDocument> l, deque<wstring> &fields)
    : lazyDoc(l), lazyFieldNames(unordered_set<>(Arrays::asList(fields)))
{
}

Status TestLazyDocument::LazyTestingStoredFieldVisitor::needsField(
    shared_ptr<FieldInfo> fieldInfo)
{
  if (fieldInfo->name == L"docid") {
    return Status::YES;
  } else if (fieldInfo->name == L"never_load") {
    return Status::NO;
  } else {
    if (lazyFieldNames->contains(fieldInfo->name)) {
      doc->push_back(lazyDoc->getField(fieldInfo));
    }
  }
  return Status::NO;
}

void TestLazyDocument::LazyTestingStoredFieldVisitor::stringField(
    shared_ptr<FieldInfo> fieldInfo,
    std::deque<char> &bytes) 
{
  wstring value = wstring(bytes, StandardCharsets::UTF_8);
  shared_ptr<FieldType> *const ft =
      make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(fieldInfo->hasVectors());
  ft->setOmitNorms(fieldInfo->omitsNorms());
  ft->setIndexOptions(fieldInfo->getIndexOptions());
  doc->push_back(make_shared<Field>(fieldInfo->name, value, ft));
}
} // namespace org::apache::lucene::document