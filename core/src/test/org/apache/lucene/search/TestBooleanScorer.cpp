using namespace std;

#include "TestBooleanScorer.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using Store = org::apache::lucene::document::Field::Store;
using StringField = org::apache::lucene::document::StringField;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using DefaultBulkScorer =
    org::apache::lucene::search::Weight::DefaultBulkScorer;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring TestBooleanScorer::FIELD = L"category";

void TestBooleanScorer::testMethod() 
{
  shared_ptr<Directory> directory = newDirectory();

  std::deque<wstring> values = {L"1", L"2", L"3", L"4"};

  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), directory);
  for (int i = 0; i < values.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(FIELD, values[i], Field::Store::YES));
    writer->addDocument(doc);
  }
  shared_ptr<IndexReader> ir = writer->getReader();
  delete writer;

  shared_ptr<BooleanQuery::Builder> booleanQuery1 =
      make_shared<BooleanQuery::Builder>();
  booleanQuery1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"1")),
                     BooleanClause::Occur::SHOULD);
  booleanQuery1->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"2")),
                     BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  query->add(booleanQuery1->build(), BooleanClause::Occur::MUST);
  query->add(make_shared<TermQuery>(make_shared<Term>(FIELD, L"9")),
             BooleanClause::Occur::MUST_NOT);

  shared_ptr<IndexSearcher> indexSearcher = newSearcher(ir);
  std::deque<std::shared_ptr<ScoreDoc>> hits =
      indexSearcher->search(query->build(), 1000)->scoreDocs;
  assertEquals(L"Number of matched documents", 2, hits.size());
  delete ir;
  delete directory;
}

wstring
TestBooleanScorer::CrazyMustUseBulkScorerQuery::toString(const wstring &field)
{
  return L"MustUseBulkScorerQuery";
}

shared_ptr<Weight> TestBooleanScorer::CrazyMustUseBulkScorerQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<WeightAnonymousInnerClass>(
      shared_from_this(), CrazyMustUseBulkScorerQuery::this);
}

// C++ TODO: You cannot use 'shared_from_this' in a constructor:
TestBooleanScorer::CrazyMustUseBulkScorerQuery::WeightAnonymousInnerClass::
    WeightAnonymousInnerClass(
        shared_ptr<CrazyMustUseBulkScorerQuery> outerInstance,
        shared_ptr<org::apache::lucene::search::TestBooleanScorer::
                       CrazyMustUseBulkScorerQuery>
            shared_from_this())
    : Weight(this)
{
  this->outerInstance = outerInstance;
}

void TestBooleanScorer::CrazyMustUseBulkScorerQuery::WeightAnonymousInnerClass::
    extractTerms(shared_ptr<Set<std::shared_ptr<Term>>> terms)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Explanation> TestBooleanScorer::CrazyMustUseBulkScorerQuery::
    WeightAnonymousInnerClass::explain(shared_ptr<LeafReaderContext> context,
                                       int doc)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Scorer> TestBooleanScorer::CrazyMustUseBulkScorerQuery::
    WeightAnonymousInnerClass::scorer(shared_ptr<LeafReaderContext> context)
{
  throw make_shared<UnsupportedOperationException>();
}

bool TestBooleanScorer::CrazyMustUseBulkScorerQuery::WeightAnonymousInnerClass::
    isCacheable(shared_ptr<LeafReaderContext> ctx)
{
  return false;
}

shared_ptr<BulkScorer> TestBooleanScorer::CrazyMustUseBulkScorerQuery::
    WeightAnonymousInnerClass::bulkScorer(shared_ptr<LeafReaderContext> context)
{
  return make_shared<BulkScorerAnonymousInnerClass>(shared_from_this());
}

TestBooleanScorer::CrazyMustUseBulkScorerQuery::WeightAnonymousInnerClass::
    BulkScorerAnonymousInnerClass::BulkScorerAnonymousInnerClass(
        shared_ptr<WeightAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

int TestBooleanScorer::CrazyMustUseBulkScorerQuery::WeightAnonymousInnerClass::
    BulkScorerAnonymousInnerClass::score(shared_ptr<LeafCollector> collector,
                                         shared_ptr<Bits> acceptDocs, int min,
                                         int max) 
{
  assert(min == 0);
  collector->setScorer(make_shared<FakeScorer>());
  collector->collect(0);
  return DocIdSetIterator::NO_MORE_DOCS;
}

int64_t TestBooleanScorer::CrazyMustUseBulkScorerQuery::
    WeightAnonymousInnerClass::BulkScorerAnonymousInnerClass::cost()
{
  return 1;
}

bool TestBooleanScorer::CrazyMustUseBulkScorerQuery::equals(any obj)
{
  return shared_from_this() == obj;
}

int TestBooleanScorer::CrazyMustUseBulkScorerQuery::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

void TestBooleanScorer::testEmbeddedBooleanScorer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(
      newTextField(L"field",
                   L"doctors are people who prescribe medicines of which they "
                   L"know little, to cure diseases of which they know less, in "
                   L"human beings of whom they know nothing",
                   Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = w->getReader();
  delete w;

  shared_ptr<IndexSearcher> s = make_shared<IndexSearcher>(r);
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"little")),
          BooleanClause::Occur::SHOULD);
  q1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"diseases")),
          BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(q1->build(), BooleanClause::Occur::SHOULD);
  q2->add(make_shared<CrazyMustUseBulkScorerQuery>(),
          BooleanClause::Occur::SHOULD);

  TestUtil::assertEquals(1, s->search(q2->build(), 10)->totalHits);
  delete r;
  delete dir;
}

