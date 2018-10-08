using namespace std;

#include "TokenInfoFST.h"

namespace org::apache::lucene::analysis::ko::dict
{
using org::apache::lucene::util::fst::FST::Arc;
using FST = org::apache::lucene::util::fst::FST;

TokenInfoFST::TokenInfoFST(shared_ptr<FST<int64_t>> fst) 
    : fst(fst), cacheCeiling(0xD7A3), rootCache(cacheRootArcs()),
      NO_OUTPUT(fst->outputs->getNoOutput())
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) private
// org.apache.lucene.util.fst.FST.Arc<long>[] cacheRootArcs() throws
// java.io.IOException
std::deque<Arc<int64_t>> TokenInfoFST::cacheRootArcs() 
{
  std::deque<Arc<int64_t>> rootCache =
      std::deque<std::shared_ptr<Arc>>(1 + (cacheCeiling - 0xAC00));
  shared_ptr<Arc<int64_t>> firstArc = make_shared<Arc<int64_t>>();
  fst->getFirstArc(firstArc);
  shared_ptr<Arc<int64_t>> arc = make_shared<Arc<int64_t>>();
  shared_ptr<FST::BytesReader> *const fstReader = fst->getBytesReader();
  // TODO: jump to AC00, readNextRealArc to ceiling? (just be careful we don't
  // add bugs)
  for (int i = 0; i < rootCache.size(); i++) {
    if (fst->findTargetArc(0xAC00 + i, firstArc, arc, fstReader) != nullptr) {
      rootCache[i] = (make_shared<Arc<int64_t>>())->copyFrom(arc);
    }
  }
  return rootCache;
}

shared_ptr<Arc<int64_t>> TokenInfoFST::findTargetArc(
    int ch, shared_ptr<Arc<int64_t>> follow, shared_ptr<Arc<int64_t>> arc,
    bool useCache, shared_ptr<FST::BytesReader> fstReader) 
{
  if (useCache && ch >= 0xAC00 && ch <= cacheCeiling) {
    assert(ch != FST::END_LABEL);
    shared_ptr<Arc<int64_t>> *const result = rootCache[ch - 0xAC00];
    if (result == nullptr) {
      return nullptr;
    } else {
      arc->copyFrom(result);
      return arc;
    }
  } else {
    return fst->findTargetArc(ch, follow, arc, fstReader);
  }
}

shared_ptr<Arc<int64_t>>
TokenInfoFST::getFirstArc(shared_ptr<Arc<int64_t>> arc)
{
  return fst->getFirstArc(arc);
}

shared_ptr<FST::BytesReader> TokenInfoFST::getBytesReader()
{
  return fst->getBytesReader();
}

shared_ptr<FST<int64_t>> TokenInfoFST::getInternalFST() { return fst; }
} // namespace org::apache::lucene::analysis::ko::dict