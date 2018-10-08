using namespace std;

#include "SrndTruncQuery.h"

namespace org::apache::lucene::queryparser::surround::query
{
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Terms = org::apache::lucene::index::Terms;
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;

SrndTruncQuery::SrndTruncQuery(const wstring &truncated, wchar_t unlimited,
                               wchar_t mask)
    : SimpleTerm(false), truncated(truncated), unlimited(unlimited), mask(mask)
{
  truncatedToPrefixAndPattern();
}

wstring SrndTruncQuery::getTruncated() { return truncated; }

wstring SrndTruncQuery::toStringUnquoted() { return getTruncated(); }

bool SrndTruncQuery::matchingChar(wchar_t c)
{
  return (c != unlimited) && (c != mask);
}

void SrndTruncQuery::appendRegExpForChar(wchar_t c,
                                         shared_ptr<StringBuilder> re)
{
  if (c == unlimited) {
    re->append(L".*");
  } else if (c == mask) {
    re->append(L".");
  } else {
    re->append(c);
  }
}

void SrndTruncQuery::truncatedToPrefixAndPattern()
{
  int i = 0;
  while ((i < truncated.length()) && matchingChar(truncated[i])) {
    i++;
  }
  prefix = truncated.substr(0, i);
  prefixRef = make_shared<BytesRef>(prefix);

  shared_ptr<StringBuilder> re = make_shared<StringBuilder>();
  while (i < truncated.length()) {
    appendRegExpForChar(truncated[i], re);
    i++;
  }
  pattern = Pattern::compile(re->toString());
}

void SrndTruncQuery::visitMatchingTerms(
    shared_ptr<IndexReader> reader, const wstring &fieldName,
    shared_ptr<MatchingTermVisitor> mtv) 
{
  int prefixLength = prefix.length();
  shared_ptr<Terms> terms = MultiFields::getTerms(reader, fieldName);
  if (terms != nullptr) {
    shared_ptr<Matcher> matcher = pattern->matcher(L"");
    try {
      shared_ptr<TermsEnum> termsEnum = terms->begin();

      TermsEnum::SeekStatus status = termsEnum->seekCeil(prefixRef);
      shared_ptr<BytesRef> text;
      if (status == TermsEnum::SeekStatus::FOUND) {
        text = prefixRef;
      } else if (status == TermsEnum::SeekStatus::NOT_FOUND) {
        text = termsEnum->term();
      } else {
        text.reset();
      }

      while (text != nullptr) {
        if (text != nullptr && StringHelper::startsWith(text, prefixRef)) {
          wstring textString = text->utf8ToString();
          matcher->reset(textString.substr(prefixLength));
          if (matcher->matches()) {
            mtv->visitMatchingTerm(make_shared<Term>(fieldName, textString));
          }
        } else {
          break;
        }
        text = termsEnum->next();
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      matcher->reset();
    }
  }
}
} // namespace org::apache::lucene::queryparser::surround::query