using namespace std;

#include "HunspellStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "Dictionary.h"
#include "Stemmer.h"

namespace org::apache::lucene::analysis::hunspell
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRef = org::apache::lucene::util::CharsRef;

HunspellStemFilter::HunspellStemFilter(shared_ptr<TokenStream> input,
                                       shared_ptr<Dictionary> dictionary)
    : HunspellStemFilter(input, dictionary, true)
{
}

HunspellStemFilter::HunspellStemFilter(shared_ptr<TokenStream> input,
                                       shared_ptr<Dictionary> dictionary,
                                       bool dedup)
    : HunspellStemFilter(input, dictionary, dedup, false)
{
}

HunspellStemFilter::HunspellStemFilter(shared_ptr<TokenStream> input,
                                       shared_ptr<Dictionary> dictionary,
                                       bool dedup, bool longestOnly)
    : org::apache::lucene::analysis::TokenFilter(input),
      stemmer(make_shared<Stemmer>(dictionary)),
      dedup(dedup && longestOnly == false) / *don't waste time deduping if longestOnly is set */, longestOnly(longestOnly)
{
}

bool HunspellStemFilter::incrementToken() 
{
  if (buffer.size() > 0 && !buffer.empty()) {
    shared_ptr<CharsRef> nextStem = buffer.erase(buffer.begin());
    restoreState(savedState);
    posIncAtt->setPositionIncrement(0);
    termAtt->setEmpty()->append(nextStem);
    return true;
  }

  if (!input->incrementToken()) {
    return false;
  }

  if (keywordAtt->isKeyword()) {
    return true;
  }

  buffer = dedup ? stemmer->uniqueStems(termAtt->buffer(), termAtt->length())
                 : stemmer->stem(termAtt->buffer(), termAtt->length());

  if (buffer.empty()) { // we do not know this word, return it unchanged
    return true;
  }

  if (longestOnly && buffer.size() > 1) {
    // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
    // while the Java Comparator parameter produces a tri-state result: ORIGINAL
    // LINE: java.util.Collections.sort(buffer, lengthComparator);
    sort(buffer.begin(), buffer.end(), lengthComparator);
  }

  shared_ptr<CharsRef> stem = buffer.erase(buffer.begin());
  termAtt->setEmpty()->append(stem);

  if (longestOnly) {
    buffer.clear();
  } else {
    if (!buffer.empty()) {
      savedState = captureState();
    }
  }

  return true;
}

void HunspellStemFilter::reset() 
{
  TokenFilter::reset();
  buffer.clear();
}

const shared_ptr<java::util::Comparator<
    std::shared_ptr<org::apache::lucene::util::CharsRef>>>
    HunspellStemFilter::lengthComparator =
        make_shared<ComparatorAnonymousInnerClass>();

HunspellStemFilter::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass()
{
}

int HunspellStemFilter::ComparatorAnonymousInnerClass::compare(
    shared_ptr<CharsRef> o1, shared_ptr<CharsRef> o2)
{
  int cmp = Integer::compare(o2->length_, o1->length_);
  if (cmp == 0) {
    // tie break on text
    return o2->compareTo(o1);
  } else {
    return cmp;
  }
}
} // namespace org::apache::lucene::analysis::hunspell