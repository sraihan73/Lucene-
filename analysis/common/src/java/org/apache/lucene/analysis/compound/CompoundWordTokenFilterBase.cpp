using namespace std;

#include "CompoundWordTokenFilterBase.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::compound
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

CompoundWordTokenFilterBase::CompoundWordTokenFilterBase(
    shared_ptr<TokenStream> input, shared_ptr<CharArraySet> dictionary,
    bool onlyLongestMatch)
    : CompoundWordTokenFilterBase(input, dictionary, DEFAULT_MIN_WORD_SIZE,
                                  DEFAULT_MIN_SUBWORD_SIZE,
                                  DEFAULT_MAX_SUBWORD_SIZE, onlyLongestMatch)
{
}

CompoundWordTokenFilterBase::CompoundWordTokenFilterBase(
    shared_ptr<TokenStream> input, shared_ptr<CharArraySet> dictionary)
    : CompoundWordTokenFilterBase(input, dictionary, DEFAULT_MIN_WORD_SIZE,
                                  DEFAULT_MIN_SUBWORD_SIZE,
                                  DEFAULT_MAX_SUBWORD_SIZE, false)
{
}

CompoundWordTokenFilterBase::CompoundWordTokenFilterBase(
    shared_ptr<TokenStream> input, shared_ptr<CharArraySet> dictionary,
    int minWordSize, int minSubwordSize, int maxSubwordSize,
    bool onlyLongestMatch)
    : org::apache::lucene::analysis::TokenFilter(input), dictionary(dictionary),
      tokens(deque<>())
{
  if (minWordSize < 0) {
    throw invalid_argument(L"minWordSize cannot be negative");
  }
  this->minWordSize = minWordSize;
  if (minSubwordSize < 0) {
    throw invalid_argument(L"minSubwordSize cannot be negative");
  }
  this->minSubwordSize = minSubwordSize;
  if (maxSubwordSize < 0) {
    throw invalid_argument(L"maxSubwordSize cannot be negative");
  }
  this->maxSubwordSize = maxSubwordSize;
  this->onlyLongestMatch = onlyLongestMatch;
}

bool CompoundWordTokenFilterBase::incrementToken() 
{
  if (!tokens.empty()) {
    assert(current != nullptr);
    shared_ptr<CompoundToken> token = tokens.pop_front();
    restoreState(current); // keep all other attributes untouched
    termAtt->setEmpty()->append(token->txt);
    offsetAtt->setOffset(token->startOffset, token->endOffset);
    posIncAtt->setPositionIncrement(0);
    return true;
  }

  current.reset(); // not really needed, but for safety
  if (input->incrementToken()) {
    // Only words longer than minWordSize get processed
    if (termAtt->length() >= this->minWordSize) {
      decompose();
      // only capture the state if we really need it for producing new tokens
      if (!tokens.empty()) {
        current = captureState();
      }
    }
    // return original token:
    return true;
  } else {
    return false;
  }
}

void CompoundWordTokenFilterBase::reset() 
{
  TokenFilter::reset();
  tokens.clear();
  current.reset();
}

CompoundWordTokenFilterBase::CompoundToken::CompoundToken(
    shared_ptr<CompoundWordTokenFilterBase> outerInstance, int offset,
    int length)
    : txt(outerInstance->termAtt->substr(offset, length)),
      startOffset(outerInstance->offsetAtt->startOffset()),
      endOffset(outerInstance->offsetAtt->endOffset()),
      outerInstance(outerInstance)
{

  // offsets of the original word
}
} // namespace org::apache::lucene::analysis::compound