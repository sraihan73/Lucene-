using namespace std;

#include "FingerprintFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

namespace org::apache::lucene::analysis::miscellaneous
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
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

FingerprintFilter::FingerprintFilter(shared_ptr<TokenStream> input)
    : FingerprintFilter(input, DEFAULT_MAX_OUTPUT_TOKEN_SIZE, DEFAULT_SEPARATOR)
{
}

FingerprintFilter::FingerprintFilter(shared_ptr<TokenStream> input,
                                     int maxOutputTokenSize, wchar_t separator)
    : org::apache::lucene::analysis::TokenFilter(input),
      maxOutputTokenSize(maxOutputTokenSize), separator(separator)
{
}

bool FingerprintFilter::incrementToken() 
{
  if (inputEnded) {
    return false;
  }
  bool result = buildSingleOutputToken();
  finalState = captureState();
  return result;
}

bool FingerprintFilter::buildSingleOutputToken() 
{
  inputEnded = false;

  std::deque<wchar_t> clonedLastTerm;
  uniqueTerms = make_shared<CharArraySet>(8, false);
  int outputTokenSize = 0;
  while (input->incrementToken()) {
    if (outputTokenSize > maxOutputTokenSize) {
      continue;
    }

    const std::deque<wchar_t> term = termAttribute->buffer();
    constexpr int length = termAttribute->length();

    if (!uniqueTerms->contains(term, 0, length)) {
      // clone the term, and add to the set of seen terms.
      clonedLastTerm = std::deque<wchar_t>(length);
      System::arraycopy(term, 0, clonedLastTerm, 0, length);
      if (uniqueTerms->size() > 0) {
        outputTokenSize++; // Add 1 for the separator char we will output
      }
      uniqueTerms->add(clonedLastTerm);
      outputTokenSize += length;
    }
  }
  // Force end-of-stream operations to get the final state.
  input->end();
  inputEnded = true;

  // Gathering complete - now output exactly zero or one token:

  // Set the attributes for the single output token
  offsetAtt->setOffset(0, offsetAtt->endOffset());
  posLenAtt->setPositionLength(1);
  posIncrAtt->setPositionIncrement(1);
  typeAtt->setType(L"fingerprint");

  // No tokens gathered - no output
  if (uniqueTerms->size() < 1) {
    termAttribute->setEmpty();
    return false;
  }

  // Tokens gathered are too large - no output
  if (outputTokenSize > maxOutputTokenSize) {
    termAttribute->setEmpty();
    uniqueTerms->clear();
    return false;
  }

  // Special case - faster option when we have a single token
  if (uniqueTerms->size() == 1) {
    termAttribute->setEmpty()->append(wstring(clonedLastTerm));
    uniqueTerms->clear();
    return true;
  }

  // Sort the set of deduplicated tokens and combine
  std::deque<any> items = uniqueTerms->toArray();

  Arrays::sort(items,
               make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  // TODO lets append directly to termAttribute?
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (auto item : items) {
    if (sb->length() >= 1) {
      sb->append(separator);
    }
    sb->append(static_cast<std::deque<wchar_t>>(item));
  }
  termAttribute->setEmpty()->append(sb);
  uniqueTerms->clear();
  return true;
}

FingerprintFilter::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<FingerprintFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

int FingerprintFilter::ComparatorAnonymousInnerClass::compare(any o1, any o2)
{
  std::deque<wchar_t> v1 = any_cast<std::deque<wchar_t>>(o1);
  std::deque<wchar_t> v2 = any_cast<std::deque<wchar_t>>(o2);
  int len1 = v1.size();
  int len2 = v2.size();
  int lim = min(len1, len2);

  int k = 0;
  while (k < lim) {
    wchar_t c1 = v1[k];
    wchar_t c2 = v2[k];
    if (c1 != c2) {
      return c1 - c2;
    }
    k++;
  }
  return len1 - len2;
}

void FingerprintFilter::end() 
{
  if (!inputEnded) {
    // Rare case - If an IOException occurs while performing
    // buildSingleOutputToken we may not have called input.end() already
    input->end();
    inputEnded = true;
  }

  if (finalState != nullptr) {
    restoreState(finalState);
  }
}

void FingerprintFilter::reset() 
{
  TokenFilter::reset();
  inputEnded = false;
  uniqueTerms.reset();
}
} // namespace org::apache::lucene::analysis::miscellaneous