using namespace std;

#include "TestCustomSearcherSort.h"

namespace org::apache::lucene::search
{
using DateTools = org::apache::lucene::document::DateTools;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestCustomSearcherSort::setUp() 
{
  LuceneTestCase::setUp();
  INDEX_SIZE = atLeast(2000);
  index = newDirectory();
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), index);
  shared_ptr<RandomGen> random =
      make_shared<RandomGen>(TestCustomSearcherSort::random());
  for (int i = 0; i < INDEX_SIZE; ++i) { // don't decrease; if to low the
                                         // problem doesn't show up
    shared_ptr<Document> doc = make_shared<Document>();
    if ((i % 5) != 0) { // some documents must not have an entry in the first
                        // sort field
      doc->push_back(make_shared<SortedDocValuesField>(
          L"publicationDate_", make_shared<BytesRef>(random->getLuceneDate())));
    }
    if ((i % 7) == 0) { // some documents to match the query (see below)
      doc->push_back(newTextField(L"content", L"test", Field::Store::YES));
    }
    // every document has a defined 'mandant' field
    // C++ TODO: There is no native C++ equivalent to 'toString':
    doc->push_back(newStringField(L"mandant", Integer::toString(i % 3),
                                  Field::Store::YES));
    writer->addDocument(doc);
  }
  reader = writer->getReader();
  delete writer;
  query = make_shared<TermQuery>(make_shared<Term>(L"content", L"test"));
}

void TestCustomSearcherSort::tearDown() 
{
  delete reader;
  delete index;
  LuceneTestCase::tearDown();
}

void TestCustomSearcherSort::testFieldSortCustomSearcher() 
{
  // log("Run testFieldSortCustomSearcher");
  // define the sort criteria
  shared_ptr<Sort> custSort = make_shared<Sort>(
      make_shared<SortField>(L"publicationDate_", SortField::Type::STRING),
      SortField::FIELD_SCORE);
  shared_ptr<IndexSearcher> searcher = make_shared<CustomSearcher>(reader, 2);
  // search and check hits
  matchHits(searcher, custSort);
}

void TestCustomSearcherSort::testFieldSortSingleSearcher() 
{
  // log("Run testFieldSortSingleSearcher");
  // define the sort criteria
  shared_ptr<Sort> custSort = make_shared<Sort>(
      make_shared<SortField>(L"publicationDate_", SortField::Type::STRING),
      SortField::FIELD_SCORE);
  shared_ptr<IndexSearcher> searcher = make_shared<CustomSearcher>(reader, 2);
  // search and check hits
  matchHits(searcher, custSort);
}

void TestCustomSearcherSort::matchHits(shared_ptr<IndexSearcher> searcher,
                                       shared_ptr<Sort> sort) 
{
  // make a query without sorting first
  std::deque<std::shared_ptr<ScoreDoc>> hitsByRank =
      searcher->search(query, numeric_limits<int>::max())->scoreDocs;
  checkHits(hitsByRank, L"Sort by rank: "); // check for duplicates
  unordered_map<int, int> resultMap = map_obj<int, int>();
  // store hits in TreeMap - TreeMap does not allow duplicates; existing
  // entries are silently overwritten
  for (int hitid = 0; hitid < hitsByRank.size(); ++hitid) {
    resultMap.emplace(static_cast<Integer>(hitsByRank[hitid]->doc),
                      static_cast<Integer>(hitid)); // Value: Hits-Objekt Index
  }

  // now make a query using the sort criteria
  std::deque<std::shared_ptr<ScoreDoc>> resultSort =
      searcher->search(query, numeric_limits<int>::max(), sort)->scoreDocs;
  checkHits(resultSort, L"Sort by custom criteria: "); // check for duplicates

  // besides the sorting both sets of hits must be identical
  for (int hitid = 0; hitid < resultSort.size(); ++hitid) {
    optional<int> idHitDate =
        static_cast<Integer>(resultSort[hitid]->doc); // document ID
                                                      // from sorted
                                                      // search
    if (resultMap.find(idHitDate) == resultMap.end()) {
      log(L"ID " + idHitDate + L" not found. Possibliy a duplicate.");
    }
    assertTrue(resultMap.find(idHitDate) !=
               resultMap.end()); // same ID must be in the
                                 // Map from the rank-sorted
                                 // search
    // every hit must appear once in both result sets --> remove it from the
    // Map.
    // At the end the Map must be empty!
    resultMap.erase(idHitDate);
  }
  if (resultMap.empty()) {
    // log("All hits matched");
  } else {
    log(L"Couldn't match " + resultMap.size() + L" hits.");
  }
  assertEquals(resultMap.size(), 0);
}

void TestCustomSearcherSort::checkHits(
    std::deque<std::shared_ptr<ScoreDoc>> &hits, const wstring &prefix)
{
  if (hits.size() > 0) {
    unordered_map<int, int> idMap = map_obj<int, int>();
    for (int docnum = 0; docnum < hits.size(); ++docnum) {
      optional<int> luceneId = nullopt;

      luceneId = static_cast<Integer>(hits[docnum]->doc);
      if (idMap.find(luceneId) != idMap.end()) {
        shared_ptr<StringBuilder> message = make_shared<StringBuilder>(prefix);
        message->append(L"Duplicate key for hit index = ");
        message->append(docnum);
        message->append(L", previous index = ");
        message->append(to_wstring(idMap[luceneId]));
        message->append(L", Lucene ID = ");
        message->append(luceneId);
        log(message->toString());
      } else {
        idMap.emplace(luceneId, static_cast<Integer>(docnum));
      }
    }
  }
}

void TestCustomSearcherSort::log(const wstring &message)
{
  if (VERBOSE) {
    wcout << message << endl;
  }
}

TestCustomSearcherSort::CustomSearcher::CustomSearcher(
    shared_ptr<IndexReader> r, int switcher)
    : IndexSearcher(r)
{
  this->switcher = switcher;
}

shared_ptr<TopFieldDocs> TestCustomSearcherSort::CustomSearcher::search(
    shared_ptr<Query> query, int nDocs,
    shared_ptr<Sort> sort) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(query, BooleanClause::Occur::MUST);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  bq->add(make_shared<TermQuery>(
              make_shared<Term>(L"mandant", Integer::toString(switcher))),
          BooleanClause::Occur::MUST);
  return IndexSearcher::search(bq->build(), nDocs, sort);
}

shared_ptr<TopDocs>
TestCustomSearcherSort::CustomSearcher::search(shared_ptr<Query> query,
                                               int nDocs) 
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(query, BooleanClause::Occur::MUST);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  bq->add(make_shared<TermQuery>(
              make_shared<Term>(L"mandant", Integer::toString(switcher))),
          BooleanClause::Occur::MUST);
  return IndexSearcher::search(bq->build(), nDocs);
}

TestCustomSearcherSort::RandomGen::RandomGen(shared_ptr<Random> random)
{
  this->random = random;
  base->set(1980, 1, 1);
}

wstring TestCustomSearcherSort::RandomGen::getLuceneDate()
{
  return DateTools::timeToString(base->getTimeInMillis() + random->nextInt() -
                                     numeric_limits<int>::min(),
                                 DateTools::Resolution::DAY);
}
} // namespace org::apache::lucene::search