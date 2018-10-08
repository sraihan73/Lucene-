using namespace std;

#include "TermRangeQuery.h"

namespace org::apache::lucene::search
{
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;

TermRangeQuery::TermRangeQuery(const wstring &field,
                               shared_ptr<BytesRef> lowerTerm,
                               shared_ptr<BytesRef> upperTerm,
                               bool includeLower, bool includeUpper)
    : AutomatonQuery(
          new Term(field, lowerTerm),
          toAutomaton(lowerTerm, upperTerm, includeLower, includeUpper),
          numeric_limits<int>::max(), true),
      lowerTerm(lowerTerm), upperTerm(upperTerm), includeLower(includeLower),
      includeUpper(includeUpper)
{
}

shared_ptr<Automaton>
TermRangeQuery::toAutomaton(shared_ptr<BytesRef> lowerTerm,
                            shared_ptr<BytesRef> upperTerm, bool includeLower,
                            bool includeUpper)
{

  if (lowerTerm == nullptr) {
    // makeBinaryInterval is more picky than we are:
    includeLower = true;
  }

  if (upperTerm == nullptr) {
    // makeBinaryInterval is more picky than we are:
    includeUpper = true;
  }

  return Automata::makeBinaryInterval(lowerTerm, includeLower, upperTerm,
                                      includeUpper);
}

shared_ptr<TermRangeQuery>
TermRangeQuery::newStringRange(const wstring &field, const wstring &lowerTerm,
                               const wstring &upperTerm, bool includeLower,
                               bool includeUpper)
{
  shared_ptr<BytesRef> lower =
      lowerTerm == L"" ? nullptr : make_shared<BytesRef>(lowerTerm);
  shared_ptr<BytesRef> upper =
      upperTerm == L"" ? nullptr : make_shared<BytesRef>(upperTerm);
  return make_shared<TermRangeQuery>(field, lower, upper, includeLower,
                                     includeUpper);
}

shared_ptr<BytesRef> TermRangeQuery::getLowerTerm() { return lowerTerm; }

shared_ptr<BytesRef> TermRangeQuery::getUpperTerm() { return upperTerm; }

bool TermRangeQuery::includesLower() { return includeLower; }

bool TermRangeQuery::includesUpper() { return includeUpper; }

wstring TermRangeQuery::toString(const wstring &field)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  if (getField() != field) {
    buffer->append(getField());
    buffer->append(L":");
  }
  buffer->append(includeLower ? L'[' : L'{');
  // TODO: all these toStrings for queries should just output the bytes, it
  // might not be UTF-8!
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(lowerTerm != nullptr ? (L"*" == Term::toString(lowerTerm)
                                             ? L"\\*"
                                             : Term::toString(lowerTerm))
                                      : L"*");
  buffer->append(L" TO ");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  buffer->append(upperTerm != nullptr ? (L"*" == Term::toString(upperTerm)
                                             ? L"\\*"
                                             : Term::toString(upperTerm))
                                      : L"*");
  buffer->append(includeUpper ? L']' : L'}');
  return buffer->toString();
}

int TermRangeQuery::hashCode()
{
  constexpr int prime = 31;
  int result = AutomatonQuery::hashCode();
  result = prime * result + (includeLower ? 1231 : 1237);
  result = prime * result + (includeUpper ? 1231 : 1237);
  result =
      prime * result + ((lowerTerm == nullptr) ? 0 : lowerTerm->hashCode());
  result =
      prime * result + ((upperTerm == nullptr) ? 0 : upperTerm->hashCode());
  return result;
}

bool TermRangeQuery::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!AutomatonQuery::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<TermRangeQuery> other =
      any_cast<std::shared_ptr<TermRangeQuery>>(obj);
  if (includeLower != other->includeLower) {
    return false;
  }
  if (includeUpper != other->includeUpper) {
    return false;
  }
  if (lowerTerm == nullptr) {
    if (other->lowerTerm != nullptr) {
      return false;
    }
  } else if (!lowerTerm->equals(other->lowerTerm)) {
    return false;
  }
  if (upperTerm == nullptr) {
    if (other->upperTerm != nullptr) {
      return false;
    }
  } else if (!upperTerm->equals(other->upperTerm)) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search