using namespace std;

#include "TestBooleanQuery.h"

namespace org::apache::lucene::search
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiReader = org::apache::lucene::index::MultiReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Occur = org::apache::lucene::search::BooleanClause::Occur;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NamedThreadFactory = org::apache::lucene::util::NamedThreadFactory;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;

void TestBooleanQuery::testEquality() 
{
  shared_ptr<BooleanQuery::Builder> bq1 = make_shared<BooleanQuery::Builder>();
  bq1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"value1")),
           BooleanClause::Occur::SHOULD);
  bq1->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"value2")),
           BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> nested1 =
      make_shared<BooleanQuery::Builder>();
  nested1->add(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"nestedvalue1")),
      BooleanClause::Occur::SHOULD);
  nested1->add(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"nestedvalue2")),
      BooleanClause::Occur::SHOULD);
  bq1->add(nested1->build(), BooleanClause::Occur::SHOULD);

  shared_ptr<BooleanQuery::Builder> bq2 = make_shared<BooleanQuery::Builder>();
  bq2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"value1")),
           BooleanClause::Occur::SHOULD);
  bq2->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"value2")),
           BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> nested2 =
      make_shared<BooleanQuery::Builder>();
  nested2->add(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"nestedvalue1")),
      BooleanClause::Occur::SHOULD);
  nested2->add(
      make_shared<TermQuery>(make_shared<Term>(L"field", L"nestedvalue2")),
      BooleanClause::Occur::SHOULD);
  bq2->add(nested2->build(), BooleanClause::Occur::SHOULD);

  TestUtil::assertEquals(bq1->build(), bq2->build());
}

void TestBooleanQuery::testEqualityDoesNotDependOnOrder()
{
  std::deque<std::shared_ptr<TermQuery>> queries = {
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
      make_shared<TermQuery>(make_shared<Term>(L"foo", L"baz"))};
  for (int iter = 0; iter < 10; ++iter) {
    deque<std::shared_ptr<BooleanClause>> clauses =
        deque<std::shared_ptr<BooleanClause>>();
    constexpr int numClauses = random()->nextInt(20);
    for (int i = 0; i < numClauses; ++i) {
      shared_ptr<Query> query = RandomPicks::randomFrom(random(), queries);
      if (random()->nextBoolean()) {
        query = make_shared<BoostQuery>(query, random()->nextFloat());
      }
      Occur occur = RandomPicks::randomFrom(random(), Occur::values());
      clauses.push_back(make_shared<BooleanClause>(query, occur));
    }

    constexpr int minShouldMatch = random()->nextInt(5);
    shared_ptr<BooleanQuery::Builder> bq1Builder =
        make_shared<BooleanQuery::Builder>();
    bq1Builder->setMinimumNumberShouldMatch(minShouldMatch);
    for (auto clause : clauses) {
      bq1Builder->add(clause);
    }
    shared_ptr<BooleanQuery> *const bq1 = bq1Builder->build();

    Collections::shuffle(clauses, random());
    shared_ptr<BooleanQuery::Builder> bq2Builder =
        make_shared<BooleanQuery::Builder>();
    bq2Builder->setMinimumNumberShouldMatch(minShouldMatch);
    for (auto clause : clauses) {
      bq2Builder->add(clause);
    }
    shared_ptr<BooleanQuery> *const bq2 = bq2Builder->build();

    QueryUtils::checkEqual(bq1, bq2);
  }
}

void TestBooleanQuery::testEqualityOnDuplicateShouldClauses()
{
  shared_ptr<BooleanQuery> bq1 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(random()->nextInt(2))
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->build();
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(bq1->getMinimumNumberShouldMatch())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::SHOULD)
          ->build();
  QueryUtils::checkUnequal(bq1, bq2);
}

