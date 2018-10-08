using namespace std;

#include "QueryBitSetProducer.h"

namespace org::apache::lucene::search::join
{
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BitDocIdSet = org::apache::lucene::util::BitDocIdSet;
using BitSet = org::apache::lucene::util::BitSet;

QueryBitSetProducer::QueryBitSetProducer(shared_ptr<Query> query) : query(query)
{
}

shared_ptr<Query> QueryBitSetProducer::getQuery() { return query; }

shared_ptr<BitSet> QueryBitSetProducer::getBitSet(
    shared_ptr<LeafReaderContext> context) 
{
  shared_ptr<LeafReader> *const reader = context->reader();
  shared_ptr<IndexReader::CacheHelper> *const cacheHelper =
      reader->getCoreCacheHelper();

  shared_ptr<DocIdSet> docIdSet = nullptr;
  if (cacheHelper != nullptr) {
    docIdSet = cache[cacheHelper->getKey()];
  }
  if (docIdSet == nullptr) {
    shared_ptr<IndexReaderContext> *const topLevelContext =
        ReaderUtil::getTopLevelContext(context);
    shared_ptr<IndexSearcher> *const searcher =
        make_shared<IndexSearcher>(topLevelContext);
    searcher->setQueryCache(nullptr);
    shared_ptr<Query> *const rewritten = searcher->rewrite(query);
    shared_ptr<Weight> *const weight =
        searcher->createWeight(rewritten, false, 1);
    shared_ptr<Scorer> *const s = weight->scorer(context);

    if (s == nullptr) {
      docIdSet = DocIdSet::EMPTY;
    } else {
      docIdSet = make_shared<BitDocIdSet>(
          BitSet::of(s->begin(), context->reader()->maxDoc()));
    }
    if (cacheHelper != nullptr) {
      cache.emplace(cacheHelper->getKey(), docIdSet);
    }
  }
  return docIdSet == DocIdSet::EMPTY
             ? nullptr
             : (std::static_pointer_cast<BitDocIdSet>(docIdSet))->bits();
}

wstring QueryBitSetProducer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return getClass().getSimpleName() + L"(" + query->toString() + L")";
}

bool QueryBitSetProducer::equals(any o)
{
  if (o == nullptr || getClass() != o.type()) {
    return false;
  }
  shared_ptr<QueryBitSetProducer> *const other =
      any_cast<std::shared_ptr<QueryBitSetProducer>>(o);
  return this->query->equals(other->query);
}

int QueryBitSetProducer::hashCode()
{
  return 31 * getClass().hashCode() + query->hashCode();
}
} // namespace org::apache::lucene::search::join