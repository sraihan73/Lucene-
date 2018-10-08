using namespace std;

#include "PerThreadPKLookup.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

PerThreadPKLookup::PerThreadPKLookup(
    shared_ptr<IndexReader> r, const wstring &idFieldName) 
    : termsEnums(std::deque<std::shared_ptr<TermsEnum>>(leaves->size())),
      postingsEnums(std::deque<std::shared_ptr<PostingsEnum>>(leaves->size())),
      liveDocs(std::deque<std::shared_ptr<Bits>>(leaves->size())),
      docBases(std::deque<int>(leaves->size())), numSegs(numSegs),
      hasDeletions(hasDeletions)
{

  deque<std::shared_ptr<LeafReaderContext>> leaves =
      deque<std::shared_ptr<LeafReaderContext>>(r->leaves());

  // Larger segments are more likely to have the id, so we sort largest to
  // smallest by numDocs:
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(leaves, new
  // java.util.Comparator<LeafReaderContext>() C++ TODO: You cannot use
  // 'shared_from_this' in a constructor:
  sort(leaves.begin(), leaves.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  int numSegs = 0;
  bool hasDeletions = false;
  for (int i = 0; i < leaves.size(); i++) {
    shared_ptr<Terms> terms = leaves[i]->reader().terms(idFieldName);
    if (terms != nullptr) {
      termsEnums[numSegs] = terms->begin();
      assert(termsEnums[numSegs] != nullptr);
      docBases[numSegs] = leaves[i]->docBase;
      liveDocs[numSegs] = leaves[i]->reader().getLiveDocs();
      hasDeletions |= leaves[i]->reader().hasDeletions();
      numSegs++;
    }
  }
}

PerThreadPKLookup::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<PerThreadPKLookup> outerInstance)
{
  this->outerInstance = outerInstance;
}

int PerThreadPKLookup::ComparatorAnonymousInnerClass::compare(
    shared_ptr<LeafReaderContext> c1, shared_ptr<LeafReaderContext> c2)
{
  return c2->reader()->numDocs() - c1->reader()->numDocs();
}

int PerThreadPKLookup::lookup(shared_ptr<BytesRef> id) 
{
  for (int seg = 0; seg < numSegs; seg++) {
    if (termsEnums[seg]->seekExact(id)) {
      postingsEnums[seg] = termsEnums[seg]->postings(postingsEnums[seg], 0);
      int docID = postingsEnums[seg]->nextDoc();
      if (docID != PostingsEnum::NO_MORE_DOCS &&
          (liveDocs[seg] == nullptr || liveDocs[seg]->get(docID))) {
        return docBases[seg] + docID;
      }
      assert(hasDeletions);
    }
  }

  return -1;
}
} // namespace org::apache::lucene::index