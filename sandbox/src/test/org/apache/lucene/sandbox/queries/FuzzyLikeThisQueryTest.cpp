using namespace std;

#include "FuzzyLikeThisQueryTest.h"

namespace org::apache::lucene::sandbox::queries
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void FuzzyLikeThisQueryTest::setUp() 
{
  LuceneTestCase::setUp();

  analyzer = make_shared<MockAnalyzer>(random());
  directory = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), directory,
      newIndexWriterConfig(analyzer)->setMergePolicy(newLogMergePolicy()));

  // Add series of docs with misspelt names
  addDoc(writer, L"jonathon smythe", L"1");
  addDoc(writer, L"jonathan smith", L"2");
  addDoc(writer, L"johnathon smyth", L"3");
  addDoc(writer, L"johnny smith", L"4");
  addDoc(writer, L"jonny smith", L"5");
  addDoc(writer, L"johnathon smythe", L"6");
  reader = writer->getReader();
  delete writer;
  searcher = newSearcher(reader);
}

void FuzzyLikeThisQueryTest::tearDown() 
{
  IOUtils::close({reader, directory, analyzer});
  LuceneTestCase::tearDown();
}

void FuzzyLikeThisQueryTest::addDoc(shared_ptr<RandomIndexWriter> writer,
                                    const wstring &name,
                                    const wstring &id) 
{
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"name", name, Field::Store::YES));
  doc->push_back(newTextField(L"id", id, Field::Store::YES));
  writer->addDocument(doc);
}

void FuzzyLikeThisQueryTest::testClosestEditDistanceMatchComesFirst() throw(
    runtime_error)
{
  shared_ptr<FuzzyLikeThisQuery> flt =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  flt->addTerms(L"smith", L"name", 2, 1);
  shared_ptr<Query> q = flt->rewrite(searcher->getIndexReader());
  unordered_set<std::shared_ptr<Term>> queryTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(q, true, 1.0f)->extractTerms(queryTerms);
      assertTrue(L"Should have variant smythe", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smythe")) != queryTerms.end()));
      assertTrue(L"Should have variant smith", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smith")) != queryTerms.end()));
      assertTrue(L"Should have variant smyth", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smyth")) != queryTerms.end()));
      shared_ptr<TopDocs> topDocs = searcher->search(flt, 1);
      std::deque<std::shared_ptr<ScoreDoc>> sd = topDocs->scoreDocs;
      assertTrue(L"score docs must match 1 doc",
                 (sd.size() > 0) && (sd.size() > 0));
      shared_ptr<Document> doc = searcher->doc(sd[0]->doc);
      assertEquals(L"Should match most similar not most rare variant", L"2",
                   doc[L"id"]);
}

void FuzzyLikeThisQueryTest::testMultiWord() 
{
  shared_ptr<FuzzyLikeThisQuery> flt =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  flt->addTerms(L"jonathin smoth", L"name", 2, 1);
  shared_ptr<Query> q = flt->rewrite(searcher->getIndexReader());
  unordered_set<std::shared_ptr<Term>> queryTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(q, true, 1.0f)->extractTerms(queryTerms);
      assertTrue(L"Should have variant jonathan", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"jonathan")) != queryTerms.end()));
      assertTrue(L"Should have variant smith", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smith")) != queryTerms.end()));
      shared_ptr<TopDocs> topDocs = searcher->search(flt, 1);
      std::deque<std::shared_ptr<ScoreDoc>> sd = topDocs->scoreDocs;
      assertTrue(L"score docs must match 1 doc",
                 (sd.size() > 0) && (sd.size() > 0));
      shared_ptr<Document> doc = searcher->doc(sd[0]->doc);
      assertEquals(L"Should match most similar when using 2 words", L"2",
                   doc[L"id"]);
}

void FuzzyLikeThisQueryTest::testNonExistingField() 
{
  shared_ptr<FuzzyLikeThisQuery> flt =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  flt->addTerms(L"jonathin smoth", L"name", 2, 1);
  flt->addTerms(L"jonathin smoth", L"this field does not exist", 2, 1);
  // don't fail here just because the field doesn't exits
  shared_ptr<Query> q = flt->rewrite(searcher->getIndexReader());
  unordered_set<std::shared_ptr<Term>> queryTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(q, true, 1.0f)->extractTerms(queryTerms);
      assertTrue(L"Should have variant jonathan", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"jonathan")) != queryTerms.end()));
      assertTrue(L"Should have variant smith", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smith")) != queryTerms.end()));
      shared_ptr<TopDocs> topDocs = searcher->search(flt, 1);
      std::deque<std::shared_ptr<ScoreDoc>> sd = topDocs->scoreDocs;
      assertTrue(L"score docs must match 1 doc",
                 (sd.size() > 0) && (sd.size() > 0));
      shared_ptr<Document> doc = searcher->doc(sd[0]->doc);
      assertEquals(L"Should match most similar when using 2 words", L"2",
                   doc[L"id"]);
}

void FuzzyLikeThisQueryTest::testNoMatchFirstWordBug() 
{
  shared_ptr<FuzzyLikeThisQuery> flt =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  flt->addTerms(L"fernando smith", L"name", 2, 1);
  shared_ptr<Query> q = flt->rewrite(searcher->getIndexReader());
  unordered_set<std::shared_ptr<Term>> queryTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(q, true, 1.0f)->extractTerms(queryTerms);
      assertTrue(L"Should have variant smith", find(queryTerms.begin(), queryTerms.end(), make_shared<Term>(L"name", L"smith")) != queryTerms.end()));
      shared_ptr<TopDocs> topDocs = searcher->search(flt, 1);
      std::deque<std::shared_ptr<ScoreDoc>> sd = topDocs->scoreDocs;
      assertTrue(L"score docs must match 1 doc",
                 (sd.size() > 0) && (sd.size() > 0));
      shared_ptr<Document> doc = searcher->doc(sd[0]->doc);
      assertEquals(L"Should match most similar when using 2 words", L"2",
                   doc[L"id"]);
}

void FuzzyLikeThisQueryTest::testFuzzyLikeThisQueryEquals()
{
  shared_ptr<Analyzer> analyzer = make_shared<MockAnalyzer>(random());
  shared_ptr<FuzzyLikeThisQuery> fltq1 =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  fltq1->addTerms(L"javi", L"subject", 2, 2);
  shared_ptr<FuzzyLikeThisQuery> fltq2 =
      make_shared<FuzzyLikeThisQuery>(10, analyzer);
  fltq2->addTerms(L"javi", L"subject", 2, 2);
  assertEquals(L"FuzzyLikeThisQuery with same attributes is not equal", fltq1,
               fltq2);
}
} // namespace org::apache::lucene::sandbox::queries