using namespace std;

#include "ScorerIndexSearcher.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Bits = org::apache::lucene::util::Bits;

ScorerIndexSearcher::ScorerIndexSearcher(shared_ptr<IndexReader> r,
                                         shared_ptr<ExecutorService> executor)
    : IndexSearcher(r, executor)
{
}

ScorerIndexSearcher::ScorerIndexSearcher(shared_ptr<IndexReader> r)
    : IndexSearcher(r)
{
}

void ScorerIndexSearcher::search(
    deque<std::shared_ptr<LeafReaderContext>> &leaves,
    shared_ptr<Weight> weight,
    shared_ptr<Collector> collector) 
{
  for (auto ctx : leaves) { // search each subreader
    // we force the use of Scorer (not BulkScorer) to make sure
    // that the scorer passed to LeafCollector.setScorer supports
    // Scorer.getChildren
    shared_ptr<Scorer> scorer = weight->scorer(ctx);
    if (scorer != nullptr) {
      shared_ptr<DocIdSetIterator> *const iterator = scorer->begin();
      shared_ptr<LeafCollector> *const leafCollector =
          collector->getLeafCollector(ctx);
      leafCollector->setScorer(scorer);
      shared_ptr<Bits> *const liveDocs = ctx->reader()->getLiveDocs();
      for (int doc = iterator->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
           doc = iterator->nextDoc()) {
        if (liveDocs == nullptr || liveDocs->get(doc)) {
          leafCollector->collect(doc);
        }
      }
    }
  }
}
} // namespace org::apache::lucene::search