void TestBooleanQuery::testEqualityOnDuplicateMustClauses()
{
  shared_ptr<BooleanQuery> bq1 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(random()->nextInt(2))
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->build();
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(bq1->getMinimumNumberShouldMatch())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::MUST)
          ->build();
  QueryUtils::checkUnequal(bq1, bq2);
}

void TestBooleanQuery::testEqualityOnDuplicateFilterClauses()
{
  shared_ptr<BooleanQuery> bq1 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(random()->nextInt(2))
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(bq1->getMinimumNumberShouldMatch())
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  QueryUtils::checkEqual(bq1, bq2);
}

void TestBooleanQuery::testEqualityOnDuplicateMustNotClauses()
{
  shared_ptr<BooleanQuery> bq1 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(random()->nextInt(2))
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  shared_ptr<BooleanQuery> bq2 =
      (make_shared<BooleanQuery::Builder>())
          ->setMinimumNumberShouldMatch(bq1->getMinimumNumberShouldMatch())
          ->add(make_shared<MatchAllDocsQuery>(), Occur::MUST)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->add(make_shared<TermQuery>(make_shared<Term>(L"foo", L"bar")),
                Occur::FILTER)
          ->build();
  QueryUtils::checkEqual(bq1, bq2);
}

void TestBooleanQuery::testHashCodeIsStable()
{
  shared_ptr<BooleanQuery> bq =
      (make_shared<BooleanQuery::Builder>())
          ->add(make_shared<TermQuery>(make_shared<Term>(
                    L"foo", TestUtil::randomSimpleString(random()))),
                Occur::SHOULD)
          ->add(make_shared<TermQuery>(make_shared<Term>(
                    L"foo", TestUtil::randomSimpleString(random()))),
                Occur::SHOULD)
          ->build();
  constexpr int hashCode = bq->hashCode();
  TestUtil::assertEquals(hashCode, bq->hashCode());
}

void TestBooleanQuery::testException()
{
  expectThrows(invalid_argument::typeid,
               [&]() { BooleanQuery::setMaxClauseCount(0); });
}

void TestBooleanQuery::testNullOrSubScorer() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c d", Field::Store::NO));
  w->addDocument(doc);

  shared_ptr<IndexReader> r = w->getReader();
  shared_ptr<IndexSearcher> s = newSearcher(r);
  // this test relies upon coord being the default implementation,
  // otherwise scores are different!
  s->setSimilarity(make_shared<ClassicSimilarity>());

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
         BooleanClause::Occur::SHOULD);

  // PhraseQuery w/ no terms added returns a null scorer
  shared_ptr<PhraseQuery> pq =
      make_shared<PhraseQuery>(L"field", std::deque<wstring>(0));
  q->add(pq, BooleanClause::Occur::SHOULD);
  TestUtil::assertEquals(1, s->search(q->build(), 10)->totalHits);

  // A required clause which returns null scorer should return null scorer to
  // IndexSearcher.
  q = make_shared<BooleanQuery::Builder>();
  pq = make_shared<PhraseQuery>(L"field", std::deque<wstring>(0));
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
         BooleanClause::Occur::SHOULD);
  q->add(pq, BooleanClause::Occur::MUST);
  TestUtil::assertEquals(0, s->search(q->build(), 10)->totalHits);

  shared_ptr<DisjunctionMaxQuery> dmq = make_shared<DisjunctionMaxQuery>(
      Arrays::asList(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
                     pq),
      1.0f);
  TestUtil::assertEquals(1, s->search(dmq, 10)->totalHits);

  delete r;
  delete w;
  delete dir;
}

