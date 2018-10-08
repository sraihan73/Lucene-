using namespace std;

#include "TestBooleanRewrites.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestBooleanRewrites::testOneClauseRewriteOptimization() throw(
    runtime_error)
{
  const wstring FIELD = L"content";
  const wstring VALUE = L"foo";

  shared_ptr<Directory> dir = newDirectory();
  delete (make_shared<RandomIndexWriter>(random(), dir));
  shared_ptr<IndexReader> r = DirectoryReader::open(dir);

  shared_ptr<TermQuery> expected =
      make_shared<TermQuery>(make_shared<Term>(FIELD, VALUE));

  constexpr int numLayers = atLeast(3);
  shared_ptr<Query> actual =
      make_shared<TermQuery>(make_shared<Term>(FIELD, VALUE));

  for (int i = 0; i < numLayers; i++) {

    shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
    bq->add(actual, random()->nextBoolean() ? BooleanClause::Occur::SHOULD
                                            : BooleanClause::Occur::MUST);
    actual = bq->build();
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(to_wstring(numLayers) + L": " + actual->toString(), expected,
               (make_shared<IndexSearcher>(r))->rewrite(actual));

  delete r;
  delete dir;
}

void TestBooleanRewrites::testSingleFilterClause() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);

  shared_ptr<BooleanQuery::Builder> query1 =
      make_shared<BooleanQuery::Builder>();
  query1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
              Occur::FILTER);

  // Single clauses rewrite to a term query
  shared_ptr<Query> *const rewritten1 = query1->build()->rewrite(reader);
  assertTrue(std::dynamic_pointer_cast<BoostQuery>(rewritten1) != nullptr);
  assertEquals(0.0f,
               (std::static_pointer_cast<BoostQuery>(rewritten1))->getBoost(),
               0.0f);

  // When there are two clauses, we cannot rewrite, but if one of them creates
  // a null scorer we will end up with a single filter scorer and will need to
  // make sure to set score=0
  shared_ptr<BooleanQuery::Builder> query2 =
      make_shared<BooleanQuery::Builder>();
  query2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
              Occur::FILTER);
  query2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
              Occur::SHOULD);
  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(query2->build()), true, 1);
  shared_ptr<Scorer> *const scorer = weight->scorer(reader->leaves()->get(0));
  assertEquals(0, scorer->begin().nextDoc());
  assertTrue(scorer->getClassName(),
             std::dynamic_pointer_cast<FilterScorer>(scorer) != nullptr);
  assertEquals(0.0f, scorer->score(), 0.0f);

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanRewrites::testSingleMustMatchAll() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  assertEquals(make_shared<ConstantScoreQuery>(
                   make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"))),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 42),
                 Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::FILTER)
           ->build();
  assertEquals(make_shared<BoostQuery>(
                   make_shared<ConstantScoreQuery>(make_shared<TermQuery>(
                       make_shared<Term>(L"foo", L"bar"))),
                   42),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<MatchAllDocsQuery>(), Occur::FILTER)
           ->build();
  assertEquals(make_shared<MatchAllDocsQuery>(), searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 42),
                 Occur::MUST)
           ->add(make_shared<MatchAllDocsQuery>(), Occur::FILTER)
           ->build();
  assertEquals(make_shared<BoostQuery>(make_shared<MatchAllDocsQuery>(), 42),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::MUST_NOT)
           ->build();
  assertEquals(bq, searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<MatchAllDocsQuery>(), Occur::FILTER)
           ->build();
  assertEquals(make_shared<MatchAllDocsQuery>(), searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::FILTER)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                 Occur::FILTER)
           ->build();
  shared_ptr<Query> expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::FILTER)
          ->build();
  assertEquals(make_shared<ConstantScoreQuery>(expected),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::FILTER)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                 Occur::MUST_NOT)
           ->build();
  expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::MUST_NOT)
          ->build();
  assertEquals(make_shared<ConstantScoreQuery>(expected),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::SHOULD)
           ->build();
  assertEquals(bq, searcher->rewrite(bq));
}

void TestBooleanRewrites::testSingleMustMatchAllWithShouldClauses() throw(
    IOException)
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::SHOULD)
          ->build();
  shared_ptr<BooleanQuery> expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<ConstantScoreQuery>(
                    make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar"))),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::SHOULD)
          ->build();
  assertEquals(expected, searcher->rewrite(bq));
}

