using namespace std;

#include "TestBooleanMinShouldMatch.h"

namespace org::apache::lucene::search
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<org::apache::lucene::store::Directory>
    TestBooleanMinShouldMatch::index;
shared_ptr<org::apache::lucene::index::IndexReader>
    TestBooleanMinShouldMatch::r;
shared_ptr<IndexSearcher> TestBooleanMinShouldMatch::s;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestBooleanMinShouldMatch::beforeClass() 
{
  std::deque<wstring> data = {L"A 1 2 3 4 5 6", L"Z       4 5 6", L"",
                               L"B   2   4 5 6", L"Y     3   5 6", L"",
                               L"C     3     6", L"X       4 5 6"};

  index = newDirectory();
  shared_ptr<RandomIndexWriter> w =
      make_shared<RandomIndexWriter>(random(), index);

  for (int i = 0; i < data.size(); i++) {
    shared_ptr<Document> doc = make_shared<Document>();
    doc->push_back(newStringField(
        L"id", to_wstring(i),
        Field::Store::YES)); // Field.Keyword("id",std::wstring.valueOf(i)));
    doc->push_back(newStringField(
        L"all", L"all", Field::Store::YES)); // Field.Keyword("all","all"));
    if (L"" != data[i]) {
      doc->push_back(newTextField(
          L"data", data[i], Field::Store::YES)); // Field.Text("data",data[i]));
    }
    w->addDocument(doc);
  }

  r = w->getReader();
  s = newSearcher(r);
  delete w;
  // System.out.println("Set up " + getName());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass() throws Exception
void TestBooleanMinShouldMatch::afterClass() 
{
  s.reset();
  delete r;
  r.reset();
  delete index;
  index.reset();
}

void TestBooleanMinShouldMatch::verifyNrHits(shared_ptr<Query> q,
                                             int expected) 
{
  // bs1
  std::deque<std::shared_ptr<ScoreDoc>> h = s->search(q, 1000)->scoreDocs;
  if (expected != h.size()) {
    printHits(getTestName(), h, s);
  }
  assertEquals(L"result count", expected, h.size());
  // System.out.println("TEST: now check");
  // bs2
  shared_ptr<TopScoreDocCollector> collector =
      TopScoreDocCollector::create(1000);
  s->search(q, collector);
  std::deque<std::shared_ptr<ScoreDoc>> h2 = collector->topDocs()->scoreDocs;
  if (expected != h2.size()) {
    printHits(getTestName(), h2, s);
  }
  assertEquals(L"result count (bs2)", expected, h2.size());

  QueryUtils::check(random(), q, s);
}

void TestBooleanMinShouldMatch::testAllOptional() 
{

  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  for (int i = 1; i <= 4; i++) {
    q->add(
        make_shared<TermQuery>(make_shared<Term>(L"data", L"" + to_wstring(i))),
        BooleanClause::Occur::SHOULD); // false, false);
  }
  q->setMinimumNumberShouldMatch(2); // match at least two of 4
  verifyNrHits(q->build(), 2);
}

void TestBooleanMinShouldMatch::testOneReqAndSomeOptional() 
{

  /* one required, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

  verifyNrHits(q->build(), 5);
}

void TestBooleanMinShouldMatch::testSomeReqAndSomeOptional() throw(
    runtime_error)
{

  /* two required, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

  verifyNrHits(q->build(), 5);
}

void TestBooleanMinShouldMatch::testOneProhibAndSomeOptional() throw(
    runtime_error)
{

  /* one prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testSomeProhibAndSomeOptional() throw(
    runtime_error)
{

  /* two prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"C")),
         BooleanClause::Occur::MUST_NOT); // false, true );

  q->setMinimumNumberShouldMatch(2); // 2 of 3 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testOneReqOneProhibAndSomeOptional() throw(
    runtime_error)
{

  /* one required, one prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testSomeReqOneProhibAndSomeOptional() throw(
    runtime_error)
{

  /* two required, one prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testOneReqSomeProhibAndSomeOptional() throw(
    runtime_error)
{

  /* one required, two prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"C")),
         BooleanClause::Occur::MUST_NOT); // false, true );

  q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testSomeReqSomeProhibAndSomeOptional() throw(
    runtime_error)
{

  /* two required, two prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"C")),
         BooleanClause::Occur::MUST_NOT); // false, true );

  q->setMinimumNumberShouldMatch(3); // 3 of 4 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testMinHigherThenNumOptional() throw(
    runtime_error)
{

  /* two required, two prohibited, some optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"5")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"4")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST_NOT); // false, true );
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"C")),
         BooleanClause::Occur::MUST_NOT); // false, true );

  q->setMinimumNumberShouldMatch(90); // 90 of 4 optional ?!?!?!

  verifyNrHits(q->build(), 0);
}

void TestBooleanMinShouldMatch::testMinEqualToNumOptional() 
{

  /* two required, two optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"6")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::SHOULD); // false, false);

  q->setMinimumNumberShouldMatch(2); // 2 of 2 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testOneOptionalEqualToMin() 
{

  /* two required, one optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"3")),
         BooleanClause::Occur::SHOULD); // false, false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::MUST); // true,  false);

  q->setMinimumNumberShouldMatch(1); // 1 of 1 optional

  verifyNrHits(q->build(), 1);
}

void TestBooleanMinShouldMatch::testNoOptionalButMin() 
{

  /* two required, no optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);
  q->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"2")),
         BooleanClause::Occur::MUST); // true,  false);

  q->setMinimumNumberShouldMatch(1); // 1 of 0 optional

  verifyNrHits(q->build(), 0);
}

void TestBooleanMinShouldMatch::testNoOptionalButMin2() 
{

  /* one required, no optional */
  shared_ptr<BooleanQuery::Builder> q = make_shared<BooleanQuery::Builder>();
  q->add(make_shared<TermQuery>(make_shared<Term>(L"all", L"all")),
         BooleanClause::Occur::MUST); // true,  false);

  q->setMinimumNumberShouldMatch(1); // 1 of 0 optional

  verifyNrHits(q->build(), 0);
}

void TestBooleanMinShouldMatch::testRandomQueries() 
{
  const wstring field = L"data";
  const std::deque<wstring> vals = {L"1", L"2", L"3", L"4", L"5", L"6",  L"A",
                                     L"Z", L"B", L"Y", L"Z", L"X", L"foo"};
  int maxLev = 4;

  // callback object to set a random setMinimumNumberShouldMatch
  shared_ptr<TestBoolean2::Callback> minNrCB =
      make_shared<CallbackAnonymousInnerClass>(shared_from_this(), field, vals);

  // increase number of iterations for more complete testing
  int num = atLeast(20);
  for (int i = 0; i < num; i++) {
    int lev = random()->nextInt(maxLev);
    constexpr int64_t seed = random()->nextLong();
    shared_ptr<BooleanQuery::Builder> q1 = TestBoolean2::randBoolQuery(
        make_shared<Random>(seed), true, lev, field, vals, nullptr);
    // BooleanQuery q2 = TestBoolean2.randBoolQuery(new Random(seed), lev,
    // field, vals, minNrCB);
    shared_ptr<BooleanQuery::Builder> q2 = TestBoolean2::randBoolQuery(
        make_shared<Random>(seed), true, lev, field, vals, nullptr);
    // only set minimumNumberShouldMatch on the top level query since setting
    // at a lower level can change the score.
    minNrCB->postCreate(q2);

    // Can't use Hits because normalized scores will mess things
    // up.  The non-sorting version of search() that returns TopDocs
    // will not normalize scores.
    shared_ptr<TopDocs> top1 = s->search(q1->build(), 100);
    shared_ptr<TopDocs> top2 = s->search(q2->build(), 100);
    if (i < 100) {
      QueryUtils::check(random(), q1->build(), s);
      QueryUtils::check(random(), q2->build(), s);
    }
    assertSubsetOfSameScores(q2->build(), top1, top2);
  }
  // System.out.println("Total hits:"+tot);
}

TestBooleanMinShouldMatch::CallbackAnonymousInnerClass::
    CallbackAnonymousInnerClass(
        shared_ptr<TestBooleanMinShouldMatch> outerInstance,
        const wstring &field, deque<wstring> &vals)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->vals = vals;
}