void TestBooleanQuery::testDeMorgan() 
{
  shared_ptr<Directory> dir1 = newDirectory();
  shared_ptr<RandomIndexWriter> iw1 =
      make_shared<RandomIndexWriter>(random(), dir1);
  shared_ptr<Document> doc1 = make_shared<Document>();
  doc1->push_back(newTextField(L"field", L"foo bar", Field::Store::NO));
  iw1->addDocument(doc1);
  shared_ptr<IndexReader> reader1 = iw1->getReader();
  delete iw1;

  shared_ptr<Directory> dir2 = newDirectory();
  shared_ptr<RandomIndexWriter> iw2 =
      make_shared<RandomIndexWriter>(random(), dir2);
  shared_ptr<Document> doc2 = make_shared<Document>();
  doc2->push_back(newTextField(L"field", L"foo baz", Field::Store::NO));
  iw2->addDocument(doc2);
  shared_ptr<IndexReader> reader2 = iw2->getReader();
  delete iw2;

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>(); // Query: +foo -ba*
  query->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"foo")),
             BooleanClause::Occur::MUST);
  shared_ptr<WildcardQuery> wildcardQuery =
      make_shared<WildcardQuery>(make_shared<Term>(L"field", L"ba*"));
  wildcardQuery->setRewriteMethod(MultiTermQuery::SCORING_BOOLEAN_REWRITE);
  query->add(wildcardQuery, BooleanClause::Occur::MUST_NOT);

  shared_ptr<MultiReader> multireader =
      make_shared<MultiReader>(reader1, reader2);
  shared_ptr<IndexSearcher> searcher = newSearcher(multireader);
  TestUtil::assertEquals(0, searcher->search(query->build(), 10)->totalHits);

  shared_ptr<ExecutorService> *const es = Executors::newCachedThreadPool(
      make_shared<NamedThreadFactory>(L"NRT search threads"));
  searcher = make_shared<IndexSearcher>(multireader, es);
  if (VERBOSE) {
    wcout << L"rewritten form: " << searcher->rewrite(query->build()) << endl;
  }
  TestUtil::assertEquals(0, searcher->search(query->build(), 10)->totalHits);
  es->shutdown();
  es->awaitTermination(1, TimeUnit::SECONDS);

  multireader->close();
  delete reader1;
  delete reader2;
  delete dir1;
  delete dir2;
}

