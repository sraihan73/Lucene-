using namespace std;

#include "DocIDMerger.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

DocIDMerger<T>::Sub::Sub(shared_ptr<MergeState::DocMap> docMap) : docMap(docMap)
{
}
} // namespace org::apache::lucene::index