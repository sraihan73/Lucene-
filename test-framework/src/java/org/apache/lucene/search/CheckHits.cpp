using namespace std;

#include "CheckHits.h"

namespace org::apache::lucene::search
{
using junit::framework::Assert;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
//    import static junit.framework.Assert.assertNotNull;
//    import static org.junit.Assert.assertNull;
float CheckHits::EXPLAIN_SCORE_TOLERANCE_DELTA = 0.001f;
float CheckHits::EXPLAIN_SCORE_TOLERANCE_MINIMUM = 1e-6.0f;

void CheckHits::checkNoMatchExplanations(
    shared_ptr<Query> q, const wstring &defaultFieldName,
    shared_ptr<IndexSearcher> searcher,
    std::deque<int> &results) 
{

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring d = q->toString(defaultFieldName);
  shared_ptr<Set<int>> ignore = set<int>();
  for (int i = 0; i < results.size(); i++) {
    ignore->add(static_cast<Integer>(results[i]));
  }

  int maxDoc = searcher->getIndexReader()->maxDoc();
  for (int doc = 0; doc < maxDoc; doc++) {
    if (ignore->contains(static_cast<Integer>(doc))) {
      continue;
    }

    shared_ptr<Explanation> exp = searcher->explain(q, doc);
    assertNotNull(L"Explanation of [[" + d + L"]] for #" + to_wstring(doc) +
                      L" is null",
                  exp);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    Assert::assertFalse(L"Explanation of [[" + d + L"]] for #" +
                            to_wstring(doc) + L" doesn't indicate non-match: " +
                            exp->toString(),
                        exp->isMatch());
  }
}

void CheckHits::checkHitCollector(shared_ptr<Random> random,
                                  shared_ptr<Query> query,
                                  const wstring &defaultFieldName,
                                  shared_ptr<IndexSearcher> searcher,
                                  std::deque<int> &results) 
{

  QueryUtils::check(random, query, searcher);

  shared_ptr<Set<int>> correct = set<int>();
  for (int i = 0; i < results.size(); i++) {
    correct->add(static_cast<Integer>(results[i]));
  }
  shared_ptr<Set<int>> *const actual = set<int>();
  shared_ptr<Collector> *const c = make_shared<SetCollector>(actual);

  searcher->search(query, c);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertEquals(L"Simple: " + query->toString(defaultFieldName), correct,
                       actual);

  for (int i = -1; i < 2; i++) {
    actual->clear();
    shared_ptr<IndexSearcher> s =
        QueryUtils::wrapUnderlyingReader(random, searcher, i);
    s->search(query, c);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    Assert::assertEquals(L"Wrap Reader " + to_wstring(i) + L": " +
                             query->toString(defaultFieldName),
                         correct, actual);
  }
}

CheckHits::SetCollector::SetCollector(shared_ptr<Set<int>> bag) : bag(bag) {}

void CheckHits::SetCollector::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
}

void CheckHits::SetCollector::collect(int doc)
{
  bag->add(static_cast<Integer>(doc + base));
}

void CheckHits::SetCollector::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  base = context->docBase;
}

bool CheckHits::SetCollector::needsScores() { return false; }

