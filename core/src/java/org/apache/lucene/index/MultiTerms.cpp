using namespace std;

#include "MultiTerms.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

MultiTerms::MultiTerms(
    std::deque<std::shared_ptr<Terms>> &subs,
    std::deque<std::shared_ptr<ReaderSlice>> &subSlices) 
    : subs(subs), subSlices(subSlices), hasFreqs(_hasFreqs),
      hasOffsets(_hasOffsets), hasPositions(_hasPositions),
      hasPayloads(hasPositions_ && _hasPayloads) / *if all subs have pos,
      and at least one has payloads.* /
{

  assert((subs.size() > 0, L"inefficient: don't use MultiTerms over one sub"));
  bool _hasFreqs = true;
  bool _hasOffsets = true;
  bool _hasPositions = true;
  bool _hasPayloads = false;
  for (int i = 0; i < subs.size(); i++) {
    _hasFreqs &= subs[i]->hasFreqs();
    _hasOffsets &= subs[i]->hasOffsets();
    _hasPositions &= subs[i]->hasPositions();
    _hasPayloads |= subs[i]->hasPayloads();
  }
}

std::deque<std::shared_ptr<Terms>> MultiTerms::getSubTerms() { return subs; }

std::deque<std::shared_ptr<ReaderSlice>> MultiTerms::getSubSlices()
{
  return subSlices;
}

shared_ptr<TermsEnum>
MultiTerms::intersect(shared_ptr<CompiledAutomaton> compiled,
                      shared_ptr<BytesRef> startTerm) 
{
  const deque<std::shared_ptr<MultiTermsEnum::TermsEnumIndex>> termsEnums =
      deque<std::shared_ptr<MultiTermsEnum::TermsEnumIndex>>();
  for (int i = 0; i < subs.size(); i++) {
    shared_ptr<TermsEnum> *const termsEnum =
        subs[i]->intersect(compiled, startTerm);
    if (termsEnum != nullptr) {
      termsEnums.push_back(
          make_shared<MultiTermsEnum::TermsEnumIndex>(termsEnum, i));
    }
  }

  if (termsEnums.size() > 0) {
    return (make_shared<MultiTermsEnum>(subSlices))
        ->reset(
            termsEnums.toArray(MultiTermsEnum::TermsEnumIndex::EMPTY_ARRAY));
  } else {
    return TermsEnum::EMPTY;
  }
}

shared_ptr<BytesRef> MultiTerms::getMin() 
{
  shared_ptr<BytesRef> minTerm = nullptr;
  for (auto terms : subs) {
    shared_ptr<BytesRef> term = terms->getMin();
    if (minTerm == nullptr || term->compareTo(minTerm) < 0) {
      minTerm = term;
    }
  }

  return minTerm;
}

shared_ptr<BytesRef> MultiTerms::getMax() 
{
  shared_ptr<BytesRef> maxTerm = nullptr;
  for (auto terms : subs) {
    shared_ptr<BytesRef> term = terms->getMax();
    if (maxTerm == nullptr || term->compareTo(maxTerm) > 0) {
      maxTerm = term;
    }
  }

  return maxTerm;
}

shared_ptr<TermsEnum> MultiTerms::iterator() 
{

  const deque<std::shared_ptr<MultiTermsEnum::TermsEnumIndex>> termsEnums =
      deque<std::shared_ptr<MultiTermsEnum::TermsEnumIndex>>();
  for (int i = 0; i < subs.size(); i++) {
    shared_ptr<TermsEnum> *const termsEnum = subs[i]->begin();
    if (termsEnum != nullptr) {
      termsEnums.push_back(
          make_shared<MultiTermsEnum::TermsEnumIndex>(termsEnum, i));
    }
  }

  if (termsEnums.size() > 0) {
    return (make_shared<MultiTermsEnum>(subSlices))
        ->reset(
            termsEnums.toArray(MultiTermsEnum::TermsEnumIndex::EMPTY_ARRAY));
  } else {
    return TermsEnum::EMPTY;
  }
}

int64_t MultiTerms::size() { return -1; }

int64_t MultiTerms::getSumTotalTermFreq() 
{
  int64_t sum = 0;
  for (auto terms : subs) {
    constexpr int64_t v = terms->getSumTotalTermFreq();
    if (v == -1) {
      return -1;
    }
    sum += v;
  }
  return sum;
}

int64_t MultiTerms::getSumDocFreq() 
{
  int64_t sum = 0;
  for (auto terms : subs) {
    constexpr int64_t v = terms->getSumDocFreq();
    if (v == -1) {
      return -1;
    }
    sum += v;
  }
  return sum;
}

int MultiTerms::getDocCount() 
{
  int sum = 0;
  for (auto terms : subs) {
    constexpr int v = terms->getDocCount();
    if (v == -1) {
      return -1;
    }
    sum += v;
  }
  return sum;
}

bool MultiTerms::hasFreqs() { return hasFreqs_; }

bool MultiTerms::hasOffsets() { return hasOffsets_; }

bool MultiTerms::hasPositions() { return hasPositions_; }

bool MultiTerms::hasPayloads() { return hasPayloads_; }
} // namespace org::apache::lucene::index