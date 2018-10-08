using namespace std;

#include "TestBooleanOr.h"

namespace org::apache::lucene::search
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
wstring TestBooleanOr::FIELD_T = L"T";
wstring TestBooleanOr::FIELD_C = L"C";

int64_t TestBooleanOr::search(shared_ptr<Query> q) 
{
  QueryUtils::check(random(), q, searcher);
  return searcher->search(q, 1000)->totalHits;
}

void TestBooleanOr::testElements() 
{
  TestUtil::assertEquals(1, search(t1));
  TestUtil::assertEquals(1, search(t2));
  TestUtil::assertEquals(1, search(c1));
  TestUtil::assertEquals(1, search(c2));
}

void TestBooleanOr::testFlat() 
{
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<BooleanClause>(t1, BooleanClause::Occur::SHOULD));
  q->add(make_shared<BooleanClause>(t2, BooleanClause::Occur::SHOULD));
  q->add(make_shared<BooleanClause>(c1, BooleanClause::Occur::SHOULD));
  q->add(make_shared<BooleanClause>(c2, BooleanClause::Occur::SHOULD));
  TestUtil::assertEquals(1, search(q->build()));
}

void TestBooleanOr::testParenthesisMust() 
{
  shared_ptr<BooleanQuery::Builder> q3 = make_shared<BooleanQuery::Builder>();
  q3->add(make_shared<BooleanClause>(t1, BooleanClause::Occur::SHOULD));
  q3->add(make_shared<BooleanClause>(t2, BooleanClause::Occur::SHOULD));
  shared_ptr<BooleanQuery::Builder> q4 = make_shared<BooleanQuery::Builder>();
  q4->add(make_shared<BooleanClause>(c1, BooleanClause::Occur::MUST));
  q4->add(make_shared<BooleanClause>(c2, BooleanClause::Occur::MUST));
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(q3->build(), BooleanClause::Occur::SHOULD);
  q2->add(q4->build(), BooleanClause::Occur::SHOULD);
  TestUtil::assertEquals(1, search(q2->build()));
}

void TestBooleanOr::testParenthesisMust2() 
{
  shared_ptr<BooleanQuery::Builder> q3 = make_shared<BooleanQuery::Builder>();
  q3->add(make_shared<BooleanClause>(t1, BooleanClause::Occur::SHOULD));
  q3->add(make_shared<BooleanClause>(t2, BooleanClause::Occur::SHOULD));
  shared_ptr<BooleanQuery::Builder> q4 = make_shared<BooleanQuery::Builder>();
  q4->add(make_shared<BooleanClause>(c1, BooleanClause::Occur::SHOULD));
  q4->add(make_shared<BooleanClause>(c2, BooleanClause::Occur::SHOULD));
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(q3->build(), BooleanClause::Occur::SHOULD);
  q2->add(q4->build(), BooleanClause::Occur::MUST);
  TestUtil::assertEquals(1, search(q2->build()));
}

void TestBooleanOr::testParenthesisShould() 
{
  shared_ptr<BooleanQuery::Builder> q3 = make_shared<BooleanQuery::Builder>();
  q3->add(make_shared<BooleanClause>(t1, BooleanClause::Occur::SHOULD));
  q3->add(make_shared<BooleanClause>(t2, BooleanClause::Occur::SHOULD));
  shared_ptr<BooleanQuery::Builder> q4 = make_shared<BooleanQuery::Builder>();
  q4->add(make_shared<BooleanClause>(c1, BooleanClause::Occur::SHOULD));
  q4->add(make_shared<BooleanClause>(c2, BooleanClause::Occur::SHOULD));
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(q3->build(), BooleanClause::Occur::SHOULD);
  q2->add(q4->build(), BooleanClause::Occur::SHOULD);
  TestUtil::assertEquals(1, search(q2->build()));
}

void TestBooleanOr::setUp() 
{
  LuceneTestCase::setUp();

  //
  dir = newDirectory();

  //
  shared_ptr<RandomIndexWriter> writer =
      make_shared<RandomIndexWriter>(random(), dir);

  //
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(newField(FIELD_T, L"Optimize not deleting all files",
                        TextField::TYPE_STORED));
  d->push_back(newField(
      FIELD_C, L"Deleted When I run an optimize in our production environment.",
      TextField::TYPE_STORED));

  //
  writer->addDocument(d);

  reader = writer->getReader();
  //
  searcher = newSearcher(reader);
  delete writer;
}

void TestBooleanOr::tearDown() 
{
  delete reader;
  delete dir;
  LuceneTestCase::tearDown();
}