void TestBooleanMinShouldMatch::CallbackAnonymousInnerClass::postCreate(
    shared_ptr<BooleanQuery::Builder> q)
{
  int opt = 0;
  for (auto clause : q->build()->clauses()) {
    if (clause->getOccur() == BooleanClause::Occur::SHOULD) {
      opt++;
    }
  }
  q->setMinimumNumberShouldMatch(LuceneTestCase::random()->nextInt(opt + 2));
  if (LuceneTestCase::random()->nextBoolean()) {
    // also add a random negation
    shared_ptr<Term> randomTerm = make_shared<Term>(
        field, vals[LuceneTestCase::random()->nextInt(vals.size())]);
    q->add(make_shared<TermQuery>(randomTerm), BooleanClause::Occur::MUST_NOT);
  }
}

void TestBooleanMinShouldMatch::assertSubsetOfSameScores(
    shared_ptr<Query> q, shared_ptr<TopDocs> top1, shared_ptr<TopDocs> top2)
{
  // The constrained query
  // should be a subset to the unconstrained query.
  if (top2->totalHits > top1->totalHits) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    fail(L"Constrained results not a subset:\n" +
         CheckHits::topdocsString(top1, 0, 0) +
         CheckHits::topdocsString(top2, 0, 0) + L"for query:" + q->toString());
  }

  for (int hit = 0; hit < top2->totalHits; hit++) {
    int id = top2->scoreDocs[hit]->doc;
    float score = top2->scoreDocs[hit]->score;
    bool found = false;
    // find this doc in other hits
    for (int other = 0; other < top1->totalHits; other++) {
      if (top1->scoreDocs[other]->doc == id) {
        found = true;
        float otherScore = top1->scoreDocs[other]->score;
        // check if scores match
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertEquals(L"Doc " + to_wstring(id) + L" scores don't match\n" +
                         CheckHits::topdocsString(top1, 0, 0) +
                         CheckHits::topdocsString(top2, 0, 0) + L"for query:" +
                         q->toString(),
                     score, otherScore,
                     CheckHits::explainToleranceDelta(score, otherScore));
      }
    }

    // check if subset
    if (!found) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      fail(L"Doc " + to_wstring(id) + L" not found\n" +
           CheckHits::topdocsString(top1, 0, 0) +
           CheckHits::topdocsString(top2, 0, 0) + L"for query:" +
           q->toString());
    }
  }
}

