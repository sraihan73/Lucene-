using namespace std;

#include "TestFieldCacheTermsFilter.h"

namespace org::apache::lucene::search
{
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;

void TestFieldCacheTermsFilter::testMissingTerms() 
{
  wstring fieldName = L"field1";
  shared_ptr<Directory> rd = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), rd);
  for (int i = 0; i < 100; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    int term = i * 10; // terms are units of 10;
    doc->push_back(
        newStringField(fieldName, L"" + to_wstring(term), Field::Store::YES));
    doc->push_back(make_shared<SortedDocValuesField>(
        fieldName, make_shared<BytesRef>(L"" + to_wstring(term))));
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  int numDocs = reader->numDocs();
  std::deque<std::shared_ptr<ScoreDoc>> results;

  deque<wstring> terms = deque<wstring>();
  terms.push_back(L"5");
  results = searcher
                ->search(make_shared<DocValuesTermsQuery>(
                             fieldName, terms.toArray(std::deque<wstring>(0))),
                         numDocs)
                ->scoreDocs;
  assertEquals(L"Must match nothing", 0, results.size());

  terms = deque<>();
  terms.push_back(L"10");
  results = searcher
                ->search(make_shared<DocValuesTermsQuery>(
                             fieldName, terms.toArray(std::deque<wstring>(0))),
                         numDocs)
                ->scoreDocs;
  assertEquals(L"Must match 1", 1, results.size());

  terms = deque<>();
  terms.push_back(L"10");
  terms.push_back(L"20");
  results = searcher
                ->search(make_shared<DocValuesTermsQuery>(
                             fieldName, terms.toArray(std::deque<wstring>(0))),
                         numDocs)
                ->scoreDocs;
  assertEquals(L"Must match 2", 2, results.size());

  delete reader;
  delete rd;
}
} // namespace org::apache::lucene::search