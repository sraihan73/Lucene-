using namespace std;

#include "Terms.h"

namespace org::apache::lucene::index
{
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

Terms::Terms() {}

shared_ptr<TermsEnum>
Terms::intersect(shared_ptr<CompiledAutomaton> compiled,
                 shared_ptr<BytesRef> startTerm) 
{

  // TODO: could we factor out a common interface b/w
  // CompiledAutomaton and FST?  Then we could pass FST there too,
  // and likely speed up resolving terms to deleted docs ... but
  // AutomatonTermsEnum makes this tricky because of its on-the-fly cycle
  // detection

  // TODO: eventually we could support seekCeil/Exact on
  // the returned enum, instead of only being able to seek
  // at the start

  shared_ptr<TermsEnum> termsEnum = iterator();

  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }

  if (startTerm == nullptr) {
    return make_shared<AutomatonTermsEnum>(termsEnum, compiled);
  } else {
    return make_shared<AutomatonTermsEnumAnonymousInnerClass>(
        shared_from_this(), termsEnum, compiled, startTerm);
  }
}

Terms::AutomatonTermsEnumAnonymousInnerClass::
    AutomatonTermsEnumAnonymousInnerClass(
        shared_ptr<Terms> outerInstance,
        shared_ptr<org::apache::lucene::index::TermsEnum> termsEnum,
        shared_ptr<CompiledAutomaton> compiled, shared_ptr<BytesRef> startTerm)
    : AutomatonTermsEnum(termsEnum, compiled)
{
  this->outerInstance = outerInstance;
  this->startTerm = startTerm;
}

shared_ptr<BytesRef> Terms::AutomatonTermsEnumAnonymousInnerClass::nextSeekTerm(
    shared_ptr<BytesRef> term) 
{
  if (term == nullptr) {
    term = startTerm;
  }
  return AutomatonTermsEnum::nextSeekTerm(term);
}

std::deque<std::shared_ptr<Terms>> const Terms::EMPTY_ARRAY =
    std::deque<std::shared_ptr<Terms>>(0);

shared_ptr<BytesRef> Terms::getMin() 
{
  return iterator()->next();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public
// org.apache.lucene.util.BytesRef getMax() throws java.io.IOException
shared_ptr<BytesRef> Terms::getMax() 
{
  int64_t size = this->size();

  if (size == 0) {
    // empty: only possible from a FilteredTermsEnum...
    return nullptr;
  } else if (size >= 0) {
    // try to seek-by-ord
    try {
      shared_ptr<TermsEnum> iterator = this->iterator();
      iterator->seekExact(size - 1);
      return iterator->term();
    } catch (const UnsupportedOperationException &e) {
      // ok
    }
  }

  // otherwise: binary search
  shared_ptr<TermsEnum> iterator = this->iterator();
  shared_ptr<BytesRef> v = iterator->next();
  if (v == nullptr) {
    // empty: only possible from a FilteredTermsEnum...
    return v;
  }

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  scratch->append(static_cast<char>(0));

  // Iterates over digits:
  while (true) {

    int low = 0;
    int high = 256;

    // Binary search current digit to find the highest
    // digit before END:
    while (low != high) {
      int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
      scratch->setByteAt(scratch->length() - 1, static_cast<char>(mid));
      if (iterator->seekCeil(scratch->get()) == TermsEnum::SeekStatus::END) {
        // Scratch was too high
        if (mid == 0) {
          scratch->setLength(scratch->length() - 1);
          return scratch->get();
        }
        high = mid;
      } else {
        // Scratch was too low; there is at least one term
        // still after it:
        if (low == mid) {
          break;
        }
        low = mid;
      }
    }

    // Recurse to next digit:
    scratch->setLength(scratch->length() + 1);
    scratch->grow(scratch->length());
  }
}

any Terms::getStats() 
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"impl=" + getClass().getSimpleName());
  sb->append(L",size=" + to_wstring(size()));
  sb->append(L",docCount=" + to_wstring(getDocCount()));
  sb->append(L",sumTotalTermFreq=" + to_wstring(getSumTotalTermFreq()));
  sb->append(L",sumDocFreq=" + to_wstring(getSumDocFreq()));
  return sb->toString();
}
} // namespace org::apache::lucene::index