void TestBooleanMinShouldMatch::testRewriteMSM1() 
{
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
          BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
          BooleanClause::Occur::SHOULD);
  q2->setMinimumNumberShouldMatch(1);
  shared_ptr<TopDocs> top1 = s->search(q1->build(), 100);
  shared_ptr<TopDocs> top2 = s->search(q2->build(), 100);
  assertSubsetOfSameScores(q2->build(), top1, top2);
}

void TestBooleanMinShouldMatch::testRewriteNegate() 
{
  shared_ptr<BooleanQuery::Builder> q1 = make_shared<BooleanQuery::Builder>();
  q1->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
          BooleanClause::Occur::SHOULD);
  shared_ptr<BooleanQuery::Builder> q2 = make_shared<BooleanQuery::Builder>();
  q2->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"1")),
          BooleanClause::Occur::SHOULD);
  q2->add(make_shared<TermQuery>(make_shared<Term>(L"data", L"Z")),
          BooleanClause::Occur::MUST_NOT);
  shared_ptr<TopDocs> top1 = s->search(q1->build(), 100);
  shared_ptr<TopDocs> top2 = s->search(q2->build(), 100);
  assertSubsetOfSameScores(q2->build(), top1, top2);
}

void TestBooleanMinShouldMatch::printHits(
    const wstring &test, std::deque<std::shared_ptr<ScoreDoc>> &h,
    shared_ptr<IndexSearcher> searcher) 
{

  System::err::println(L"------- " + test + L" -------");

  shared_ptr<DecimalFormat> f = make_shared<DecimalFormat>(
      L"0.000000", DecimalFormatSymbols::getInstance(Locale::ROOT));

  for (int i = 0; i < h.size(); i++) {
    shared_ptr<Document> d = searcher->doc(h[i]->doc);
    float score = h[i]->score;
    System::err::println(L"#" + to_wstring(i) + L": " + f->format(score) +
                         L" - " + d[L"id"] + L" - " + d[L"data"]);
  }
}
} // namespace org::apache::lucene::search