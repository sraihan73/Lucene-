using namespace std;

#include "ReqExclBulkScorer.h"

namespace org::apache::lucene::search
{
using Bits = org::apache::lucene::util::Bits;

ReqExclBulkScorer::ReqExclBulkScorer(shared_ptr<BulkScorer> req,
                                     shared_ptr<DocIdSetIterator> excl)
    : req(req), excl(excl)
{
}

int ReqExclBulkScorer::score(shared_ptr<LeafCollector> collector,
                             shared_ptr<Bits> acceptDocs, int min,
                             int max) 
{
  int upTo = min;
  int exclDoc = excl->docID();

  while (upTo < max) {
    if (exclDoc < upTo) {
      exclDoc = excl->advance(upTo);
    }
    if (exclDoc == upTo) {
      // upTo is excluded so we can consider that we scored up to upTo+1
      upTo += 1;
      exclDoc = excl->nextDoc();
    } else {
      upTo = req->score(collector, acceptDocs, upTo, min(exclDoc, max));
    }
  }

  if (upTo == max) {
    upTo = req->score(collector, acceptDocs, upTo, upTo);
  }

  return upTo;
}

int64_t ReqExclBulkScorer::cost() { return req->cost(); }
} // namespace org::apache::lucene::search