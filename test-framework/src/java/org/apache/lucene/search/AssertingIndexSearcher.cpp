using namespace std;

#include "AssertingIndexSearcher.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;

AssertingIndexSearcher::AssertingIndexSearcher(shared_ptr<Random> random,
                                               shared_ptr<IndexReader> r)
    : IndexSearcher(r), random(make_shared<Random>(random->nextLong()))
{
}

AssertingIndexSearcher::AssertingIndexSearcher(
    shared_ptr<Random> random, shared_ptr<IndexReaderContext> context)
    : IndexSearcher(context), random(make_shared<Random>(random->nextLong()))
{
}

AssertingIndexSearcher::AssertingIndexSearcher(shared_ptr<Random> random,
                                               shared_ptr<IndexReader> r,
                                               shared_ptr<ExecutorService> ex)
    : IndexSearcher(r, ex), random(make_shared<Random>(random->nextLong()))
{
}

AssertingIndexSearcher::AssertingIndexSearcher(
    shared_ptr<Random> random, shared_ptr<IndexReaderContext> context,
    shared_ptr<ExecutorService> ex)
    : IndexSearcher(context, ex),
      random(make_shared<Random>(random->nextLong()))
{
}

shared_ptr<Weight>
AssertingIndexSearcher::createWeight(shared_ptr<Query> query, bool needsScores,
                                     float boost) 
{
  // this adds assertions to the inner weights/scorers too
  return make_shared<AssertingWeight>(
      random, IndexSearcher::createWeight(query, needsScores, boost),
      needsScores);
}

shared_ptr<Query>
AssertingIndexSearcher::rewrite(shared_ptr<Query> original) 
{
  // TODO: use the more sophisticated QueryUtils.check sometimes!
  QueryUtils::check(original);
  shared_ptr<Query> rewritten = IndexSearcher::rewrite(original);
  QueryUtils::check(rewritten);
  return rewritten;
}

void AssertingIndexSearcher::search(
    deque<std::shared_ptr<LeafReaderContext>> &leaves,
    shared_ptr<Weight> weight,
    shared_ptr<Collector> collector) 
{
  assert(std::dynamic_pointer_cast<AssertingWeight>(weight) != nullptr);
  IndexSearcher::search(leaves, weight,
                        AssertingCollector::wrap(random, collector));
}

wstring AssertingIndexSearcher::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"AssertingIndexSearcher(" + IndexSearcher::toString() + L")";
}
} // namespace org::apache::lucene::search