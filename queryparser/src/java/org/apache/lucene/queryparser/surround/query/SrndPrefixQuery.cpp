using namespace std;

#include "SrndPrefixQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;

SrndPrefixQuery::SrndPrefixQuery(const wstring &prefix, bool quoted,
                                 wchar_t truncator)
    : SimpleTerm(quoted), prefixRef(make_shared<BytesRef>(prefix)),
      prefix(prefix), truncator(truncator)
{
}

wstring SrndPrefixQuery::getPrefix() { return prefix; }

wchar_t SrndPrefixQuery::getSuffixOperator() { return truncator; }

shared_ptr<Term> SrndPrefixQuery::getLucenePrefixTerm(const wstring &fieldName)
{
  return make_shared<Term>(fieldName, getPrefix());
}

wstring SrndPrefixQuery::toStringUnquoted() { return getPrefix(); }

void SrndPrefixQuery::suffixToString(shared_ptr<StringBuilder> r)
{
  r->append(getSuffixOperator());
}

void SrndPrefixQuery::visitMatchingTerms(
    shared_ptr<IndexReader> reader, const wstring &fieldName,
    shared_ptr<MatchingTermVisitor> mtv) 
{
  /* inspired by PrefixQuery.rewrite(): */
  shared_ptr<Terms> terms = MultiFields::getTerms(reader, fieldName);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> termsEnum = terms->begin();

    bool skip = false;
    TermsEnum::SeekStatus status =
        termsEnum->seekCeil(make_shared<BytesRef>(getPrefix()));
    if (status == TermsEnum::SeekStatus::FOUND) {
      mtv->visitMatchingTerm(getLucenePrefixTerm(fieldName));
    } else if (status == TermsEnum::SeekStatus::NOT_FOUND) {
      if (StringHelper::startsWith(termsEnum->term(), prefixRef)) {
        mtv->visitMatchingTerm(
            make_shared<Term>(fieldName, termsEnum->term()->utf8ToString()));
      } else {
        skip = true;
      }
    } else {
      // EOF
      skip = true;
    }

    if (!skip) {
      while (true) {
        shared_ptr<BytesRef> text = termsEnum->next();
        if (text != nullptr && StringHelper::startsWith(text, prefixRef)) {
          mtv->visitMatchingTerm(
              make_shared<Term>(fieldName, text->utf8ToString()));
        } else {
          break;
        }
      }
    }
  }
}
} // namespace org::apache::lucene::queryparser::surround::query