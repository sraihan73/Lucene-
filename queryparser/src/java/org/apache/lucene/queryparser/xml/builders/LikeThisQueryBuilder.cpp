using namespace std;

#include "LikeThisQueryBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using MoreLikeThisQuery = org::apache::lucene::queries::mlt::MoreLikeThisQuery;
using QueryBuilder = org::apache::lucene::queryparser::xml::QueryBuilder;
using BoostQuery = org::apache::lucene::search::BoostQuery;
using Query = org::apache::lucene::search::Query;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

LikeThisQueryBuilder::LikeThisQueryBuilder(
    shared_ptr<Analyzer> analyzer, std::deque<wstring> &defaultFieldNames)
    : analyzer(analyzer), defaultFieldNames(defaultFieldNames)
{
}

shared_ptr<Query>
LikeThisQueryBuilder::getQuery(shared_ptr<Element> e) 
{
  wstring fieldsList =
      e->getAttribute(L"fieldNames"); // a comma-delimited deque of fields
  std::deque<wstring> fields = defaultFieldNames;
  if ((fieldsList != L"") && (StringHelper::trim(fieldsList)->length() > 0)) {
    fields = StringHelper::trim(fieldsList)->split(L",");
    // trim the fieldnames
    for (int i = 0; i < fields.size(); i++) {
      fields[i] = StringHelper::trim(fields[i]);
    }
  }

  // Parse any "stopWords" attribute
  // TODO MoreLikeThis needs to ideally have per-field stopWords lists - until
  // then I use all analyzers/fields to generate multi-field compatible stop deque
  wstring stopWords = e->getAttribute(L"stopWords");
  shared_ptr<Set<wstring>> stopWordsSet = nullptr;
  if ((stopWords != L"") && (fields.size() > 0)) {
    stopWordsSet = unordered_set<>();
    for (auto field : fields) {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.analysis.TokenStream ts = analyzer.tokenStream(field,
      // stopWords))
      {
        org::apache::lucene::analysis::TokenStream ts =
            analyzer->tokenStream(field, stopWords);
        try {
          shared_ptr<CharTermAttribute> termAtt =
              ts->addAttribute(CharTermAttribute::typeid);
          ts->reset();
          while (ts->incrementToken()) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            stopWordsSet->add(termAtt->toString());
          }
          ts->end();
        } catch (const IOException &ioe) {
          throw make_shared<ParserException>(
              L"IoException parsing stop words deque in " + getClassName() +
              L":" + ioe->getLocalizedMessage());
        }
      }
    }
  }

  shared_ptr<MoreLikeThisQuery> mlt = make_shared<MoreLikeThisQuery>(
      DOMUtils::getText(e), fields, analyzer, fields[0]);
  mlt->setMaxQueryTerms(
      DOMUtils::getAttribute(e, L"maxQueryTerms", DEFAULT_MAX_QUERY_TERMS));
  mlt->setMinTermFrequency(DOMUtils::getAttribute(e, L"minTermFrequency",
                                                  DEFAULT_MIN_TERM_FREQUENCY));
  mlt->setPercentTermsToMatch(
      DOMUtils::getAttribute(e, L"percentTermsToMatch",
                             DEFAULT_PERCENT_TERMS_TO_MATCH) /
      100);
  mlt->setStopWords(stopWordsSet);
  int minDocFreq = DOMUtils::getAttribute(e, L"minDocFreq", -1);
  if (minDocFreq >= 0) {
    mlt->setMinDocFreq(minDocFreq);
  }

  shared_ptr<Query> q = mlt;
  float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
  if (boost != 1.0f) {
    q = make_shared<BoostQuery>(mlt, boost);
  }
  return q;
}
} // namespace org::apache::lucene::queryparser::xml::builders