void CheckHits::checkHits(shared_ptr<Random> random, shared_ptr<Query> query,
                          const wstring &defaultFieldName,
                          shared_ptr<IndexSearcher> searcher,
                          std::deque<int> &results) 
{

  std::deque<std::shared_ptr<ScoreDoc>> hits =
      searcher->search(query, 1000)->scoreDocs;

  shared_ptr<Set<int>> correct = set<int>();
  for (int i = 0; i < results.size(); i++) {
    correct->add(static_cast<Integer>(results[i]));
  }

  shared_ptr<Set<int>> actual = set<int>();
  for (int i = 0; i < hits.size(); i++) {
    actual->add(static_cast<Integer>(hits[i]->doc));
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertEquals(query->toString(defaultFieldName), correct, actual);

  QueryUtils::check(random, query, searcher, LuceneTestCase::rarely(random));
}

void CheckHits::checkDocIds(const wstring &mes, std::deque<int> &results,
                            std::deque<std::shared_ptr<ScoreDoc>> &hits)
{
  Assert::assertEquals(mes + L" nr of hits", hits.size(), results.size());
  for (int i = 0; i < results.size(); i++) {
    Assert::assertEquals(mes + L" doc nrs for hit " + to_wstring(i), results[i],
                         hits[i]->doc);
  }
}

void CheckHits::checkHitsQuery(shared_ptr<Query> query,
                               std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                               std::deque<std::shared_ptr<ScoreDoc>> &hits2,
                               std::deque<int> &results)
{

  checkDocIds(L"hits1", results, hits1);
  checkDocIds(L"hits2", results, hits2);
  checkEqual(query, hits1, hits2);
}

void CheckHits::checkEqual(shared_ptr<Query> query,
                           std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                           std::deque<std::shared_ptr<ScoreDoc>> &hits2)
{
  constexpr float scoreTolerance = 1.0e-6f;
  if (hits1.size() != hits2.size()) {
    Assert::fail(L"Unequal lengths: hits1=" + hits1.size() + L",hits2=" +
                 hits2.size());
  }
  for (int i = 0; i < hits1.size(); i++) {
    if (hits1[i]->doc != hits2[i]->doc) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      Assert::fail(L"Hit " + to_wstring(i) + L" docnumbers don't match\n" +
                   hits2str(hits1, hits2, 0, 0) + L"for query:" +
                   query->toString());
    }

    if ((hits1[i]->doc != hits2[i]->doc) ||
        abs(hits1[i]->score - hits2[i]->score) > scoreTolerance) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      Assert::fail(
          L"Hit " + to_wstring(i) + L", doc nrs " + to_wstring(hits1[i]->doc) +
          L" and " + to_wstring(hits2[i]->doc) + L"\nunequal       : " +
          to_wstring(hits1[i]->score) + L"\n           and: " +
          to_wstring(hits2[i]->score) + L"\nfor query:" + query->toString());
    }
  }
}

wstring CheckHits::hits2str(std::deque<std::shared_ptr<ScoreDoc>> &hits1,
                            std::deque<std::shared_ptr<ScoreDoc>> &hits2,
                            int start, int end)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int len1 = hits1.empty() ? 0 : hits1.size();
  int len2 = hits2.empty() ? 0 : hits2.size();
  if (end <= 0) {
    end = max(len1, len2);
  }

  sb->append(L"Hits length1=")
      ->append(len1)
      ->append(L"\tlength2=")
      ->append(len2);

  sb->append(L'\n');
  for (int i = start; i < end; i++) {
    sb->append(L"hit=")->append(i)->append(L':');
    if (i < len1) {
      sb->append(L" doc")
          ->append(hits1[i]->doc)
          ->append(L'=')
          ->append(hits1[i]->score);
    } else {
      sb->append(L"               ");
    }
    sb->append(L",\t");
    if (i < len2) {
      sb->append(L" doc")
          ->append(hits2[i]->doc)
          ->append(L'=')
          ->append(hits2[i]->score);
    }
    sb->append(L'\n');
  }
  return sb->toString();
}

wstring CheckHits::topdocsString(shared_ptr<TopDocs> docs, int start, int end)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"TopDocs totalHits=")
      ->append(docs->totalHits)
      ->append(L" top=")
      ->append(docs->scoreDocs.size())
      ->append(L'\n');
  if (end <= 0) {
    end = docs->scoreDocs.size();
  } else {
    end = min(end, docs->scoreDocs.size());
  }
  for (int i = start; i < end; i++) {
    sb->append(L'\t');
    sb->append(i);
    sb->append(L") doc=");
    sb->append(docs->scoreDocs[i]->doc);
    sb->append(L"\tscore=");
    sb->append(docs->scoreDocs[i]->score);
    sb->append(L'\n');
  }
  return sb->toString();
}

void CheckHits::checkExplanations(
    shared_ptr<Query> query, const wstring &defaultFieldName,
    shared_ptr<IndexSearcher> searcher) 
{
  checkExplanations(query, defaultFieldName, searcher, false);
}

void CheckHits::checkExplanations(shared_ptr<Query> query,
                                  const wstring &defaultFieldName,
                                  shared_ptr<IndexSearcher> searcher,
                                  bool deep) 
{

  searcher->search(query, make_shared<ExplanationAsserter>(
                              query, defaultFieldName, searcher, deep));
}

