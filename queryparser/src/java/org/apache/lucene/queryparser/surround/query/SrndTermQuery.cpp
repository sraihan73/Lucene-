using namespace std;

#include "SrndTermQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Terms = org::apache::lucene::index::Terms;
using MultiFields = org::apache::lucene::index::MultiFields;
using BytesRef = org::apache::lucene::util::BytesRef;

SrndTermQuery::SrndTermQuery(const wstring &termText, bool quoted)
    : SimpleTerm(quoted), termText(termText)
{
}

wstring SrndTermQuery::getTermText() { return termText; }

shared_ptr<Term> SrndTermQuery::getLuceneTerm(const wstring &fieldName)
{
  return make_shared<Term>(fieldName, getTermText());
}

wstring SrndTermQuery::toStringUnquoted() { return getTermText(); }

void SrndTermQuery::visitMatchingTerms(
    shared_ptr<IndexReader> reader, const wstring &fieldName,
    shared_ptr<MatchingTermVisitor> mtv) 
{
  /* check term presence in index here for symmetry with other SimpleTerm's */
  shared_ptr<Terms> terms = MultiFields::getTerms(reader, fieldName);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    TermsEnum::SeekStatus status =
        termsEnum->seekCeil(make_shared<BytesRef>(getTermText()));
    if (status == TermsEnum::SeekStatus::FOUND) {
      mtv->visitMatchingTerm(getLuceneTerm(fieldName));
    }
  }
}
} // namespace org::apache::lucene::queryparser::surround::query