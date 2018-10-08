using namespace std;

#include "TermVectorFilteredLeafReader.h"

namespace org::apache::lucene::search::uhighlight
{
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

TermVectorFilteredLeafReader::TermVectorFilteredLeafReader(
    shared_ptr<LeafReader> baseLeafReader, shared_ptr<Terms> filterTerms)
    : org::apache::lucene::index::FilterLeafReader(baseLeafReader),
      filterTerms(filterTerms)
{
}

shared_ptr<Terms>
TermVectorFilteredLeafReader::terms(const wstring &field) 
{
  shared_ptr<Terms> terms = in_->terms(field);
  return terms == nullptr ? nullptr
                          : make_shared<TermsFilteredTerms>(terms, filterTerms);
}

TermVectorFilteredLeafReader::TermsFilteredTerms::TermsFilteredTerms(
    shared_ptr<Terms> baseTerms, shared_ptr<Terms> filterTerms)
    : org::apache::lucene::index::FilterLeafReader::FilterTerms(baseTerms),
      filterTerms(filterTerms)
{
}

shared_ptr<TermsEnum>
TermVectorFilteredLeafReader::TermsFilteredTerms::iterator() 
{
  return make_shared<TermVectorFilteredTermsEnum>(in_->begin(),
                                                  filterTerms->begin());
}

shared_ptr<TermsEnum>
TermVectorFilteredLeafReader::TermsFilteredTerms::intersect(
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
{
  return make_shared<TermVectorFilteredTermsEnum>(
      in_->begin(), filterTerms->intersect(compiled, startTerm));
}

TermVectorFilteredLeafReader::TermVectorFilteredTermsEnum::
    TermVectorFilteredTermsEnum(shared_ptr<TermsEnum> baseTermsEnum,
                                shared_ptr<TermsEnum> filteredTermsEnum)
    : org::apache::lucene::index::FilterLeafReader::FilterTermsEnum(
          filteredTermsEnum),
      baseTermsEnum(baseTermsEnum)
{
}

shared_ptr<PostingsEnum>
TermVectorFilteredLeafReader::TermVectorFilteredTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  moveToCurrentTerm();
  return baseTermsEnum->postings(reuse, flags);
}

void TermVectorFilteredLeafReader::TermVectorFilteredTermsEnum::
    moveToCurrentTerm() 
{
  shared_ptr<BytesRef> currentTerm = in_->term(); // from filteredTermsEnum
  bool termInBothTermsEnum = baseTermsEnum->seekExact(currentTerm);

  if (!termInBothTermsEnum) {
    throw make_shared<IllegalStateException>(
        L"Term deque term " + currentTerm +
        L" does not appear in full index.");
  }
}

shared_ptr<CacheHelper> TermVectorFilteredLeafReader::getCoreCacheHelper()
{
  return nullptr;
}

shared_ptr<CacheHelper> TermVectorFilteredLeafReader::getReaderCacheHelper()
{
  return nullptr;
}
} // namespace org::apache::lucene::search::uhighlight