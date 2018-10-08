using namespace std;

#include "TestSearchForDuplicates.h"

namespace org::apache::lucene
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using TermQuery = org::apache::lucene::search::TermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
const wstring TestSearchForDuplicates::PRIORITY_FIELD = L"priority";
const wstring TestSearchForDuplicates::ID_FIELD = L"id";
const wstring TestSearchForDuplicates::HIGH_PRIORITY = L"high";
const wstring TestSearchForDuplicates::MED_PRIORITY = L"medium";
const wstring TestSearchForDuplicates::LOW_PRIORITY = L"low";

void TestSearchForDuplicates::testRun() 
{
  shared_ptr<StringWriter> sw = make_shared<StringWriter>();
  shared_ptr<PrintWriter> pw = make_shared<PrintWriter>(sw, true);
  constexpr int MAX_DOCS = atLeast(225);
  doTest(random(), pw, false, MAX_DOCS);
  pw->close();
  sw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring multiFileOutput = sw->toString();
  // System.out.println(multiFileOutput);

  sw = make_shared<StringWriter>();
  pw = make_shared<PrintWriter>(sw, true);
  doTest(random(), pw, true, MAX_DOCS);
  pw->close();
  sw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring singleFileOutput = sw->toString();

  assertEquals(multiFileOutput, singleFileOutput);
}

void TestSearchForDuplicates::doTest(shared_ptr<Random> random,
                                     shared_ptr<PrintWriter> out,
                                     bool useCompoundFiles,
                                     int MAX_DOCS) 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random);
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  shared_ptr<MergePolicy> *const mp = conf->getMergePolicy();
  mp->setNoCFSRatio(useCompoundFiles ? 1.0 : 0.0);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, conf);
  if (VERBOSE) {
    wcout << L"TEST: now build index MAX_DOCS=" << MAX_DOCS << endl;
  }

  for (int j = 0; j < MAX_DOCS; j++) {
    shared_ptr<Document> d = make_shared<Document>();
    d->push_back(
        newTextField(PRIORITY_FIELD, HIGH_PRIORITY, Field::Store::YES));
    d->push_back(make_shared<StoredField>(ID_FIELD, j));
    d->push_back(make_shared<NumericDocValuesField>(ID_FIELD, j));
    writer->addDocument(d);
  }
  delete writer;

  // try a search without OR
  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<Query> query =
      make_shared<TermQuery>(make_shared<Term>(PRIORITY_FIELD, HIGH_PRIORITY));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Query: " + query->toString(PRIORITY_FIELD));
  if (VERBOSE) {
    wcout << L"TEST: search query=" << query << endl;
  }

  shared_ptr<Sort> *const sort =
      make_shared<Sort>(SortField::FIELD_SCORE,
                        make_shared<SortField>(ID_FIELD, SortField::Type::INT));

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, MAX_DOCS, sort)->scoreDocs;
  printHits(out, hits, searcher);
  checkHits(hits, MAX_DOCS, searcher);

  // try a new search with OR
  searcher = newSearcher(reader);
  hits.clear();

  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(PRIORITY_FIELD, HIGH_PRIORITY)),
      BooleanClause::Occur::SHOULD);
  booleanQuery->add(
      make_shared<TermQuery>(make_shared<Term>(PRIORITY_FIELD, MED_PRIORITY)),
      BooleanClause::Occur::SHOULD);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  out->println(L"Query: " + booleanQuery->build()->toString(PRIORITY_FIELD));

  hits = searcher->search(booleanQuery->build(), MAX_DOCS, sort)->scoreDocs;
  printHits(out, hits, searcher);
  checkHits(hits, MAX_DOCS, searcher);

  delete reader;
  delete directory;
}

void TestSearchForDuplicates::printHits(
    shared_ptr<PrintWriter> out, std::deque<std::shared_ptr<ScoreDoc>> &hits,
    shared_ptr<IndexSearcher> searcher) 
{
  out->println(hits.size() + L" total results\n");
  for (int i = 0; i < hits.size(); i++) {
    if (i < 10 || (i > 94 && i < 105)) {
      shared_ptr<Document> d = searcher->doc(hits[i]->doc);
      out->println(to_wstring(i) + L" " + d[ID_FIELD]);
    }
  }
}

void TestSearchForDuplicates::checkHits(
    std::deque<std::shared_ptr<ScoreDoc>> &hits, int expectedCount,
    shared_ptr<IndexSearcher> searcher) 
{
  assertEquals(L"total results", expectedCount, hits.size());
  for (int i = 0; i < hits.size(); i++) {
    if (i < 10 || (i > 94 && i < 105)) {
      shared_ptr<Document> d = searcher->doc(hits[i]->doc);
      assertEquals(L"check " + to_wstring(i), to_wstring(i), d[ID_FIELD]);
    }
  }
}
} // namespace org::apache::lucene