void TestBooleanOr::testBooleanScorerMax() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> riw = make_shared<RandomIndexWriter>(
      random(), dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));

  int docCount = atLeast(10000);

  for (int i = 0; i < docCount; i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newField(L"field", L"a", TextField::TYPE_NOT_STORED));
    riw->addDocument(doc);
  }

  riw->forceMerge(1);
  shared_ptr<IndexReader> r = riw->getReader();
  delete riw;

  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
          BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
          BooleanClause::Occur::SHOULD);

  shared_ptr<Weight> w = s->createWeight(s->rewrite(bq->build()), true, 1);

  TestUtil::assertEquals(1, s->getIndexReader()->leaves().size());
  shared_ptr<BulkScorer> scorer =
      w->bulkScorer(s->getIndexReader()->leaves()[0]);

  shared_ptr<FixedBitSet> *const hits = make_shared<FixedBitSet>(docCount);
  shared_ptr<AtomicInteger> *const end = make_shared<AtomicInteger>();
  shared_ptr<LeafCollector> c = make_shared<SimpleCollectorAnonymousInnerClass>(
      shared_from_this(), hits, end);

  while (end->intValue() < docCount) {
    constexpr int min = end->intValue();
    constexpr int inc = TestUtil::nextInt(random(), 1, 1000);
    constexpr int max = end->addAndGet(inc);
    scorer->score(c, nullptr, min, max);
  }

  TestUtil::assertEquals(docCount, hits->cardinality());
  delete r;
  delete dir;
}

TestBooleanOr::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<TestBooleanOr> outerInstance,
                                       shared_ptr<FixedBitSet> hits,
                                       shared_ptr<AtomicInteger> end)
{
  this->outerInstance = outerInstance;
  this->hits = hits;
  this->end = end;
}

void TestBooleanOr::SimpleCollectorAnonymousInnerClass::collect(int doc)
{
  assertTrue(L"collected doc=" + to_wstring(doc) + L" beyond max=" + end,
             doc < end->intValue());
  hits->set(doc);
}

bool TestBooleanOr::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return false;
}

shared_ptr<BulkScorer> TestBooleanOr::scorer(deque<int> &matches)
{
  return make_shared<BulkScorerAnonymousInnerClass>();
}

TestBooleanOr::BulkScorerAnonymousInnerClass::BulkScorerAnonymousInnerClass() {}

int TestBooleanOr::BulkScorerAnonymousInnerClass::score(
    shared_ptr<LeafCollector> collector, shared_ptr<Bits> acceptDocs, int min,
    int max) 
{
  collector->setScorer(scorer);
  while (i < matches->length && matches[i] < min) {
    i += 1;
  }
  while (i < matches->length && matches[i] < max) {
    scorer->doc = matches[i];
    if (acceptDocs == nullptr || acceptDocs->get(scorer::doc)) {
      collector->collect(scorer::doc);
    }
    i += 1;
  }
  if (i == matches->length) {
    return DocIdSetIterator::NO_MORE_DOCS;
  }
  return RandomNumbers::randomIntBetween(LuceneTestCase::random(), max,
                                         matches[i]);
}

int64_t TestBooleanOr::BulkScorerAnonymousInnerClass::cost()
{
  return matches->length;
}

void TestBooleanOr::testSubScorerNextIsNotMatch() 
{
  const deque<std::shared_ptr<BulkScorer>> optionalScorers = Arrays::asList(
      scorer({100000, 1000001, 9999999}), scorer({4000, 1000051}),
      scorer({5000, 100000, 9999998, 9999999}));
  Collections::shuffle(optionalScorers, random());
  shared_ptr<BooleanScorer> scorer = make_shared<BooleanScorer>(
      nullptr, optionalScorers, 1, random()->nextBoolean());
  const deque<int> matches = deque<int>();
  scorer->score(make_shared<LeafCollectorAnonymousInnerClass>(
                    shared_from_this(), scorer, matches),
                nullptr);
  TestUtil::assertEquals(
      Arrays::asList(4000, 5000, 100000, 1000001, 1000051, 9999998, 9999999),
      matches);
}

TestBooleanOr::LeafCollectorAnonymousInnerClass::
    LeafCollectorAnonymousInnerClass(
        shared_ptr<TestBooleanOr> outerInstance,
        shared_ptr<org::apache::lucene::search::BooleanScorer> scorer,
        deque<int> &matches)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
  this->matches = matches;
}

void TestBooleanOr::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void TestBooleanOr::LeafCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  matches.push_back(doc);
}
} // namespace org::apache::lucene::search