void TestBooleanScorer::testOptimizeTopLevelClauseOrNull() 
{
  // When there is a single non-null scorer, this scorer should be used
  // directly
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // so that weights are not wrapped
  shared_ptr<LeafReaderContext> *const ctx = reader->leaves()[0];
  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->build();

  // no scores -> term scorer
  shared_ptr<Weight> weight =
      searcher->createWeight(searcher->rewrite(query), false, 1);
  shared_ptr<BulkScorer> scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<DefaultBulkScorer>(scorer) !=
             nullptr); // term scorer

  // scores -> term scorer too
  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::SHOULD)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                    Occur::SHOULD)
              ->build();
  weight = searcher->createWeight(searcher->rewrite(query), true, 1);
  scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<DefaultBulkScorer>(scorer) !=
             nullptr); // term scorer

  delete w;
  delete reader;
  delete dir;
}

void TestBooleanScorer::testOptimizeProhibitedClauses() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"bar", Field::Store::NO));
  doc->push_back(make_shared<StringField>(L"foo", L"baz", Field::Store::NO));
  w->addDocument(doc);
  doc = make_shared<Document>();
  doc->push_back(make_shared<StringField>(L"foo", L"baz", Field::Store::NO));
  w->addDocument(doc);
  w->forceMerge(1);
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // so that weights are not wrapped
  shared_ptr<LeafReaderContext> *const ctx = reader->leaves()[0];

  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST_NOT)
          ->build();
  shared_ptr<Weight> weight =
      searcher->createWeight(searcher->rewrite(query), true, 1);
  shared_ptr<BulkScorer> scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<ReqExclBulkScorer>(scorer) != nullptr);

  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                    Occur::SHOULD)
              ->add(make_shared<MatchAllDocsQuery>(), Occur::SHOULD)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::MUST_NOT)
              ->build();
  weight = searcher->createWeight(searcher->rewrite(query), true, 1);
  scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<ReqExclBulkScorer>(scorer) != nullptr);

  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                    Occur::MUST)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::MUST_NOT)
              ->build();
  weight = searcher->createWeight(searcher->rewrite(query), true, 1);
  scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<ReqExclBulkScorer>(scorer) != nullptr);

  query = (make_shared<BooleanQuery::Builder>())
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz")),
                    Occur::FILTER)
              ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                    Occur::MUST_NOT)
              ->build();
  weight = searcher->createWeight(searcher->rewrite(query), true, 1);
  scorer =
      (std::static_pointer_cast<BooleanWeight>(weight))->booleanScorer(ctx);
  assertTrue(std::dynamic_pointer_cast<ReqExclBulkScorer>(scorer) != nullptr);

  delete w;
  delete reader;
  delete dir;
}

void TestBooleanScorer::testSparseClauseOptimization() 
{
  // When some windows have only one scorer that can match, the scorer will
  // directly call the collector in this window
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> emptyDoc = make_shared<Document>();
  constexpr int numDocs = atLeast(10);
  for (int d = 0; d < numDocs; ++d) {
    for (int i = random()->nextInt(5000); i >= 0; --i) {
      w->addDocument(emptyDoc);
    }
    shared_ptr<Document> doc = make_shared<Document>();
    for (wstring value : Arrays::asList(L"foo", L"bar", L"baz")) {
      if (random()->nextBoolean()) {
        doc->push_back(
            make_shared<StringField>(L"field", value, Field::Store::NO));
      }
    }
  }
  for (int i = TestUtil::nextInt(random(), 3000, 5000); i >= 0; --i) {
    w->addDocument(emptyDoc);
  }
  if (random()->nextBoolean()) {
    w->forceMerge(1);
  }
  shared_ptr<IndexReader> reader = w->getReader();
  shared_ptr<IndexSearcher> searcher = newSearcher(reader);

  shared_ptr<Query> query =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<BoostQuery>(
                    make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
                    3),
                Occur::SHOULD)
          ->add(make_shared<BoostQuery>(
                    make_shared<TermQuery>(make_shared<Term>(L"field", L"bar")),
                    3),
                Occur::SHOULD)
          ->add(make_shared<BoostQuery>(
                    make_shared<TermQuery>(make_shared<Term>(L"field", L"baz")),
                    3),
                Occur::SHOULD)
          ->build();

  // duel BS1 vs. BS2
  QueryUtils::check(random(), query, searcher);

  delete reader;
  delete w;
  delete dir;
}
} // namespace org::apache::lucene::search