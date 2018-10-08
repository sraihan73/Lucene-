using namespace std;

#include "SortedDocValues.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

SortedDocValues::SortedDocValues() {}

shared_ptr<BytesRef> SortedDocValues::binaryValue() 
{
  int ord = ordValue();
  if (ord == -1) {
    return empty;
  } else {
    return lookupOrd(ord);
  }
}

int SortedDocValues::lookupTerm(shared_ptr<BytesRef> key) 
{
  int low = 0;
  int high = getValueCount() - 1;

  while (low <= high) {
    int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
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

shared_ptr<TermsEnum> SortedDocValues::termsEnum() 
{
  return make_shared<SortedDocValuesTermsEnum>(shared_from_this());
}

shared_ptr<TermsEnum> SortedDocValues::intersect(
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