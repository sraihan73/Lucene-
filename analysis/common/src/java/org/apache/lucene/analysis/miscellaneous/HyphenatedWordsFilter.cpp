using namespace std;

#include "HyphenatedWordsFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using namespace org::apache::lucene::analysis;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

HyphenatedWordsFilter::HyphenatedWordsFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool HyphenatedWordsFilter::incrementToken() 
{
  while (!exhausted && input->incrementToken()) {
    std::deque<wchar_t> term = termAttribute->buffer();
    int termLength = termAttribute->length();
    lastEndOffset = offsetAttribute->endOffset();

    if (termLength > 0 && term[termLength - 1] == L'-') {
      // a hyphenated word
      // capture the state of the first token only
      if (savedState == nullptr) {
        savedState = captureState();
      }
      hyphenated->append(term, 0, termLength - 1);
    } else if (savedState == nullptr) {
      // not part of a hyphenated word.
      return true;
    } else {
      // the final portion of a hyphenated word
      hyphenated->append(term, 0, termLength);
      unhyphenate();
      return true;
    }
  }

  exhausted = true;

  if (savedState != nullptr) {
    // the final term ends with a hyphen
    // add back the hyphen, for backwards compatibility.
    hyphenated->append(L'-');
    unhyphenate();
    return true;
  }

  return false;
}

void HyphenatedWordsFilter::reset() 
{
  TokenFilter::reset();
  hyphenated->setLength(0);
  savedState.reset();
  exhausted = false;
  lastEndOffset = 0;
}

void HyphenatedWordsFilter::unhyphenate()
{
  restoreState(savedState);
  savedState.reset();

  std::deque<wchar_t> term = termAttribute->buffer();
  int length = hyphenated->length();
  if (length > termAttribute->length()) {
    term = termAttribute->resizeBuffer(length);
  }

  hyphenated->getChars(0, length, term, 0);
  termAttribute->setLength(length);
  offsetAttribute->setOffset(offsetAttribute->startOffset(), lastEndOffset);
  hyphenated->setLength(0);
}
} // namespace org::apache::lucene::analysis::miscellaneous