using namespace std;

#include "SuggestIndexSearcher.h"

namespace org::apache::lucene::search::suggest::document
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using BulkScorer = org::apache::lucene::search::BulkScorer;
using CollectionTerminatedException =
    org::apache::lucene::search::CollectionTerminatedException;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;

SuggestIndexSearcher::SuggestIndexSearcher(shared_ptr<IndexReader> reader)
    : org::apache::lucene::search::IndexSearcher(reader)
{
}

shared_ptr<TopSuggestDocs>
SuggestIndexSearcher::suggest(shared_ptr<CompletionQuery> query, int n,
                              bool skipDuplicates) 
{
  shared_ptr<TopSuggestDocsCollector> collector =
      make_shared<TopSuggestDocsCollector>(n, skipDuplicates);
  suggest(query, collector);
  return collector->get();
}

void SuggestIndexSearcher::suggest(
    shared_ptr<CompletionQuery> query,
    shared_ptr<TopSuggestDocsCollector> collector) 
{
  // TODO use IndexSearcher.rewrite instead
  // have to implement equals() and hashCode() in CompletionQuerys and co
  query = std::static_pointer_cast<CompletionQuery>(
      query->rewrite(getIndexReader()));
  shared_ptr<Weight> weight =
      query->createWeight(shared_from_this(), collector->needsScores(), 1.0f);
  for (auto context : getIndexReader()->leaves()) {
    shared_ptr<BulkScorer> scorer = weight->bulkScorer(context);
    if (scorer != nullptr) {
      try {
        scorer->score(collector->getLeafCollector(context),
                      context->reader()->getLiveDocs());
      } catch (const CollectionTerminatedException &e) {
        // collection was terminated prematurely
        // continue with the following leaf
      }
    }
  }
}
} // namespace org::apache::lucene::search::suggest::document