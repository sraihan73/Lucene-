using namespace std;

#include "PatternCaptureGroupTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

PatternCaptureGroupTokenFilter::PatternCaptureGroupTokenFilter(
    shared_ptr<TokenStream> input, bool preserveOriginal,
    deque<Pattern> &patterns)
    : org::apache::lucene::analysis::TokenFilter(input),
      matchers(std::deque<std::shared_ptr<Matcher>>(patterns->length)),
      groupCounts(std::deque<int>(patterns->length)),
      preserveOriginal(preserveOriginal)
{
  this->currentGroup = std::deque<int>(patterns->length);
  for (int i = 0; i < patterns->length; i++) {
    this->matchers[i] = patterns[i].matcher(L"");
    this->groupCounts[i] = this->matchers[i]->groupCount();
    this->currentGroup[i] = -1;
  }
}

bool PatternCaptureGroupTokenFilter::nextCapture()
{
  int min_offset = numeric_limits<int>::max();
  currentMatcher = -1;
  shared_ptr<Matcher> matcher;

  for (int i = 0; i < matchers.size(); i++) {
    matcher = matchers[i];
    if (currentGroup[i] == -1) {
      currentGroup[i] = matcher->find() ? 1 : 0;
    }
    if (currentGroup[i] != 0) {
      while (currentGroup[i] < groupCounts[i] + 1) {
        constexpr int start = matcher->start(currentGroup[i]);
        constexpr int end = matcher->end(currentGroup[i]);
        if (start == end ||
            preserveOriginal && start == 0 && spare->length() == end) {
          currentGroup[i]++;
          continue;
        }
        if (start < min_offset) {
          min_offset = start;
          currentMatcher = i;
        }
        break;
      }
      if (currentGroup[i] == groupCounts[i] + 1) {
        currentGroup[i] = -1;
        i--;
      }
    }
  }
  return currentMatcher != -1;
}

bool PatternCaptureGroupTokenFilter::incrementToken() 
{

  if (currentMatcher != -1 && nextCapture()) {
    assert(state != nullptr);
    clearAttributes();
    restoreState(state);
    constexpr int start =
        matchers[currentMatcher]->start(currentGroup[currentMatcher]);
    constexpr int end =
        matchers[currentMatcher]->end(currentGroup[currentMatcher]);

    posAttr->setPositionIncrement(0);
    charTermAttr->copyBuffer(spare->chars(), start, end - start);
    currentGroup[currentMatcher]++;
    return true;
  }

  if (!input->incrementToken()) {
    return false;
  }

  std::deque<wchar_t> buffer = charTermAttr->buffer();
  int length = charTermAttr->length();
  spare->copyChars(buffer, 0, length);
  state = captureState();

  for (int i = 0; i < matchers.size(); i++) {
    matchers[i]->reset(spare->get());
    currentGroup[i] = -1;
  }

  if (preserveOriginal) {
    currentMatcher = 0;
  } else if (nextCapture()) {
    constexpr int start =
        matchers[currentMatcher]->start(currentGroup[currentMatcher]);
    constexpr int end =
        matchers[currentMatcher]->end(currentGroup[currentMatcher]);

    // if we start at 0 we can simply set the length and save the copy
    if (start == 0) {
      charTermAttr->setLength(end);
    } else {
      charTermAttr->copyBuffer(spare->chars(), start, end - start);
    }
    currentGroup[currentMatcher]++;
  }
  return true;
}

void PatternCaptureGroupTokenFilter::reset() 
{
  TokenFilter::reset();
  state.reset();
  currentMatcher = -1;
}
} // namespace org::apache::lucene::analysis::pattern