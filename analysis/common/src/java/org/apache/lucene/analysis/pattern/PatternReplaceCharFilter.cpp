using namespace std;

#include "PatternReplaceCharFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using BaseCharFilter =
    org::apache::lucene::analysis::charfilter::BaseCharFilter;

PatternReplaceCharFilter::PatternReplaceCharFilter(shared_ptr<Pattern> pattern,
                                                   const wstring &replacement,
                                                   shared_ptr<Reader> in_)
    : org::apache::lucene::analysis::charfilter::BaseCharFilter(in_),
      pattern(pattern), replacement(replacement)
{
}

int PatternReplaceCharFilter::read(std::deque<wchar_t> &cbuf, int off,
                                   int len) 
{
  // Buffer all input on the first call.
  if (transformedInput == nullptr) {
    fill();
  }

  return transformedInput->read(cbuf, off, len);
}

void PatternReplaceCharFilter::fill() 
{
  shared_ptr<StringBuilder> buffered = make_shared<StringBuilder>();
  std::deque<wchar_t> temp(1024);
  for (int cnt = input->read(temp); cnt > 0; cnt = input->read(temp)) {
    buffered->append(temp, 0, cnt);
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  transformedInput =
      make_shared<StringReader>(processPattern(buffered)->toString());
}

int PatternReplaceCharFilter::read() 
{
  if (transformedInput == nullptr) {
    fill();
  }

  return transformedInput->read();
}

int PatternReplaceCharFilter::correct(int currentOff)
{
  return max(0, BaseCharFilter::correct(currentOff));
}

shared_ptr<std::wstring>
PatternReplaceCharFilter::processPattern(shared_ptr<std::wstring> input)
{
  shared_ptr<Matcher> *const m = pattern->matcher(input);

  shared_ptr<StringBuilder> *const cumulativeOutput =
      make_shared<StringBuilder>();
  int cumulative = 0;
  int lastMatchEnd = 0;
  while (m->find()) {
    constexpr int groupSize = m->end() - m->start();
    constexpr int skippedSize = m->start() - lastMatchEnd;
    lastMatchEnd = m->end();

    constexpr int lengthBeforeReplacement =
        cumulativeOutput->length() + skippedSize;
    m->appendReplacement(cumulativeOutput, replacement);
    // Matcher doesn't tell us how many characters have been appended before the
    // replacement. So we need to calculate it. Skipped characters have been
    // added as part of appendReplacement.
    constexpr int replacementSize =
        cumulativeOutput->length() - lengthBeforeReplacement;

    if (groupSize != replacementSize) {
      if (replacementSize < groupSize) {
        // The replacement is smaller.
        // Add the 'backskip' to the next index after the replacement (this is
        // possibly after the end of string, but it's fine -- it just means the
        // last character of the replaced block doesn't reach the end of the
        // original string.
        cumulative += groupSize - replacementSize;
        int atIndex = lengthBeforeReplacement + replacementSize;
        // System.err.println(atIndex + "!" + cumulative);
        addOffCorrectMap(atIndex, cumulative);
      } else {
        // The replacement is larger. Every new index needs to point to the last
        // element of the original group (if any).
        for (int i = groupSize; i < replacementSize; i++) {
          addOffCorrectMap(lengthBeforeReplacement + i, --cumulative);
          // System.err.println((lengthBeforeReplacement + i) + " " +
          // cumulative);
        }
      }
    }
  }

  // Append the remaining output, no further changes to indices.
  m->appendTail(cumulativeOutput);
  return cumulativeOutput;
}
} // namespace org::apache::lucene::analysis::pattern