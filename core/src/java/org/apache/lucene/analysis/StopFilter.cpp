using namespace std;

#include "StopFilter.h"
#include "CharArraySet.h"
#include "TokenStream.h"
#include "tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis
{
using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

StopFilter::StopFilter(shared_ptr<TokenStream> in_,
                       shared_ptr<CharArraySet> stopWords)
    : org::apache::lucene::analysis::FilteringTokenFilter(in_),
      stopWords(stopWords)
{
}

shared_ptr<CharArraySet> StopFilter::makeStopSet(deque<wstring> &stopWords)
{
  return makeStopSet(stopWords, false);
}

template <typename T1>
shared_ptr<CharArraySet> StopFilter::makeStopSet(deque<T1> stopWords)
{
  return makeStopSet(stopWords, false);
}

shared_ptr<CharArraySet>
StopFilter::makeStopSet(std::deque<wstring> &stopWords, bool ignoreCase)
{
  shared_ptr<CharArraySet> stopSet =
      make_shared<CharArraySet>(stopWords.size(), ignoreCase);
  stopSet->addAll(Arrays::asList(stopWords));
  return stopSet;
}

template <typename T1>
shared_ptr<CharArraySet> StopFilter::makeStopSet(deque<T1> stopWords,
                                                 bool ignoreCase)
{
  shared_ptr<CharArraySet> stopSet =
      make_shared<CharArraySet>(stopWords.size(), ignoreCase);
  stopSet->addAll(stopWords);
  return stopSet;
}

bool StopFilter::accept()
{
  return !stopWords->contains(termAtt->buffer(), 0, termAtt->length());
}
} // namespace org::apache::lucene::analysis