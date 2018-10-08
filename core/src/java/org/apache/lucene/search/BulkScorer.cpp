using namespace std;

#include "BulkScorer.h"

namespace org::apache::lucene::search
{
using Bits = org::apache::lucene::util::Bits;

void BulkScorer::score(shared_ptr<LeafCollector> collector,
                       shared_ptr<Bits> acceptDocs) 
{
  constexpr int next =
      score(collector, acceptDocs, 0, DocIdSetIterator::NO_MORE_DOCS);
  assert(next == DocIdSetIterator::NO_MORE_DOCS);
}
} // namespace org::apache::lucene::search