void TestBooleanQuery::testBS2DisjunctionNextVsAdvance() 
{
  shared_ptr<Directory> *const d = newDirectory();
  shared_ptr<RandomIndexWriter> *const w =
      make_shared<RandomIndexWriter>(random(), d);
  constexpr int numDocs = atLeast(300);
  for (int docUpto = 0; docUpto < numDocs; docUpto++) {
    wstring contents = L"a";
    if (random()->nextInt(20) <= 16) {
      contents += L" b";
    }
    if (random()->nextInt(20) <= 8) {
      contents += L" c";
    }
    if (random()->nextInt(20) <= 4) {
      contents += L" d";
    }
    if (random()->nextInt(20) <= 2) {
      contents += L" e";
    }
    if (random()->nextInt(20) <= 1) {
      contents += L" f";
    }
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(
        make_shared<TextField>(L"field", contents, Field::Store::NO));
    w->addDocument(doc);
  }
  w->forceMerge(1);
  shared_ptr<IndexReader> *const r = w->getReader();
  shared_ptr<IndexSearcher> *const s = newSearcher(r);
  delete w;

  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {
    if (VERBOSE) {
      wcout << L"iter=" << iter << endl;
    }
    const deque<wstring> terms =
        deque<wstring>(Arrays::asList(L"a", L"b", L"c", L"d", L"e", L"f"));
    constexpr int numTerms = TestUtil::nextInt(random(), 1, terms.size());
    while (terms.size() > numTerms) {
      // C++ TODO: The Java deque 'remove(Object)' method is not converted:
      terms.remove(random()->nextInt(terms.size()));
    }

    if (VERBOSE) {
      wcout << L"  terms=" << terms << endl;
    }

    shared_ptr<BooleanQuery::Builder> *const q =
        make_shared<BooleanQuery::Builder>();
    for (auto term : terms) {
      q->add(make_shared<BooleanClause>(
          make_shared<TermQuery>(make_shared<Term>(L"field", term)),
          BooleanClause::Occur::SHOULD));
    }

    shared_ptr<Weight> weight =
        s->createWeight(s->rewrite(q->build()), true, 1);

    shared_ptr<Scorer> scorer = weight->scorer(s->leafContexts[0]);

    // First pass: just use .nextDoc() to gather all hits
    const deque<std::shared_ptr<ScoreDoc>> hits =
        deque<std::shared_ptr<ScoreDoc>>();
    while (scorer->begin().nextDoc() != DocIdSetIterator::NO_MORE_DOCS) {
      hits.push_back(make_shared<ScoreDoc>(scorer->docID(), scorer->score()));
    }

    if (VERBOSE) {
      wcout << L"  " << hits.size() << L" hits" << endl;
    }

    // Now, randomly next/advance through the deque and
    // verify exact match:
    for (int iter2 = 0; iter2 < 10; iter2++) {

      weight = s->createWeight(s->rewrite(q->build()), true, 1);
      scorer = weight->scorer(s->leafContexts[0]);

      if (VERBOSE) {
        wcout << L"  iter2=" << iter2 << endl;
      }

      int upto = -1;
      while (upto < hits.size()) {
        constexpr int nextUpto;
        constexpr int nextDoc;
        constexpr int left = hits.size() - upto;
        if (left == 1 || random()->nextBoolean()) {
          // next
          nextUpto = 1 + upto;
          nextDoc = scorer->begin().nextDoc();
        } else {
          // advance
          int inc = TestUtil::nextInt(random(), 1, left - 1);
          nextUpto = inc + upto;
          nextDoc = scorer->begin().advance(hits[nextUpto]->doc);
        }

        if (nextUpto == hits.size()) {
          TestUtil::assertEquals(DocIdSetIterator::NO_MORE_DOCS, nextDoc);
        } else {
          shared_ptr<ScoreDoc> *const hit = hits[nextUpto];
          TestUtil::assertEquals(hit->doc, nextDoc);
          // Test for precise float equality:
          assertTrue(L"doc " + to_wstring(hit->doc) +
                         L" has wrong score: expected=" +
                         to_wstring(hit->score) + L" actual=" +
                         to_wstring(scorer->score()),
                     hit->score == scorer->score());
        }
        upto = nextUpto;
      }
    }
  }

  delete r;
  delete d;
}

void TestBooleanQuery::testBooleanSpanQuery() 
{
  bool failed = false;
  int hits = 0;
  shared_ptr<Directory> directory = newDirectory();
  shared_ptr<Analyzer> indexerAnalyzer = make_shared<MockAnalyzer>(random());

  shared_ptr<IndexWriterConfig> config =
      make_shared<IndexWriterConfig>(indexerAnalyzer);
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(directory, config);
  wstring FIELD = L"content";
  shared_ptr<Document> d = make_shared<Document>();
  d->push_back(
      make_shared<TextField>(FIELD, L"clockwork orange", Field::Store::YES));
  writer->addDocument(d);
  delete writer;

  shared_ptr<IndexReader> indexReader = DirectoryReader::open(directory);
  shared_ptr<IndexSearcher> searcher = newSearcher(indexReader);

  shared_ptr<BooleanQuery::Builder> query =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<SpanQuery> sq1 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"clockwork"));
  shared_ptr<SpanQuery> sq2 =
      make_shared<SpanTermQuery>(make_shared<Term>(FIELD, L"clckwork"));
  query->add(sq1, BooleanClause::Occur::SHOULD);
  query->add(sq2, BooleanClause::Occur::SHOULD);
  shared_ptr<TopScoreDocCollector> collector =
      TopScoreDocCollector::create(1000);
  searcher->search(query->build(), collector);
  hits = collector->topDocs()->scoreDocs.size();
  for (auto scoreDoc : collector->topDocs()->scoreDocs) {
    wcout << scoreDoc->doc << endl;
  }
  delete indexReader;
  assertEquals(L"Bug in bool query composed of span queries", failed, false);
  assertEquals(L"Bug in bool query composed of span queries", hits, 1);
  delete directory;
}

