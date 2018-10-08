using namespace std;

#include "CheckJoinIndex.h"

namespace org::apache::lucene::search::join
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSet = org::apache::lucene::util::BitSet;
using BitSetIterator = org::apache::lucene::util::BitSetIterator;
using Bits = org::apache::lucene::util::Bits;

CheckJoinIndex::CheckJoinIndex() {}

void CheckJoinIndex::check(
    shared_ptr<IndexReader> reader,
    shared_ptr<BitSetProducer> parentsFilter) 
{
  for (auto context : reader->leaves()) {
    if (context->reader()->maxDoc() == 0) {
      continue;
    }
    shared_ptr<BitSet> *const parents = parentsFilter->getBitSet(context);
    if (parents == nullptr || parents->cardinality() == 0) {
      throw make_shared<IllegalStateException>(
          L"Every segment should have at least one parent, but " +
          context->reader() + L" does not have any");
    }
    if (parents->get(context->reader()->maxDoc() - 1) == false) {
      throw make_shared<IllegalStateException>(
          L"The last document of a segment must always be a parent, but " +
          context->reader() + L" has a child as a last doc");
    }
    shared_ptr<Bits> *const liveDocs = context->reader()->getLiveDocs();
    if (liveDocs != nullptr) {
      int prevParentDoc = -1;
      shared_ptr<DocIdSetIterator> it =
          make_shared<BitSetIterator>(parents, 0LL);
      for (int parentDoc = it->nextDoc();
           parentDoc != DocIdSetIterator::NO_MORE_DOCS;
           parentDoc = it->nextDoc()) {
        constexpr bool parentIsLive = liveDocs->get(parentDoc);
        for (int child = prevParentDoc + 1; child != parentDoc; child++) {
          constexpr bool childIsLive = liveDocs->get(child);
          if (parentIsLive != childIsLive) {
            if (childIsLive) {
              throw make_shared<IllegalStateException>(
                  L"Parent doc " + to_wstring(parentDoc) + L" of segment " +
                  context->reader() +
                  L" is live but has a deleted child document " +
                  to_wstring(child));
            } else {
              throw make_shared<IllegalStateException>(
                  L"Parent doc " + to_wstring(parentDoc) + L" of segment " +
                  context->reader() +
                  L" is deleted but has a live child document " +
                  to_wstring(child));
            }
          }
        }
        prevParentDoc = parentDoc;
      }
    }
  }
}
} // namespace org::apache::lucene::search::join