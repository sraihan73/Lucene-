using namespace std;

#include "MoreLikeThisQuery.h"

namespace org::apache::lucene::queries::mlt
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;

MoreLikeThisQuery::MoreLikeThisQuery(const wstring &likeText,
                                     std::deque<wstring> &moreLikeFields,
                                     shared_ptr<Analyzer> analyzer,
                                     const wstring &fieldName)
    : fieldName(Objects::requireNonNull(fieldName))
{
  this->likeText = Objects::requireNonNull(likeText);
  this->moreLikeFields = Objects::requireNonNull(moreLikeFields);
  this->analyzer = Objects::requireNonNull(analyzer);
}

shared_ptr<Query>
MoreLikeThisQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  shared_ptr<MoreLikeThis> mlt = make_shared<MoreLikeThis>(reader);

  mlt->setFieldNames(moreLikeFields);
  mlt->setAnalyzer(analyzer);
  mlt->setMinTermFreq(minTermFrequency);
  if (minDocFreq >= 0) {
    mlt->setMinDocFreq(minDocFreq);
  }
  mlt->setMaxQueryTerms(maxQueryTerms);
  mlt->setStopWords(stopWords);
  shared_ptr<BooleanQuery> bq = std::static_pointer_cast<BooleanQuery>(
      mlt->like(fieldName, make_shared<StringReader>(likeText)));
  shared_ptr<BooleanQuery::Builder> newBq =
      make_shared<BooleanQuery::Builder>();
  for (auto clause : bq) {
    newBq->add(clause);
  }
  // make at least half the terms match
  newBq->setMinimumNumberShouldMatch(
      static_cast<int>(bq->clauses().size() * percentTermsToMatch));
  return newBq->build();
}

wstring MoreLikeThisQuery::toString(const wstring &field)
{
  return L"like:" + likeText;
}

float MoreLikeThisQuery::getPercentTermsToMatch()
{
  return percentTermsToMatch;
}

void MoreLikeThisQuery::setPercentTermsToMatch(float percentTermsToMatch)
{
  this->percentTermsToMatch = percentTermsToMatch;
}

shared_ptr<Analyzer> MoreLikeThisQuery::getAnalyzer() { return analyzer; }

void MoreLikeThisQuery::setAnalyzer(shared_ptr<Analyzer> analyzer)
{
  this->analyzer = analyzer;
}

wstring MoreLikeThisQuery::getLikeText() { return likeText; }

void MoreLikeThisQuery::setLikeText(const wstring &likeText)
{
  this->likeText = likeText;
}

int MoreLikeThisQuery::getMaxQueryTerms() { return maxQueryTerms; }

void MoreLikeThisQuery::setMaxQueryTerms(int maxQueryTerms)
{
  this->maxQueryTerms = maxQueryTerms;
}

int MoreLikeThisQuery::getMinTermFrequency() { return minTermFrequency; }

void MoreLikeThisQuery::setMinTermFrequency(int minTermFrequency)
{
  this->minTermFrequency = minTermFrequency;
}

std::deque<wstring> MoreLikeThisQuery::getMoreLikeFields()
{
  return moreLikeFields;
}

void MoreLikeThisQuery::setMoreLikeFields(std::deque<wstring> &moreLikeFields)
{
  this->moreLikeFields = moreLikeFields;
}

// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public std::unordered_set<?> getStopWords()
shared_ptr < Set < ? >> MoreLikeThisQuery::getStopWords() { return stopWords; }

template <typename T1>
void MoreLikeThisQuery::setStopWords(shared_ptr<Set<T1>> stopWords)
{
  this->stopWords = stopWords;
}

int MoreLikeThisQuery::getMinDocFreq() { return minDocFreq; }

void MoreLikeThisQuery::setMinDocFreq(int minDocFreq)
{
  this->minDocFreq = minDocFreq;
}

int MoreLikeThisQuery::hashCode()
{
  constexpr int prime = 31;
  int result = classHash();
  result =
      prime * result + Objects::hash(analyzer, fieldName, likeText, stopWords);
  result = prime * result + maxQueryTerms;
  result = prime * result + minDocFreq;
  result = prime * result + minTermFrequency;
  result = prime * result + Arrays::hashCode(moreLikeFields);
  result = prime * result + Float::floatToIntBits(percentTermsToMatch);
  return result;
}

bool MoreLikeThisQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool MoreLikeThisQuery::equalsTo(shared_ptr<MoreLikeThisQuery> other)
{
  return maxQueryTerms == other->maxQueryTerms &&
         minDocFreq == other->minDocFreq &&
         minTermFrequency == other->minTermFrequency &&
         Float::floatToIntBits(percentTermsToMatch) ==
             Float::floatToIntBits(other->percentTermsToMatch) &&
         analyzer->equals(other->analyzer) && fieldName == other->fieldName &&
         likeText == other->likeText &&
         Arrays::equals(moreLikeFields, other->moreLikeFields) &&
         Objects::equals(stopWords, other->stopWords);
}
} // namespace org::apache::lucene::queries::mlt