float CheckHits::explainToleranceDelta(float f1, float f2)
{
  return max(EXPLAIN_SCORE_TOLERANCE_MINIMUM,
             max(abs(f1), abs(f2)) * EXPLAIN_SCORE_TOLERANCE_DELTA);
}

void CheckHits::checkMatches(
    shared_ptr<Query> query,
    shared_ptr<IndexSearcher> searcher) 
{
  searcher->search(query, make_shared<MatchesAsserter>(query, searcher));
}

void CheckHits::verifyExplanation(const wstring &q, int doc, float score,
                                  bool deep, shared_ptr<Explanation> expl)
{
  float value = expl->getValue();
  Assert::assertEquals(q + L": score(doc=" + to_wstring(doc) + L")=" +
                           to_wstring(score) + L" != explanationScore=" +
                           to_wstring(value) + L" Explanation: " + expl,
                       score, value, explainToleranceDelta(score, value));

  if (!deep) {
    return;
  }

  std::deque<std::shared_ptr<Explanation>> detail = expl->getDetails();
  // TODO: can we improve this entire method? it's really geared to work only
  // with TF/IDF
  if (StringHelper::endsWith(expl->getDescription(), L"computed from:")) {
    return; // something more complicated.
  }
  wstring descr = expl->getDescription().toLowerCase(Locale::ROOT);
  if (StringHelper::startsWith(descr, L"score based on ") &&
      descr.find(L"child docs in range") != wstring::npos) {
    Assert::assertTrue(L"Child doc explanations are missing",
                       detail.size() > 0);
  }
  if (detail.size() > 0) {
    if (detail.size() == 1) {
      // simple containment, unless it's a freq of: (which lets a query explain
      // how the freq is calculated), just verify contained expl has same score
      if (StringHelper::endsWith(expl->getDescription(), L"with freq of:") ==
              false &&
          (score >= 0 ||
           StringHelper::endsWith(expl->getDescription(),
                                  L"times others of:") == false)) {
        verifyExplanation(q, doc, score, deep, detail[0]);
      }
    } else {
      // explanation must either:
      // - end with one of: "product of:", "sum of:", "max of:", or
      // - have "max plus <x> times others" (where <x> is float).
      float x = 0;
      bool productOf = StringHelper::endsWith(descr, L"product of:");
      bool sumOf = StringHelper::endsWith(descr, L"sum of:");
      bool maxOf = StringHelper::endsWith(descr, L"max of:");
      bool computedOf = regex_match(descr, regex(L".*, computed as .* from:"));
      bool maxTimesOthers = false;
      if (!(productOf || sumOf || maxOf || computedOf)) {
        // maybe 'max plus x times others'
        int k1 = (int)descr.find(L"max plus ");
        if (k1 >= 0) {
          k1 += (wstring(L"max plus ")).length();
          int k2 = (int)descr.find(L" ", k1);
          try {
            x = stof(descr.substr(k1, k2 - k1)->trim());
            if (descr.substr(k2)->trim()->equals(L"times others of:")) {
              maxTimesOthers = true;
            }
          } catch (const NumberFormatException &e) {
          }
        }
      }
      // TODO: this is a TERRIBLE assertion!!!!
      Assert::assertTrue(
          q + L": multi valued explanation description=\"" + descr +
              L"\" must be 'max of plus x times others', 'computed as x from:' "
              L"or end with 'product of'" +
              L" or 'sum of:' or 'max of:' - " + expl,
          productOf || sumOf || maxOf || computedOf || maxTimesOthers);
      float sum = 0;
      float product = 1;
      float max = 0;
      for (int i = 0; i < detail.size(); i++) {
        float dval = detail[i]->getValue();
        verifyExplanation(q, doc, dval, deep, detail[i]);
        product *= dval;
        sum += dval;
        max = max(max, dval);
      }
      float combined = 0;
      if (productOf) {
        combined = product;
      } else if (sumOf) {
        combined = sum;
      } else if (maxOf) {
        combined = max;
      } else if (maxTimesOthers) {
        combined = max + x * (sum - max);
      } else {
        Assert::assertTrue(L"should never get here!", computedOf);
        combined = value;
      }
      Assert::assertEquals(
          q + L": actual subDetails combined==" + to_wstring(combined) +
              L" != value=" + to_wstring(value) + L" Explanation: " + expl,
          combined, value, explainToleranceDelta(combined, value));
    }
  }
}

