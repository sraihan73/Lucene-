using namespace std;

#include "TestBaseExplanationTestCase.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using junit::framework::AssertionFailedError;

void TestBaseExplanationTestCase::testQueryNoMatchWhenExpected() throw(
    runtime_error)
{
  expectThrows(AssertionFailedError::typeid, [&]() {
    qtest(make_shared<TermQuery>(make_shared<Term>(FIELD, L"BOGUS")),
          std::deque<int>{3});
  });
}

void TestBaseExplanationTestCase::testQueryMatchWhenNotExpected() throw(
    runtime_error)
{
  expectThrows(AssertionFailedError::typeid, [&]() {
    qtest(make_shared<TermQuery>(make_shared<Term>(FIELD, L"w1")),
          std::deque<int>{0, 1});
  });
}

void TestBaseExplanationTestCase::testIncorrectExplainScores() throw(
    runtime_error)
{
  // sanity check what a real TermQuery matches
  qtest(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
        std::deque<int>{1, 3});

  // ensure when the Explanations are broken, we get an error about those
  // matches
  expectThrows(AssertionFailedError::typeid, [&]() {
    qtest(make_shared<BrokenExplainTermQuery>(make_shared<Term>(FIELD, L"zz"),
                                              false, true),
          std::deque<int>{1, 3});
  });
}

void TestBaseExplanationTestCase::testIncorrectExplainMatches() throw(
    runtime_error)
{
  // sanity check what a real TermQuery matches
  qtest(make_shared<TermQuery>(make_shared<Term>(FIELD, L"zz")),
        std::deque<int>{1, 3});

  // ensure when the Explanations are broken, we get an error about the non
  // matches
  expectThrows(AssertionFailedError::typeid, [&]() {
    CheckHits::checkNoMatchExplanations(
        make_shared<BrokenExplainTermQuery>(make_shared<Term>(FIELD, L"zz"),
                                            true, false),
        FIELD, searcher, std::deque<int>{1, 3});
  });
}

TestBaseExplanationTestCase::BrokenExplainTermQuery::BrokenExplainTermQuery(
    shared_ptr<Term> t, bool toggleExplainMatch, bool breakExplainScores)
    : TermQuery(t), toggleExplainMatch(toggleExplainMatch),
      breakExplainScores(breakExplainScores)
{
}

shared_ptr<Weight>
TestBaseExplanationTestCase::BrokenExplainTermQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  return make_shared<BrokenExplainWeight>(
      shared_from_this(),
      TermQuery::createWeight(searcher, needsScores, boost));
}

TestBaseExplanationTestCase::BrokenExplainWeight::BrokenExplainWeight(
    shared_ptr<BrokenExplainTermQuery> q, shared_ptr<Weight> in_)
    : FilterWeight(q, in_)
{
}

shared_ptr<BulkScorer>
TestBaseExplanationTestCase::BrokenExplainWeight::bulkScorer(
    shared_ptr<LeafReaderContext> context) 
{
  return in_->bulkScorer(context);
}

shared_ptr<Explanation>
TestBaseExplanationTestCase::BrokenExplainWeight::explain(
    shared_ptr<LeafReaderContext> context, int doc) 
{
  shared_ptr<BrokenExplainTermQuery> q =
      std::static_pointer_cast<BrokenExplainTermQuery>(this->getQuery());
  shared_ptr<Explanation> result = in_->explain(context, doc);
  if (result->isMatch()) {
    if (q->breakExplainScores) {
      result = Explanation::match(-1.0F * result->getValue(),
                                  L"Broken Explanation Score", result);
    }
    if (q->toggleExplainMatch) {
      result = Explanation::noMatch(L"Broken Explanation Matching", result);
    }
  } else {
    if (q->toggleExplainMatch) {
      result =
          Explanation::match(-42.0F, L"Broken Explanation Matching", result);
    }
  }
  return result;
}
} // namespace org::apache::lucene::search