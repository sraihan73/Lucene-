using namespace std;

#include "QueryUtils.h"

namespace org::apache::lucene::search
{
using junit::framework::Assert;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafMetaData = org::apache::lucene::index::LeafMetaData;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using MultiReader = org::apache::lucene::index::MultiReader;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using PointValues = org::apache::lucene::index::PointValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Terms = org::apache::lucene::index::Terms;
using Bits = org::apache::lucene::util::Bits;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using Version = org::apache::lucene::util::Version;
//    import static junit.framework.Assert.assertEquals;
//    import static junit.framework.Assert.assertFalse;
//    import static junit.framework.Assert.assertTrue;

void QueryUtils::check(shared_ptr<Query> q) { checkHashEquals(q); }

void QueryUtils::checkHashEquals(shared_ptr<Query> q)
{
  checkEqual(q, q);

  // test that a class check is done so that no exception is thrown
  // in the implementation of equals()
  shared_ptr<Query> whacky = make_shared<QueryAnonymousInnerClass>();
  checkUnequal(q, whacky);

  // null test
  assertFalse(q->equals(nullptr));
}

QueryUtils::QueryAnonymousInnerClass::QueryAnonymousInnerClass() {}

wstring QueryUtils::QueryAnonymousInnerClass::toString(const wstring &field)
{
  return L"My Whacky Query";
}

bool QueryUtils::QueryAnonymousInnerClass::equals(any o)
{
  return o == shared_from_this();
}

int QueryUtils::QueryAnonymousInnerClass::hashCode()
{
  return System::identityHashCode(shared_from_this());
}

void QueryUtils::checkEqual(shared_ptr<Query> q1, shared_ptr<Query> q2)
{
  assertEquals(q1, q2);
  assertEquals(q1->hashCode(), q2->hashCode());
}

void QueryUtils::checkUnequal(shared_ptr<Query> q1, shared_ptr<Query> q2)
{
  assertFalse(q1 + L" equal to " + q2, q1->equals(q2));
  assertFalse(q2 + L" equal to " + q1, q2->equals(q1));
}

void QueryUtils::checkExplanations(
    shared_ptr<Query> q, shared_ptr<IndexSearcher> s) 
{
  CheckHits::checkExplanations(q, L"", s, true);
}

void QueryUtils::check(shared_ptr<Random> random, shared_ptr<Query> q1,
                       shared_ptr<IndexSearcher> s)
{
  check(random, q1, s, true);
}

void QueryUtils::check(shared_ptr<Random> random, shared_ptr<Query> q1,
                       shared_ptr<IndexSearcher> s, bool wrap)
{
  try {
    check(q1);
    if (s != nullptr) {
      checkFirstSkipTo(q1, s);
      checkSkipTo(q1, s);
      checkBulkScorerSkipTo(random, q1, s);
      if (wrap) {
        check(random, q1, wrapUnderlyingReader(random, s, -1), false);
        check(random, q1, wrapUnderlyingReader(random, s, 0), false);
        check(random, q1, wrapUnderlyingReader(random, s, +1), false);
      }
      checkExplanations(q1, s);
      CheckHits::checkMatches(q1, s);
    }
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

shared_ptr<IndexSearcher>
QueryUtils::wrapUnderlyingReader(shared_ptr<Random> random,
                                 shared_ptr<IndexSearcher> s,
                                 int const edge) 
{

  shared_ptr<IndexReader> r = s->getIndexReader();

  // we can't put deleted docs before the nested reader, because
  // it will throw off the docIds
  std::deque<std::shared_ptr<IndexReader>> readers = {
      edge < 0 ? r : make_shared<MultiReader>(),
      make_shared<MultiReader>(),
      make_shared<MultiReader>(edge < 0 ? emptyReader(4)
                                        : make_shared<MultiReader>(),
                               make_shared<MultiReader>(),
                               0 == edge ? r : make_shared<MultiReader>()),
      0 < edge ? make_shared<MultiReader>() : emptyReader(7),
      make_shared<MultiReader>(),
      make_shared<MultiReader>(0 < edge ? make_shared<MultiReader>()
                                        : emptyReader(5),
                               make_shared<MultiReader>(),
                               0 < edge ? r : make_shared<MultiReader>())};

  shared_ptr<IndexSearcher> out =
      LuceneTestCase::newSearcher(make_shared<MultiReader>(readers));
  out->setSimilarity(s->getSimilarity(true));
  return out;
}

shared_ptr<IndexReader> QueryUtils::emptyReader(int const maxDoc)
{
  return make_shared<LeafReaderAnonymousInnerClass>(maxDoc);
}

QueryUtils::LeafReaderAnonymousInnerClass::LeafReaderAnonymousInnerClass(
    int maxDoc)
{
  this->maxDoc = maxDoc;
}

shared_ptr<Terms> QueryUtils::LeafReaderAnonymousInnerClass::terms(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<NumericDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getNumericDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<BinaryDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getBinaryDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<SortedDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getSortedDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<SortedNumericDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getSortedNumericDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<SortedSetDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getSortedSetDocValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<NumericDocValues>
QueryUtils::LeafReaderAnonymousInnerClass::getNormValues(
    const wstring &field) 
{
  return nullptr;
}

shared_ptr<FieldInfos>
QueryUtils::LeafReaderAnonymousInnerClass::getFieldInfos()
{
  return make_shared<FieldInfos>(std::deque<std::shared_ptr<FieldInfo>>(0));
}

shared_ptr<Bits> QueryUtils::LeafReaderAnonymousInnerClass::getLiveDocs()
{
  return liveDocs;
}

shared_ptr<PointValues>
QueryUtils::LeafReaderAnonymousInnerClass::getPointValues(
    const wstring &fieldName)
{
  return nullptr;
}

void QueryUtils::LeafReaderAnonymousInnerClass::checkIntegrity() throw(
    IOException)
{
}

shared_ptr<Fields> QueryUtils::LeafReaderAnonymousInnerClass::getTermVectors(
    int docID) 
{
  return nullptr;
}

int QueryUtils::LeafReaderAnonymousInnerClass::numDocs() { return 0; }

int QueryUtils::LeafReaderAnonymousInnerClass::maxDoc() { return maxDoc; }

void QueryUtils::LeafReaderAnonymousInnerClass::document(
    int docID, shared_ptr<StoredFieldVisitor> visitor) 
{
}

void QueryUtils::LeafReaderAnonymousInnerClass::doClose()  {}

shared_ptr<LeafMetaData>
QueryUtils::LeafReaderAnonymousInnerClass::getMetaData()
{
  return make_shared<LeafMetaData>(Version::LATEST->major, Version::LATEST,
                                   nullptr);
}

shared_ptr<IndexReader::CacheHelper>
QueryUtils::LeafReaderAnonymousInnerClass::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<IndexReader::CacheHelper>
QueryUtils::LeafReaderAnonymousInnerClass::getReaderCacheHelper()
{
  return nullptr;
}

void QueryUtils::checkSkipTo(shared_ptr<Query> q,
                             shared_ptr<IndexSearcher> s) 
{
  // System.out.println("Checking "+q);
  const deque<std::shared_ptr<LeafReaderContext>> readerContextArray =
      s->getTopReaderContext()->leaves();

  constexpr int skip_op = 0;
  constexpr int next_op = 1;
  const std::deque<std::deque<int>> orders = {
      std::deque<int>{next_op},
      std::deque<int>{skip_op},
      std::deque<int>{skip_op, next_op},
      std::deque<int>{next_op, skip_op},
      std::deque<int>{skip_op, skip_op, next_op, next_op},
      std::deque<int>{next_op, next_op, skip_op, skip_op},
      std::deque<int>{skip_op, skip_op, skip_op, next_op, next_op}};
  for (int k = 0; k < orders.size(); k++) {

    const std::deque<int> order = orders[k];
    // System.out.print("Order:");for (int i = 0; i < order.length; i++)
    // System.out.print(order[i]==skip_op ? " skip()":" next()");
    // System.out.println();
    const std::deque<int> opidx = {0};
    const std::deque<int> lastDoc = {-1};

    // FUTURE: ensure scorer.doc()==-1

    constexpr float maxDiff = 1e-5.0f;
    std::deque<std::shared_ptr<LeafReader>> lastReader = {nullptr};

    s->search(q, make_shared<SimpleCollectorAnonymousInnerClass>(
                     q, s, readerContextArray, skip_op, order, opidx, lastDoc,
                     maxDiff, lastReader));

    if (lastReader[0] != nullptr) {
      // confirm that skipping beyond the last doc, on the
      // previous reader, hits NO_MORE_DOCS
      shared_ptr<LeafReader> *const previousReader = lastReader[0];
      shared_ptr<IndexSearcher> indexSearcher =
          LuceneTestCase::newSearcher(previousReader, false);
      indexSearcher->setSimilarity(s->getSimilarity(true));
      shared_ptr<Query> rewritten = indexSearcher->rewrite(q);
      shared_ptr<Weight> w = indexSearcher->createWeight(rewritten, true, 1);
      shared_ptr<LeafReaderContext> ctx = previousReader->getContext();
      shared_ptr<Scorer> scorer = w->scorer(ctx);
      if (scorer != nullptr) {
        shared_ptr<DocIdSetIterator> iterator = scorer->begin();
        bool more = false;
        shared_ptr<Bits> *const liveDocs = lastReader[0]->getLiveDocs();
        for (int d = iterator->advance(lastDoc[0] + 1);
             d != DocIdSetIterator::NO_MORE_DOCS; d = iterator->nextDoc()) {
          if (liveDocs == nullptr || liveDocs->get(d)) {
            more = true;
            break;
          }
        }
        Assert::assertFalse(L"query's last doc was " + to_wstring(lastDoc[0]) +
                                L" but advance(" + to_wstring(lastDoc[0] + 1) +
                                L") got to " + to_wstring(scorer->docID()),
                            more);
      }
    }
  }
}

QueryUtils::SimpleCollectorAnonymousInnerClass::
    SimpleCollectorAnonymousInnerClass(
        shared_ptr<org::apache::lucene::search::Query> q,
        shared_ptr<org::apache::lucene::search::IndexSearcher> s,
        deque<std::shared_ptr<LeafReaderContext>> &readerContextArray,
        int skip_op, deque<int> &order, deque<int> &opidx,
        deque<int> &lastDoc, float maxDiff,
        deque<std::shared_ptr<LeafReader>> &lastReader)
{
  this->q = q;
  this->s = s;
  this->readerContextArray = readerContextArray;
  this->skip_op = skip_op;
  this->order = order;
  this->opidx = opidx;
  this->lastDoc = lastDoc;
  this->maxDiff = maxDiff;
  this->lastReader = lastReader;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->sc = scorer;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  float score = sc::score();
  lastDoc[0] = doc;
  try {
    if (scorer == nullptr) {
      shared_ptr<Query> rewritten = s->rewrite(q);
      shared_ptr<Weight> w = s->createWeight(rewritten, true, 1);
      shared_ptr<LeafReaderContext> context = readerContextArray[leafPtr];
      scorer = w->scorer(context);
      iterator = scorer::begin();
    }

    int op = order[(opidx[0]++) % order.size()];
    // System.out.println(op==skip_op ?
    // "skip("+(sdoc[0]+1)+")":"next()");
    bool more = op == skip_op
                    ? iterator::advance(scorer::docID() + 1) !=
                          DocIdSetIterator::NO_MORE_DOCS
                    : iterator::nextDoc() != DocIdSetIterator::NO_MORE_DOCS;
    int scorerDoc = scorer::docID();
    float scorerScore = scorer::score();
    float scorerScore2 = scorer::score();
    float scoreDiff = abs(score - scorerScore);
    float scorerDiff = abs(scorerScore2 - scorerScore);

    bool success = false;
    try {
      assertTrue(more);
      assertEquals(L"scorerDoc=" + to_wstring(scorerDoc) + L",doc=" +
                       to_wstring(doc),
                   scorerDoc, doc);
      assertTrue(L"score=" + to_wstring(score) + L", scorerScore=" +
                     to_wstring(scorerScore),
                 scoreDiff <= maxDiff);
      assertTrue(L"scorerScorer=" + to_wstring(scorerScore) +
                     L", scorerScore2=" + to_wstring(scorerScore2),
                 scorerDiff <= maxDiff);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        if (LuceneTestCase::VERBOSE) {
          shared_ptr<StringBuilder> sbord = make_shared<StringBuilder>();
          for (int i = 0; i < order.size(); i++) {
            sbord->append(order[i] == skip_op ? L" skip()" : L" next()");
          }
          wcout << L"ERROR matching docs:" << L"\n\t"
                << (doc != scorerDoc ? L"--> " : L"") << L"doc=" << doc
                << L", scorerDoc=" << scorerDoc << L"\n\t"
                << (!more ? L"--> " : L"") << L"tscorer.more=" << more
                << L"\n\t" << (scoreDiff > maxDiff ? L"--> " : L"")
                << L"scorerScore=" << scorerScore << L" scoreDiff=" << scoreDiff
                << L" maxDiff=" << maxDiff << L"\n\t"
                << (scorerDiff > maxDiff ? L"--> " : L"") << L"scorerScore2="
                << scorerScore2 << L" scorerDiff=" << scorerDiff
                << L"\n\thitCollector.doc=" << doc << L" score=" << score
                << L"\n\t Scorer=" << scorer << L"\n\t Query=" << q << L"  "
                << q->getClassName() << L"\n\t Searcher=" << s << L"\n\t Order="
                << sbord << L"\n\t Op="
                << (op == skip_op ? L" skip()" : L" next()") << endl;
        }
      }
    }
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

bool QueryUtils::SimpleCollectorAnonymousInnerClass::needsScores()
{
  return true;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  // confirm that skipping beyond the last doc, on the
  // previous reader, hits NO_MORE_DOCS
  if (lastReader[0] != nullptr) {
    shared_ptr<LeafReader> *const previousReader = lastReader[0];
    shared_ptr<IndexSearcher> indexSearcher =
        LuceneTestCase::newSearcher(previousReader, false);
    indexSearcher->setSimilarity(s->getSimilarity(true));
    shared_ptr<Query> rewritten = indexSearcher->rewrite(q);
    shared_ptr<Weight> w = indexSearcher->createWeight(rewritten, true, 1);
    shared_ptr<LeafReaderContext> ctx =
        std::static_pointer_cast<LeafReaderContext>(
            indexSearcher->getTopReaderContext());
    shared_ptr<Scorer> scorer = w->scorer(ctx);
    if (scorer != nullptr) {
      shared_ptr<DocIdSetIterator> iterator = scorer->begin();
      bool more = false;
      shared_ptr<Bits> *const liveDocs = context->reader()->getLiveDocs();
      for (int d = iterator->advance(lastDoc[0] + 1);
           d != DocIdSetIterator::NO_MORE_DOCS; d = iterator->nextDoc()) {
        if (liveDocs == nullptr || liveDocs->get(d)) {
          more = true;
          break;
        }
      }
      Assert::assertFalse(L"query's last doc was " + to_wstring(lastDoc[0]) +
                              L" but advance(" + to_wstring(lastDoc[0] + 1) +
                              L") got to " + to_wstring(scorer->docID()),
                          more);
    }
    leafPtr++;
  }
  lastReader[0] = context->reader();
  assert(readerContextArray[leafPtr]->reader() == context->reader());
  this->scorer = nullptr;
  lastDoc[0] = -1;
}

void QueryUtils::checkFirstSkipTo(
    shared_ptr<Query> q, shared_ptr<IndexSearcher> s) 
{
  // System.out.println("checkFirstSkipTo: "+q);
  constexpr float maxDiff = 1e-3.0f;
  const std::deque<int> lastDoc = {-1};
  std::deque<std::shared_ptr<LeafReader>> lastReader = {nullptr};
  const deque<std::shared_ptr<LeafReaderContext>> context =
      s->getTopReaderContext()->leaves();
  shared_ptr<Query> rewritten = s->rewrite(q);
  s->search(q, make_shared<SimpleCollectorAnonymousInnerClass2>(
                   s, maxDiff, lastDoc, lastReader, context, rewritten));

  if (lastReader[0] != nullptr) {
    // confirm that skipping beyond the last doc, on the
    // previous reader, hits NO_MORE_DOCS
    shared_ptr<LeafReader> *const previousReader = lastReader[0];
    shared_ptr<IndexSearcher> indexSearcher =
        LuceneTestCase::newSearcher(previousReader, false);
    indexSearcher->setSimilarity(s->getSimilarity(true));
    shared_ptr<Weight> w = indexSearcher->createWeight(rewritten, true, 1);
    shared_ptr<Scorer> scorer =
        w->scorer(std::static_pointer_cast<LeafReaderContext>(
            indexSearcher->getTopReaderContext()));
    if (scorer != nullptr) {
      shared_ptr<DocIdSetIterator> iterator = scorer->begin();
      bool more = false;
      shared_ptr<Bits> *const liveDocs = lastReader[0]->getLiveDocs();
      for (int d = iterator->advance(lastDoc[0] + 1);
           d != DocIdSetIterator::NO_MORE_DOCS; d = iterator->nextDoc()) {
        if (liveDocs == nullptr || liveDocs->get(d)) {
          more = true;
          break;
        }
      }
      Assert::assertFalse(L"query's last doc was " + to_wstring(lastDoc[0]) +
                              L" but advance(" + to_wstring(lastDoc[0] + 1) +
                              L") got to " + to_wstring(scorer->docID()),
                          more);
    }
  }
}

QueryUtils::SimpleCollectorAnonymousInnerClass2::
    SimpleCollectorAnonymousInnerClass2(
        shared_ptr<org::apache::lucene::search::IndexSearcher> s, float maxDiff,
        deque<int> &lastDoc, deque<std::shared_ptr<LeafReader>> &lastReader,
        deque<std::shared_ptr<LeafReaderContext>> &context,
        shared_ptr<org::apache::lucene::search::Query> rewritten)
{
  this->s = s;
  this->maxDiff = maxDiff;
  this->lastDoc = lastDoc;
  this->lastReader = lastReader;
  this->context = context;
  this->rewritten = rewritten;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer)
{
  this->scorer = scorer;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass2::collect(int doc) throw(
    IOException)
{
  float score = scorer::score();
  try {
    int64_t startMS = System::currentTimeMillis();
    for (int i = lastDoc[0] + 1; i <= doc; i++) {
      shared_ptr<Weight> w = s->createWeight(rewritten, true, 1);
      shared_ptr<Scorer> scorer = w->scorer(context[leafPtr]);
      Assert::assertTrue(
          L"query collected " + to_wstring(doc) + L" but advance(" +
              to_wstring(i) + L") says no more docs!",
          scorer->begin().advance(i) != DocIdSetIterator::NO_MORE_DOCS);
      Assert::assertEquals(L"query collected " + to_wstring(doc) +
                               L" but advance(" + to_wstring(i) + L") got to " +
                               to_wstring(scorer->docID()),
                           doc, scorer->docID());
      float advanceScore = scorer->score();
      Assert::assertEquals(L"unstable advance(" + to_wstring(i) + L") score!",
                           advanceScore, scorer->score(), maxDiff);
      Assert::assertEquals(L"query assigned doc " + to_wstring(doc) +
                               L" a score of <" + to_wstring(score) +
                               L"> but advance(" + to_wstring(i) + L") has <" +
                               to_wstring(advanceScore) + L">!",
                           score, advanceScore, maxDiff);

      // Hurry things along if they are going slow (eg
      // if you got SimpleText codec this will kick in):
      if (i < doc && System::currentTimeMillis() - startMS > 5) {
        i = doc - 1;
      }
    }
    lastDoc[0] = doc;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

bool QueryUtils::SimpleCollectorAnonymousInnerClass2::needsScores()
{
  return true;
}

void QueryUtils::SimpleCollectorAnonymousInnerClass2::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  // confirm that skipping beyond the last doc, on the
  // previous reader, hits NO_MORE_DOCS
  if (lastReader[0] != nullptr) {
    shared_ptr<LeafReader> *const previousReader = lastReader[0];
    shared_ptr<IndexSearcher> indexSearcher =
        LuceneTestCase::newSearcher(previousReader, false);
    indexSearcher->setSimilarity(s->getSimilarity(true));
    shared_ptr<Weight> w = indexSearcher->createWeight(rewritten, true, 1);
    shared_ptr<Scorer> scorer =
        w->scorer(std::static_pointer_cast<LeafReaderContext>(
            indexSearcher->getTopReaderContext()));
    if (scorer != nullptr) {
      shared_ptr<DocIdSetIterator> iterator = scorer->begin();
      bool more = false;
      shared_ptr<Bits> *const liveDocs = context->reader()->getLiveDocs();
      for (int d = iterator->advance(lastDoc[0] + 1);
           d != DocIdSetIterator::NO_MORE_DOCS; d = iterator->nextDoc()) {
        if (liveDocs == nullptr || liveDocs->get(d)) {
          more = true;
          break;
        }
      }
      Assert::assertFalse(L"query's last doc was " + to_wstring(lastDoc[0]) +
                              L" but advance(" + to_wstring(lastDoc[0] + 1) +
                              L") got to " + to_wstring(scorer->docID()),
                          more);
    }
    leafPtr++;
  }

  lastReader[0] = context->reader();
  lastDoc[0] = -1;
}

void QueryUtils::checkBulkScorerSkipTo(
    shared_ptr<Random> r, shared_ptr<Query> query,
    shared_ptr<IndexSearcher> searcher) 
{
  query = searcher->rewrite(query);
  shared_ptr<Weight> weight = searcher->createWeight(query, true, 1);
  for (auto context : searcher->getIndexReader()->leaves()) {
    shared_ptr<Scorer> *const scorer = weight->scorer(context);
    shared_ptr<BulkScorer> *const bulkScorer = weight->bulkScorer(context);
    if (scorer == nullptr && bulkScorer == nullptr) {
      continue;
    } else if (bulkScorer == nullptr) {
      // ensure scorer is exhausted (it just didnt return null)
      assert(scorer->begin().nextDoc() == DocIdSetIterator::NO_MORE_DOCS);
      continue;
    }
    shared_ptr<DocIdSetIterator> iterator = scorer->begin();
    int upTo = 0;
    while (true) {
      constexpr int min = upTo + r->nextInt(5);
      constexpr int max = min + 1 + r->nextInt(r->nextBoolean() ? 10 : 5000);
      if (scorer->docID() < min) {
        iterator->advance(min);
      }
      constexpr int next =
          bulkScorer->score(make_shared<LeafCollectorAnonymousInnerClass>(
                                scorer, iterator, min, max),
                            nullptr, min, max);
      assert(max <= next);
      assert(next <= scorer->docID());
      upTo = max;

      if (scorer->docID() == DocIdSetIterator::NO_MORE_DOCS) {
        bulkScorer->score(
            make_shared<LeafCollectorAnonymousInnerClass2>(scorer), nullptr,
            upTo, DocIdSetIterator::NO_MORE_DOCS);
        break;
      }
    }
  }
}

QueryUtils::LeafCollectorAnonymousInnerClass::LeafCollectorAnonymousInnerClass(
    shared_ptr<org::apache::lucene::search::Scorer> scorer,
    shared_ptr<org::apache::lucene::search::DocIdSetIterator> iterator, int min,
    int max)
{
  this->scorer = scorer;
  this->iterator = iterator;
  this->min = min;
  this->max = max;
}

void QueryUtils::LeafCollectorAnonymousInnerClass::setScorer(
    shared_ptr<Scorer> scorer) 
{
  this->scorer2 = scorer;
}

void QueryUtils::LeafCollectorAnonymousInnerClass::collect(int doc) throw(
    IOException)
{
  assert(doc >= min);
  assert(doc < max);
  Assert::assertEquals(scorer->docID(), doc);
  Assert::assertEquals(scorer->score(), scorer2::score(), 0.01f);
  iterator->nextDoc();
}

QueryUtils::LeafCollectorAnonymousInnerClass2::
    LeafCollectorAnonymousInnerClass2(
        shared_ptr<org::apache::lucene::search::Scorer> scorer)
{
  this->scorer = scorer;
}

void QueryUtils::LeafCollectorAnonymousInnerClass2::setScorer(
    shared_ptr<Scorer> scorer) 
{
}

void QueryUtils::LeafCollectorAnonymousInnerClass2::collect(int doc) throw(
    IOException)
{
  // no more matches
  assert(false);
}
} // namespace org::apache::lucene::search