void TestBooleanRewrites::testDeduplicateMustAndFilter() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::FILTER)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                 Occur::FILTER)
           ->build();
  shared_ptr<BooleanQuery> expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::FILTER)
          ->build();
  assertEquals(expected, searcher->rewrite(bq));
}

void TestBooleanRewrites::testConvertShouldAndFilterToMust() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  // no minShouldMatch
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               searcher->rewrite(bq));

  // minShouldMatch is set to -1
  bq = (make_shared<BooleanQuery::Builder>())
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::SHOULD)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::FILTER)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                 Occur::SHOULD)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quz")),
                 Occur::SHOULD)
           ->setMinimumNumberShouldMatch(2)
           ->build();

  shared_ptr<BooleanQuery> expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quz")),
                Occur::SHOULD)
          ->setMinimumNumberShouldMatch(1)
          ->build();
  assertEquals(expected, searcher->rewrite(bq));
}

void TestBooleanRewrites::testDuplicateMustOrFilterWithMustNot() throw(
    IOException)
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  // Test Must with MustNot
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bad")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST_NOT)
          ->build();

  assertEquals(make_shared<MatchNoDocsQuery>(), searcher->rewrite(bq));

  // Test Filter with MustNot
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bad")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST_NOT)
          ->build();

  assertEquals(make_shared<MatchNoDocsQuery>(), searcher->rewrite(bq2));
}

void TestBooleanRewrites::testMatchAllMustNot() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  // Test Must with MatchAll MustNot
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bad")),
                Occur::SHOULD)
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST_NOT)
          ->build();

  assertEquals(make_shared<MatchNoDocsQuery>(), searcher->rewrite(bq));

  // Test Must with MatchAll MustNot and other MustNot
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bad")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bor")),
                Occur::MUST_NOT)
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST_NOT)
          ->build();

  assertEquals(make_shared<MatchNoDocsQuery>(), searcher->rewrite(bq2));
}

void TestBooleanRewrites::testRemoveMatchAllFilter() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<MatchAllDocsQuery>(), Occur::FILTER)
          ->build();
  assertEquals(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
               searcher->rewrite(bq));

  bq = (make_shared<BooleanQuery::Builder>())
           ->setMinimumNumberShouldMatch(random()->nextInt(5))
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                 Occur::MUST)
           ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                 Occur::MUST)
           ->add(make_shared<MatchAllDocsQuery>(), Occur::FILTER)
           ->build();
  shared_ptr<BooleanQuery> expected =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(bq->getMinimumNumberShouldMatch())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::MUST)
          ->build();
  assertEquals(expected, searcher->rewrite(bq));
}

void TestBooleanRewrites::testRandom() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<TextField> f =
      make_shared<TextField>(L"body", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  f->setStringValue(L"");
  w->addDocument(doc);
  f->setStringValue(L"a b");
  w->addDocument(doc);
  f->setStringValue(L"b c");
  w->addDocument(doc);
  f->setStringValue(L"a");
  w->addDocument(doc);
  f->setStringValue(L"c");
  w->addDocument(doc);
  constexpr int numRandomDocs = atLeast(3);
  for (int i = 0; i < numRandomDocs; ++i) {
    constexpr int numTerms = random()->nextInt(20);
    shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
    for (int j = 0; j < numTerms; ++j) {
      text->append(static_cast<wchar_t>(L'a' + random()->nextInt(4)))
          ->append(L' ');
    }
    f->setStringValue(text->toString());
    w->addDocument(doc);
  }
  shared_ptr<IndexReader> *const reader = w->getReader();
  delete w;
  shared_ptr<IndexSearcher> *const searcher1 = newSearcher(reader);
  shared_ptr<IndexSearcher> *const searcher2 =
      make_shared<IndexSearcherAnonymousInnerClass>(shared_from_this());
  searcher2->setSimilarity(searcher1->getSimilarity(true));

  constexpr int iters = atLeast(1000);
  for (int i = 0; i < iters; ++i) {
    shared_ptr<Query> query = randomQuery();
    shared_ptr<TopDocs> *const td1 = searcher1->search(query, 100);
    shared_ptr<TopDocs> *const td2 = searcher2->search(query, 100);
    assertEquals(td1, td2);
  }

  delete searcher1->getIndexReader();
  delete dir;
}

TestBooleanRewrites::IndexSearcherAnonymousInnerClass::
    IndexSearcherAnonymousInnerClass(
        shared_ptr<TestBooleanRewrites> outerInstance)
    : IndexSearcher(reader)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Query>
TestBooleanRewrites::IndexSearcherAnonymousInnerClass::rewrite(
    shared_ptr<Query> original) 
{
  // no-op: disable rewriting
  return original;
}

