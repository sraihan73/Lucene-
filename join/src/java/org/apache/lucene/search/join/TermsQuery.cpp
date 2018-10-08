using namespace std;

#include "TermsQuery.h"

namespace org::apache::lucene::search::join
{
using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

TermsQuery::TermsQuery(const wstring &toField, shared_ptr<BytesRefHash> terms,
                       const wstring &fromField, shared_ptr<Query> fromQuery,
                       any indexReaderContextId)
    : org::apache::lucene::search::MultiTermQuery(toField), terms(terms),
      ords(terms->sort()), fromField(fromField), fromQuery(fromQuery),
      indexReaderContextId(indexReaderContextId)
{
}

shared_ptr<TermsEnum>
TermsQuery::getTermsEnum(shared_ptr<Terms> terms,
                         shared_ptr<AttributeSource> atts) 
{
  if (this->terms->size() == 0) {
    return TermsEnum::EMPTY;
  }

  return make_shared<SeekingTermSetTermsEnum>(terms->begin(), this->terms,
                                              ords);
}

wstring TermsQuery::toString(const wstring &string)
{
  return wstring(L"TermsQuery{") + L"field=" + field +
         // C++ TODO: There is no native C++ equivalent to 'toString':
         L"fromQuery=" + fromQuery->toString(field) + L'}';
}

bool TermsQuery::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (!MultiTermQuery::equals(obj)) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }

  shared_ptr<TermsQuery> other = any_cast<std::shared_ptr<TermsQuery>>(obj);
  return Objects::equals(field, other->field) &&
         Objects::equals(fromField, other->fromField) &&
         Objects::equals(fromQuery, other->fromQuery) &&
         Objects::equals(indexReaderContextId, other->indexReaderContextId);
}

int TermsQuery::hashCode()
{
  return classHash() +
         Objects::hash(field, fromField, fromQuery, indexReaderContextId);
}

TermsQuery::SeekingTermSetTermsEnum::SeekingTermSetTermsEnum(
    shared_ptr<TermsEnum> tenum, shared_ptr<BytesRefHash> terms,
    std::deque<int> &ords)
    : org::apache::lucene::index::FilteredTermsEnum(tenum), terms(terms),
      ords(ords), lastElement(terms->size() - 1),
      lastTerm(terms->get(ords[lastElement], make_shared<BytesRef>()))
{
  seekTerm = terms->get(ords[upto], spare);
}

shared_ptr<BytesRef> TermsQuery::SeekingTermSetTermsEnum::nextSeekTerm(
    shared_ptr<BytesRef> currentTerm) 
{
  shared_ptr<BytesRef> temp = seekTerm;
  seekTerm.reset();
  return temp;
}

AcceptStatus TermsQuery::SeekingTermSetTermsEnum::accept(
    shared_ptr<BytesRef> term) 
{
  if (term->compareTo(lastTerm) > 0) {
    return AcceptStatus::END;
  }

  shared_ptr<BytesRef> currentTerm = terms->get(ords[upto], spare);
  if (term->compareTo(currentTerm) == 0) {
    if (upto == lastElement) {
      return AcceptStatus::YES;
    } else {
      seekTerm = terms->get(ords[++upto], spare);
      return AcceptStatus::YES_AND_SEEK;
    }
  } else {
    if (upto == lastElement) {
      return AcceptStatus::NO;
    } else { // Our current term doesn't match the the given term.
      int cmp;
      do { // We maybe are behind the given term by more than one step. Keep
           // incrementing till we're the same or higher.
        if (upto == lastElement) {
          return AcceptStatus::NO;
        }
        // typically the terms dict is a superset of query's terms so it's
        // unusual that we have to skip many of our terms so we don't do a
        // binary search here
        seekTerm = terms->get(ords[++upto], spare);
      } while ((cmp = seekTerm->compareTo(term)) < 0);
      if (cmp == 0) {
        if (upto == lastElement) {
          return AcceptStatus::YES;
        }
        seekTerm = terms->get(ords[++upto], spare);
        return AcceptStatus::YES_AND_SEEK;
      } else {
        return AcceptStatus::NO_AND_SEEK;
      }
    }
  }
}
} // namespace org::apache::lucene::search::join