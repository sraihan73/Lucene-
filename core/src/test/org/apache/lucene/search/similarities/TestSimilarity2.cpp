using namespace std;

#include "TestSimilarity2.h"

namespace org::apache::lucene::search::similarities
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using TopDocs = org::apache::lucene::search::TopDocs;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestSimilarity2::setUp() 
{
  LuceneTestCase::setUp();
  sims = deque<>();
  sims.push_back(make_shared<ClassicSimilarity>());
  sims.push_back(make_shared<BM25Similarity>());
  // TODO: not great that we dup this all with TestSimilarityBase
  for (auto basicModel : TestSimilarityBase::BASIC_MODELS) {
    for (auto afterEffect : TestSimilarityBase::AFTER_EFFECTS) {
      for (auto normalization : TestSimilarityBase::NORMALIZATIONS) {
        sims.push_back(
            make_shared<DFRSimilarity>(basicModel, afterEffect, normalization));
      }
    }
  }
  for (auto distribution : TestSimilarityBase::DISTRIBUTIONS) {
    for (auto lambda : TestSimilarityBase::LAMBDAS) {
      for (auto normalization : TestSimilarityBase::NORMALIZATIONS) {
        sims.push_back(
            make_shared<IBSimilarity>(distribution, lambda, normalization));
      }
    }
  }
  sims.push_back(make_shared<LMDirichletSimilarity>());
  sims.push_back(make_shared<LMJelinekMercerSimilarity>(0.1f));
  sims.push_back(make_shared<LMJelinekMercerSimilarity>(0.7f));
  for (auto independence : TestSimilarityBase::INDEPENDENCE_MEASURES) {
    sims.push_back(make_shared<DFISimilarity>(independence));
  }
}

void TestSimilarity2::testEmptyIndex() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    TestUtil::assertEquals(
        0, is->search(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                      10)
               ->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testEmptyField() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               BooleanClause::Occur::SHOULD);
    query->add(make_shared<TermQuery>(make_shared<Term>(L"bar", L"baz")),
               BooleanClause::Occur::SHOULD);
    TestUtil::assertEquals(1, is->search(query->build(), 10)->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testEmptyTerm() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"foo", L"bar", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               BooleanClause::Occur::SHOULD);
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
               BooleanClause::Occur::SHOULD);
    TestUtil::assertEquals(1, is->search(query->build(), 10)->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testNoNorms() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setOmitNorms(true);
  ft->freeze();
  doc->push_back(newField(L"foo", L"bar", ft));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               BooleanClause::Occur::SHOULD);
    TestUtil::assertEquals(1, is->search(query->build(), 10)->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testNoFieldSkew() 
{
  shared_ptr<Directory> dir = newDirectory();
  // an evil merge policy could reorder our docs for no reason
  shared_ptr<IndexWriterConfig> iwConfig =
      newIndexWriterConfig()->setMergePolicy(newLogMergePolicy());
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwConfig);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"foo", L"bar baz somethingelse", Field::Store::NO));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  shared_ptr<BooleanQuery::Builder> queryBuilder =
      make_shared<BooleanQuery::Builder>();
  queryBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    BooleanClause::Occur::SHOULD);
  queryBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                    BooleanClause::Occur::SHOULD);
  shared_ptr<Query> query = queryBuilder->build();

  // collect scores
  deque<std::shared_ptr<Explanation>> scores =
      deque<std::shared_ptr<Explanation>>();
  for (auto sim : sims) {
    is->setSimilarity(sim);
    scores.push_back(is->explain(query, 0));
  }
  delete ir;

  // add some additional docs without the field
  int numExtraDocs = TestUtil::nextInt(random(), 1, 1000);
  for (int i = 0; i < numExtraDocs; i++) {
    iw->addDocument(make_shared<Document>());
  }

  // check scores are the same
  ir = iw->getReader();
  is = newSearcher(ir);
  for (int i = 0; i < sims.size(); i++) {
    is->setSimilarity(sims[i]);
    shared_ptr<Explanation> expected = scores[i];
    shared_ptr<Explanation> actual = is->explain(query, 0);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertEquals(sims[i]->toString() + L": actual=" + actual + L",expected=" +
                     expected,
                 expected->getValue(), actual->getValue(), 0.0F);
  }

  delete iw;
  delete ir;
  delete dir;
}

void TestSimilarity2::testOmitTF() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->freeze();
  shared_ptr<Field> f = newField(L"foo", L"bar", ft);
  doc->push_back(f);
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               BooleanClause::Occur::SHOULD);
    TestUtil::assertEquals(1, is->search(query->build(), 10)->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testOmitTFAndNorms() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  ft->setIndexOptions(IndexOptions::DOCS);
  ft->setOmitNorms(true);
  ft->freeze();
  shared_ptr<Field> f = newField(L"foo", L"bar", ft);
  doc->push_back(f);
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<BooleanQuery::Builder> query =
        make_shared<BooleanQuery::Builder>();
    query->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               BooleanClause::Occur::SHOULD);
    TestUtil::assertEquals(1, is->search(query->build(), 10)->totalHits);
  }
  delete ir;
  delete dir;
}

void TestSimilarity2::testCrazySpans() 
{
  // The problem: "normal" lucene queries create scorers, returning null if
  // terms dont exist This means they never score a term that does not exist.
  // however with spans, there is only one scorer for the whole hierarchy:
  // inner queries are not real queries, their boosts are ignored, etc.
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  doc->push_back(newField(L"foo", L"bar", ft));
  iw->addDocument(doc);
  shared_ptr<IndexReader> ir = iw->getReader();
  delete iw;
  shared_ptr<IndexSearcher> is = newSearcher(ir);

  for (auto sim : sims) {
    is->setSimilarity(sim);
    shared_ptr<SpanTermQuery> s1 =
        make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"bar"));
    shared_ptr<SpanTermQuery> s2 =
        make_shared<SpanTermQuery>(make_shared<Term>(L"foo", L"baz"));
    shared_ptr<Query> query = make_shared<SpanOrQuery>(s1, s2);
    shared_ptr<TopDocs> td = is->search(query, 10);
    TestUtil::assertEquals(1, td->totalHits);
    float score = td->scoreDocs[0]->score;
    assertFalse(L"negative score for " + sim, score < 0.0f);
    assertFalse(L"inf score for " + sim, isinf(score));
    assertFalse(L"nan score for " + sim, isnan(score));
  }
  delete ir;
  delete dir;
}
} // namespace org::apache::lucene::search::similarities