void TestBooleanQuery::testMinShouldMatchLeniency() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<IndexWriter> w = make_shared<IndexWriter>(
      dir, newIndexWriterConfig(make_shared<MockAnalyzer>(random())));
  shared_ptr<Document> doc = make_shared<Document>();
  doc->push_back(newTextField(L"field", L"a b c d", Field::Store::NO));
  w->addDocument(doc);
  shared_ptr<IndexReader> r = DirectoryReader::open(w);
  shared_ptr<IndexSearcher> s = newSearcher(r);
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
          BooleanClause::Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
          BooleanClause::Occur::SHOULD);

  // No doc can match: BQ has only 2 clauses and we are asking for
  // minShouldMatch=4
  bq->setMinimumNumberShouldMatch(4);
  TestUtil::assertEquals(0, s->search(bq->build(), 1)->totalHits);
  delete r;
  delete w;
  delete dir;
}

shared_ptr<BitSet>
TestBooleanQuery::getMatches(shared_ptr<IndexSearcher> searcher,
                             shared_ptr<Query> query) 
{
  shared_ptr<BitSet> set = make_shared<BitSet>();
  searcher->search(query, make_shared<SimpleCollectorAnonymousInnerClass>(set));
  return set;
}

TestBooleanQuery::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(shared_ptr<BitSet> set)
{
  this->set = set;
}

bool TestBooleanQuery::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return LuceneTestCase::random()->nextBoolean();
}

void TestBooleanQuery::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  outerInstance->super.doSetNextReader(context);
  docBase = context->docBase;
}

void TestBooleanQuery::SimpleCollectorAnonymousInnerClass::collect(
    int doc) 
{
  set->set(docBase + doc);
}

void TestBooleanQuery::testFILTERClauseBehavesLikeMUST() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c d", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  f->setStringValue(L"b d");
  w->addDocument(doc);
  f->setStringValue(L"d");
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);

  for (shared_ptr<deque<wstring>> requiredTerms :
       Arrays::asList<deque<wstring>>(
           Arrays::asList(L"a", L"d"), Arrays::asList(L"a", L"b", L"d"),
           Arrays::asList(L"d"), Arrays::asList(L"e"), Arrays::asList())) {
    shared_ptr<BooleanQuery::Builder> *const bq1 =
        make_shared<BooleanQuery::Builder>();
    shared_ptr<BooleanQuery::Builder> *const bq2 =
        make_shared<BooleanQuery::Builder>();
    for (auto term : requiredTerms) {
      shared_ptr<Query> *const q =
          make_shared<TermQuery>(make_shared<Term>(L"field", term));
      bq1->add(q, Occur::MUST);
      bq2->add(q, Occur::FILTER);
    }

    shared_ptr<BitSet> *const matches1 = getMatches(searcher, bq1->build());
    shared_ptr<BitSet> *const matches2 = getMatches(searcher, bq2->build());
    TestUtil::assertEquals(matches1, matches2);
  }

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::assertSameScoresWithoutFilters(
    shared_ptr<IndexSearcher> searcher,
    shared_ptr<BooleanQuery> bq) 
{
  shared_ptr<BooleanQuery::Builder> *const bq2Builder =
      make_shared<BooleanQuery::Builder>();
  for (auto c : bq) {
    if (c->getOccur() != Occur::FILTER) {
      bq2Builder->add(c);
    }
  }
  bq2Builder->setMinimumNumberShouldMatch(bq->getMinimumNumberShouldMatch());
  shared_ptr<BooleanQuery> bq2 = bq2Builder->build();

  shared_ptr<AtomicBoolean> *const matched = make_shared<AtomicBoolean>();
  searcher->search(bq, make_shared<SimpleCollectorAnonymousInnerClass2>(
                           shared_from_this(), searcher, bq2, matched));
  assertTrue(matched->get());
}