CheckHits::ExplanationAssertingSearcher::ExplanationAssertingSearcher(
    shared_ptr<IndexReader> r)
    : IndexSearcher(r)
{
}

void CheckHits::ExplanationAssertingSearcher::checkExplanations(
    shared_ptr<Query> q) 
{
  IndexSearcher::search(
      q, make_shared<ExplanationAsserter>(q, nullptr, shared_from_this()));
}

shared_ptr<TopFieldDocs> CheckHits::ExplanationAssertingSearcher::search(
    shared_ptr<Query> query, int n, shared_ptr<Sort> sort) 
{

  checkExplanations(query);
  return IndexSearcher::search(query, n, sort);
}

void CheckHits::ExplanationAssertingSearcher::search(
    shared_ptr<Query> query, shared_ptr<Collector> results) 
{
  checkExplanations(query);
  IndexSearcher::search(query, results);
}

shared_ptr<TopDocs>
CheckHits::ExplanationAssertingSearcher::search(shared_ptr<Query> query,
                                                int n) 
{

  checkExplanations(query);
  return IndexSearcher::search(query, n);
}

CheckHits::ExplanationAsserter::ExplanationAsserter(
    shared_ptr<Query> q, const wstring &defaultFieldName,
    shared_ptr<IndexSearcher> s)
    : ExplanationAsserter(q, defaultFieldName, s, false)
{
}

CheckHits::ExplanationAsserter::ExplanationAsserter(
    shared_ptr<Query> q, const wstring &defaultFieldName,
    shared_ptr<IndexSearcher> s, bool deep)
{
  this->q = q;
  this->s = s;
  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->d = q->toString(defaultFieldName);
  this->deep = deep;
}

void CheckHits::ExplanationAsserter::setScorer(shared_ptr<Scorer> scorer) throw(
    IOException)
{
  this->scorer = scorer;
}

void CheckHits::ExplanationAsserter::collect(int doc) 
{
  shared_ptr<Explanation> exp = nullptr;
  doc = doc + base;
  try {
    exp = s->explain(q, doc);
  } catch (const IOException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("exception in hitcollector of
    // [[" + d + "]] for #" + doc, e);
    throw runtime_error(L"exception in hitcollector of [[" + d + L"]] for #" +
                        to_wstring(doc));
  }

  assertNotNull(L"Explanation of [[" + d + L"]] for #" + to_wstring(doc) +
                    L" is null",
                exp);
  verifyExplanation(d, doc, scorer->score(), deep, exp);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  Assert::assertTrue(L"Explanation of [[" + d + L"]] for #" + to_wstring(doc) +
                         L" does not indicate match: " + exp->toString(),
                     exp->isMatch());
}

void CheckHits::ExplanationAsserter::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  base = context->docBase;
}

bool CheckHits::ExplanationAsserter::needsScores() { return true; }

CheckHits::MatchesAsserter::MatchesAsserter(
    shared_ptr<Query> query,
    shared_ptr<IndexSearcher> searcher) 
    : weight(searcher->createWeight(searcher->rewrite(query), false, 1))
{
}

void CheckHits::MatchesAsserter::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  this->context = context;
  this->lastCheckedDoc = -1;
}

void CheckHits::MatchesAsserter::collect(int doc) 
{
  shared_ptr<Matches> matches = this->weight->matches(context, doc);
  assertNotNull(L"Unexpected null Matches object in doc" + to_wstring(doc) +
                    L" for query " + this->weight->getQuery(),
                matches);
  if (lastCheckedDoc != doc - 1) {
    assertNull(L"Unexpected non-null Matches object in non-matching doc" +
                   to_wstring(doc) + L" for query " + this->weight->getQuery(),
               this->weight->matches(context, doc - 1));
  }
  lastCheckedDoc = doc;
}

bool CheckHits::MatchesAsserter::needsScores() { return false; }
} // namespace org::apache::lucene::search