shared_ptr<Query> TestBooleanRewrites::randomBooleanQuery()
{
  if (random()->nextInt(10) == 0) {
    return make_shared<BoostQuery>(randomBooleanQuery(),
                                   TestUtil::nextInt(random(), 1, 10));
  }
  constexpr int numClauses = random()->nextInt(5);
  shared_ptr<BooleanQuery::Builder> b = make_shared<BooleanQuery::Builder>();
  int numShoulds = 0;
  for (int i = 0; i < numClauses; ++i) {
    constexpr Occur occur =
        Occur::values()[random()->nextInt(Occur::values()->length)];
    if (occur == Occur::SHOULD) {
      numShoulds++;
    }
    shared_ptr<Query> *const query = randomQuery();
    b->add(query, occur);
  }
  b->setMinimumNumberShouldMatch(
      random()->nextBoolean() ? 0
                              : TestUtil::nextInt(random(), 0, numShoulds + 1));
  return b->build();
}

shared_ptr<Query> TestBooleanRewrites::randomQuery()
{
  if (random()->nextInt(10) == 0) {
    return make_shared<BoostQuery>(randomBooleanQuery(),
                                   TestUtil::nextInt(random(), 1, 10));
  }
  switch (random()->nextInt(6)) {
  case 0:
    return make_shared<MatchAllDocsQuery>();
  case 1:
    return make_shared<TermQuery>(make_shared<Term>(L"body", L"a"));
  case 2:
    return make_shared<TermQuery>(make_shared<Term>(L"body", L"b"));
  case 3:
    return make_shared<TermQuery>(make_shared<Term>(L"body", L"c"));
  case 4:
    return make_shared<TermQuery>(make_shared<Term>(L"body", L"d"));
  case 5:
    return randomBooleanQuery();
  default:
    throw make_shared<AssertionError>();
  }
}

void TestBooleanRewrites::assertEquals(shared_ptr<TopDocs> td1,
                                       shared_ptr<TopDocs> td2)
{
  assertEquals(td1->totalHits, td2->totalHits);
  assertEquals(td1->scoreDocs.size(), td2->scoreDocs.size());
  unordered_map<int, float> expectedScores =
      Arrays::stream(td1->scoreDocs)
          .collect(Collectors::toMap([&](any sd) { sd::doc; },
                                     [&](any sd) { sd::score; }));
  shared_ptr<Set<int>> actualResultSet = Arrays::stream(td2->scoreDocs)
                                             .map_obj([&](any sd) { sd::doc; })
                                             .collect(Collectors::toSet());

  assertEquals(L"Set of matching documents differs", expectedScores.keySet(),
               actualResultSet);

  for (auto scoreDoc : td2->scoreDocs) {
    constexpr float expectedScore = expectedScores[scoreDoc->doc];
    constexpr float actualScore = scoreDoc->score;
    assertEquals(expectedScore, actualScore,
                 expectedScore / 100); // error under 1%
  }
}

void TestBooleanRewrites::testDeduplicateShouldClauses() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->build();
  shared_ptr<Query> expected = make_shared<BoostQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2);
  assertEquals(expected, searcher->rewrite(query));

  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(
              make_shared<BoostQuery>(
                  make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2),
              Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::SHOULD)
          ->build();
  expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(
              make_shared<BoostQuery>(
                  make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 3),
              Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::SHOULD)
          ->build();
  assertEquals(expected, searcher->rewrite(query));

  query = (make_shared<BooleanQuery::Builder>())
              ->setMinimumNumberShouldMatch(2)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::SHOULD)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::SHOULD)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                    Occur::SHOULD)
              ->build();
  expected = query;
  assertEquals(expected, searcher->rewrite(query));
}

void TestBooleanRewrites::testDeduplicateMustClauses() 
{
  shared_ptr<IndexSearcher> searcher = newSearcher(make_shared<MultiReader>());

  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->build();
  shared_ptr<Query> expected = make_shared<BoostQuery>(
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2);
  assertEquals(expected, searcher->rewrite(query));

  query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(
              make_shared<BoostQuery>(
                  make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 2),
              Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::MUST)
          ->build();
  expected =
      (make_shared<BooleanQuery::Builder>())
          ->add(
              make_shared<BoostQuery>(
                  make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")), 3),
              Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"quux")),
                Occur::MUST)
          ->build();
  assertEquals(expected, searcher->rewrite(query));
}
} // namespace org::apache::lucene::search