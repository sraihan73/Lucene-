using namespace std;

#include "TestTotalHitCountCollector.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestTotalHitCountCollector::testBasics() 
{
  shared_ptr<Directory> indexStore = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), indexStore);
  for (int i = 0; i < 5; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(make_shared<StringField>(L"string", L"a" + to_wstring(i),
                                            Field::Store::NO));
    doc->push_back(make_shared<StringField>(L"string", L"b" + to_wstring(i),
                                            Field::Store::NO));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> reader = writer->getReader();
  delete writer;

  shared_ptr<IndexSearcher> searcher = newSearcher(reader);
  shared_ptr<TotalHitCountCollector> c = make_shared<TotalHitCountCollector>();
  searcher->search(make_shared<MatchAllDocsQuery>(), c);
  assertEquals(5, c->getTotalHits());
  delete reader;
  delete indexStore;
}
} // namespace org::apache::lucene::search