TestBooleanQuery::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<TestBooleanQuery> outerInstance,
        shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        shared_ptr<org::apache::lucene::search::BooleanQuery> bq2,
        shared_ptr<AtomicBoolean> matched)
{
  this->outerInstance = outerInstance;
  this->searcher = searcher;
  this->bq2 = bq2;
  this->matched = matched;
}

void TestBooleanQuery::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  outerInstance->super->doSetNextReader(context);
  docBase = context->docBase;
}

bool TestBooleanQuery::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}

void TestBooleanQuery::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer = scorer;
}

void TestBooleanQuery::SimpleCollectorAnonymousInnerClass2::collect(
    int doc) 
{
  constexpr float actualScore = scorer::score();
  constexpr float expectedScore =
      searcher->explain(bq2, docBase + doc)->getValue();
  assertEquals(expectedScore, actualScore, 10e-5);
  matched->set(true);
}

void TestBooleanQuery::testFilterClauseDoesNotImpactScore() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c d", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  f->setStringValue(L"b d");
  w->addDocument(doc);
  f->setStringValue(L"a d");
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);

  shared_ptr<BooleanQuery::Builder> qBuilder =
      make_shared<BooleanQuery::Builder>();
  shared_ptr<BooleanQuery> q = qBuilder->build();
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
                Occur::FILTER);

  // With a single clause, we will rewrite to the underlying
  // query. Make sure that it returns null scores
  assertSameScoresWithoutFilters(searcher, qBuilder->build());

  // Now with two clauses, we will get a conjunction scorer
  // Make sure it returns null scores
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
                Occur::FILTER);
  q = qBuilder->build();
  assertSameScoresWithoutFilters(searcher, q);

  // Now with a scoring clause, we need to make sure that
  // the bool scores are the same as those from the term
  // query
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
                Occur::SHOULD);
  q = qBuilder->build();
  assertSameScoresWithoutFilters(searcher, q);

  // FILTER and empty SHOULD
  qBuilder = make_shared<BooleanQuery::Builder>();
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
                Occur::FILTER);
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"e")),
                Occur::SHOULD);
  q = qBuilder->build();
  assertSameScoresWithoutFilters(searcher, q);

  // mix of FILTER and MUST
  qBuilder = make_shared<BooleanQuery::Builder>();
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
                Occur::FILTER);
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"d")),
                Occur::MUST);
  q = qBuilder->build();
  assertSameScoresWithoutFilters(searcher, q);

  // FILTER + minShouldMatch
  qBuilder = make_shared<BooleanQuery::Builder>();
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
                Occur::FILTER);
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
                Occur::SHOULD);
  qBuilder->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"d")),
                Occur::SHOULD);
  qBuilder->setMinimumNumberShouldMatch(1);
  q = qBuilder->build();
  assertSameScoresWithoutFilters(searcher, q);

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testConjunctionPropagatesApproximations() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  // not LuceneTestCase.newSearcher to not have the asserting wrappers
  // and do instanceof checks
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(pq, Occur::MUST);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
         Occur::FILTER);

  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(q->build()), true, 1);
  shared_ptr<Scorer> *const scorer =
      weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertTrue(std::dynamic_pointer_cast<ConjunctionScorer>(scorer) != nullptr);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testDisjunctionPropagatesApproximations() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(pq, Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
         Occur::SHOULD);

  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(q->build()), true, 1);
  shared_ptr<Scorer> *const scorer = weight->scorer(reader->leaves()->get(0));
  assertTrue(std::dynamic_pointer_cast<DisjunctionScorer>(scorer) != nullptr);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testBoostedScorerPropagatesApproximations() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  // not LuceneTestCase.newSearcher to not have the asserting wrappers
  // and do instanceof checks
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(pq, Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"d")),
         Occur::SHOULD);

  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(q->build()), true, 1);
  shared_ptr<Scorer> *const scorer =
      weight->scorer(searcher->getIndexReader()->leaves()[0]);
  assertTrue(std::dynamic_pointer_cast<PhraseScorer>(scorer) != nullptr);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testExclusionPropagatesApproximations() throw(
    IOException)
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(pq, Occur::SHOULD);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
         Occur::MUST_NOT);

  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(q->build()), true, 1);
  shared_ptr<Scorer> *const scorer = weight->scorer(reader->leaves()->get(0));
  assertTrue(std::dynamic_pointer_cast<ReqExclScorer>(scorer) != nullptr);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testReqOptPropagatesApproximations() 
{
  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), dir);
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<Field> f = newTextField(L"field", L"a b c", Field::Store::NO);
  doc->push_back(f);
  w->addDocument(doc);
  w->commit();

  shared_ptr<DirectoryReader> reader = w->getReader();
  shared_ptr<IndexSearcher> *const searcher =
      make_shared<IndexSearcher>(reader);
  searcher->setQueryCache(nullptr); // to still have approximations

  shared_ptr<PhraseQuery> pq = make_shared<PhraseQuery>(L"field", L"a", L"b");

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(pq, Occur::MUST);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
         Occur::SHOULD);

  shared_ptr<Weight> *const weight =
      searcher->createWeight(searcher->rewrite(q->build()), true, 1);
  shared_ptr<Scorer> *const scorer = weight->scorer(reader->leaves()->get(0));
  assertTrue(std::dynamic_pointer_cast<ReqOptSumScorer>(scorer) != nullptr);
  assertNotNull(scorer->twoPhaseIterator());

  reader->close();
  delete w;
  delete dir;
}

