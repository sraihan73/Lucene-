using namespace std;

#include "TestSearch.h"

namespace org::apache::lucene
{
using namespace org::apache::lucene::analysis;
using namespace org::apache::lucene::document;
using namespace org::apache::lucene::index;
using namespace org::apache::lucene::search;
using namespace org::apache::lucene::store;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSearch::testNegativeQueryBoost() 
{
  shared_ptr<BoostQuery> q = make_shared<BoostQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), -42.0f);
  assertEquals(-42.0f, q->getBoost(), 0.0f);

  shared_ptr<Directory> directory = newDirectory();
  try {
    shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
    shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);

    shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, conf);
    try {
      shared_ptr<Document> d = make_shared<Document>();
      d->push_back(newTextField(L"foo", L"bar", Field::Store::YES));
      writer->addDocument(d);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete writer;
    }

    shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
    try {
      shared_ptr<IndexSearcher> searcher = newSearcher(reader);

      std::deque<std::shared_ptr<ScoreDoc>> hits =
          searcher->search(q, 1000)->scoreDocs;
      assertEquals(1, hits.size());
      assertTrue(L"score is positive: " + to_wstring(hits[0]->score),
                 hits[0]->score <= 0);

      shared_ptr<Explanation> explain = searcher->explain(q, hits[0]->doc);
      assertEquals(L"score doesn't match explanation", hits[0]->score,
                   explain->getValue(), 0.001f);
      assertTrue(L"explain doesn't think doc is a match", explain->isMatch());

    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      delete reader;
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete directory;
  }
}

void TestSearch::testSearch() 
{
  shared_ptr<StringWriter> sw = make_shared<StringWriter>();
  shared_ptr<PrintWriter> pw = make_shared<PrintWriter>(sw, true);
  doTestSearch(random(), pw, false);
  pw->close();
  sw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring multiFileOutput = sw->toString();
  // System.out.println(multiFileOutput);

  sw = make_shared<StringWriter>();
  pw = make_shared<PrintWriter>(sw, true);
  doTestSearch(random(), pw, true);
  pw->close();
  sw->close();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring singleFileOutput = sw->toString();

  assertEquals(multiFileOutput, singleFileOutput);
}

void TestSearch::doTestSearch(shared_ptr<Random> random,
                              shared_ptr<PrintWriter> out,
                              bool useCompoundFile) 
{
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random);
  shared_ptr<IndexWriterConfig> conf = newIndexWriterConfig(analyzer);
  shared_ptr<MergePolicy> mp = conf->getMergePolicy();
  mp->setNoCFSRatio(useCompoundFile ? 1.0 : 0.0);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, conf);

  std::deque<wstring> docs = {
      L"a b c d e", L"a b c d e a b c d e", L"a b c d e f g h i j", L"a c e",
      L"e c a",     L"a c e a c e",         L"a c e a b c"};
  for (int j = 0; j < docs.size(); j++) {
    shared_ptr<Document> d = make_shared<Document>();
    d->push_back(newTextField(L"contents", docs[j], Field::Store::YES));
    d->push_back(make_shared<NumericDocValuesField>(L"id", j));
    writer->addDocument(d);
  }
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  std::deque<std::shared_ptr<ScoreDoc>> hits;

  shared_ptr<Sort> sort =
      make_shared<Sort>(SortField::FIELD_SCORE,
                        make_shared<SortField>(L"id", SortField::Type::INT));

  for (auto query : buildQueries()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    out->println(L"Query: " + query->toString(L"contents"));
    if (VERBOSE) {
      wcout << L"TEST: query=" << query << endl;
    }

    hits = searcher->search(query, 1000, sort)->scoreDocs;

    out->println(hits.size() + L" total results");
    for (int i = 0; i < hits.size() && i < 10; i++) {
      shared_ptr<Document> d = searcher->doc(hits[i]->doc);
      out->println(to_wstring(i) + L" " + to_wstring(hits[i]->score) + L" " +
                   d[L"contents"]);
    }
  }
  delete reader;
  delete directory;
}

deque<std::shared_ptr<Query>> TestSearch::buildQueries()
{
  deque<std::shared_ptr<Query>> queries = deque<std::shared_ptr<Query>>();

  shared_ptr<BooleanQuery::Builder> booleanAB =
      make_shared<BooleanQuery::Builder>();
  booleanAB->add(make_shared<TermQuery>(make_shared<Term>(L"contents", L"a")),
                 BooleanClause::Occur::SHOULD);
  booleanAB->add(make_shared<TermQuery>(make_shared<Term>(L"contents", L"b")),
                 BooleanClause::Occur::SHOULD);
  queries.push_back(booleanAB->build());

  shared_ptr<PhraseQuery> phraseAB =
      make_shared<PhraseQuery>(L"contents", L"a", L"b");
  queries.push_back(phraseAB);

  shared_ptr<PhraseQuery> phraseABC =
      make_shared<PhraseQuery>(L"contents", L"a", L"b", L"c");
  queries.push_back(phraseABC);

  shared_ptr<BooleanQuery::Builder> booleanAC =
      make_shared<BooleanQuery::Builder>();
  booleanAC->add(make_shared<TermQuery>(make_shared<Term>(L"contents", L"a")),
                 BooleanClause::Occur::SHOULD);
  booleanAC->add(make_shared<TermQuery>(make_shared<Term>(L"contents", L"c")),
                 BooleanClause::Occur::SHOULD);
  queries.push_back(booleanAC->build());

  shared_ptr<PhraseQuery> phraseAC =
      make_shared<PhraseQuery>(L"contents", L"a", L"c");
  queries.push_back(phraseAC);

  shared_ptr<PhraseQuery> phraseACE =
      make_shared<PhraseQuery>(L"contents", L"a", L"c", L"e");
  queries.push_back(phraseACE);

  return queries;
}
} // namespace org::apache::lucene