using namespace std;

#include "SortedSetDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

SortedSetDocValues::SortedSetDocValues() {}

int64_t
SortedSetDocValues::lookupTerm(shared_ptr<BytesRef> key) 
{
  int64_t low = 0;
  int64_t high = getValueCount() - 1;

  while (low <= high) {
    int64_t mid = static_cast<int64_t>(
        static_cast<uint64_t>((low + high)) >> 1);
    shared_ptr<BytesRef> *const term = lookupOrd(mid);
    int cmp = term->compareTo(key);

    if (cmp < 0) {
      low = mid + 1;
    } else if (cmp > 0) {
      high = mid - 1;
    } else {
      return mid; // key found
    }
  }

  return -(low + 1); // key not found.
}

shared_ptr<TermsEnum> SortedSetDocValues::termsEnum() 
{
  return make_shared<SortedSetDocValuesTermsEnum>(shared_from_this());
}

shared_ptr<TermsEnum> SortedSetDocValues::intersect(
    shared_ptr<CompiledAutomaton> automaton) 
{
  shared_ptr<TermsEnum> in_ = termsEnum();
  switch (automaton->type) {
  case CompiledAutomaton::AUTOMATON_TYPE::NONE:
    return TermsEnum::EMPTY;
  case CompiledAutomaton::AUTOMATON_TYPE::ALL:
    return in_;
  case CompiledAutomaton::AUTOMATON_TYPE::SINGLE:
    return make_shared<SingleTermsEnum>(in_, automaton->term);
  case CompiledAutomaton::AUTOMATON_TYPE::NORMAL:
    return make_shared<AutomatonTermsEnum>(in_, automaton);
  default:
    // unreachable
    throw runtime_error(L"unhandled case");
  }
}
} // namespace org::apache::lucene::index