void TestBooleanQuery::testToString()
{
  shared_ptr<BooleanQuery::Builder> bq = make_shared<BooleanQuery::Builder>();
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"a")),
          Occur::SHOULD);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"b")),
          Occur::MUST);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"c")),
          Occur::MUST_NOT);
  bq->add(make_shared<TermQuery>(make_shared<Term>(L"field", L"d")),
          Occur::FILTER);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  TestUtil::assertEquals(L"a +b -c #d", bq->build()->toString(L"field"));
}

void TestBooleanQuery::testExtractTerms() 
{
  shared_ptr<Term> a = make_shared<Term>(L"f", L"a");
  shared_ptr<Term> b = make_shared<Term>(L"f", L"b");
  shared_ptr<Term> c = make_shared<Term>(L"f", L"c");
  shared_ptr<Term> d = make_shared<Term>(L"f", L"d");
  shared_ptr<BooleanQuery::Builder> bqBuilder =
      make_shared<BooleanQuery::Builder>();
  bqBuilder->add(make_shared<TermQuery>(a), Occur::SHOULD);
  bqBuilder->add(make_shared<TermQuery>(b), Occur::MUST);
  bqBuilder->add(make_shared<TermQuery>(c), Occur::FILTER);
  bqBuilder->add(make_shared<TermQuery>(d), Occur::MUST_NOT);
  shared_ptr<IndexSearcher> searcher =
      make_shared<IndexSearcher>(make_shared<MultiReader>());
  shared_ptr<BooleanQuery> bq = bqBuilder->build();

  shared_ptr<Set<std::shared_ptr<Term>>> scoringTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(searcher->rewrite(bq), true, 1)
      ->extractTerms(scoringTerms);
  TestUtil::assertEquals(unordered_set<>(Arrays::asList(a, b)), scoringTerms);

  shared_ptr<Set<std::shared_ptr<Term>>> matchingTerms =
      unordered_set<std::shared_ptr<Term>>();
  searcher->createWeight(searcher->rewrite(bq), false, 1)
      ->extractTerms(matchingTerms);
  TestUtil::assertEquals(unordered_set<>(Arrays::asList(a, b, c)),
                         matchingTerms);
}
} // namespace org::apache::lucene::search