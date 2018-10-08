using namespace std;

#include "CapitalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

CapitalizationFilter::CapitalizationFilter(shared_ptr<TokenStream> in_)
    : CapitalizationFilter(in_, true, nullptr, true, nullptr, 0,
                           DEFAULT_MAX_WORD_COUNT, DEFAULT_MAX_TOKEN_LENGTH)
{
}

CapitalizationFilter::CapitalizationFilter(
    shared_ptr<TokenStream> in_, bool onlyFirstWord,
    shared_ptr<CharArraySet> keep, bool forceFirstLetter,
    shared_ptr<deque<std::deque<wchar_t>>> okPrefix, int minWordLength,
    int maxWordCount, int maxTokenLength)
    : org::apache::lucene::analysis::TokenFilter(in_),
      onlyFirstWord(onlyFirstWord), keep(keep),
      forceFirstLetter(forceFirstLetter), okPrefix(okPrefix),
      minWordLength(minWordLength), maxWordCount(maxWordCount),
      maxTokenLength(maxTokenLength)
{
  if (minWordLength < 0) {
    throw invalid_argument(
        L"minWordLength must be greater than or equal to zero");
  }
  if (maxWordCount < 1) {
    throw invalid_argument(L"maxWordCount must be greater than zero");
  }
  if (maxTokenLength < 1) {
    throw invalid_argument(L"maxTokenLength must be greater than zero");
  }
}

bool CapitalizationFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  std::deque<wchar_t> termBuffer = termAtt->buffer();
  int termBufferLength = termAtt->length();
  std::deque<wchar_t> backup;

  if (maxWordCount < DEFAULT_MAX_WORD_COUNT) {
    // make a backup in case we exceed the word count
    backup = std::deque<wchar_t>(termBufferLength);
    System::arraycopy(termBuffer, 0, backup, 0, termBufferLength);
  }

  if (termBufferLength < maxTokenLength) {
    int wordCount = 0;

    int lastWordStart = 0;
    for (int i = 0; i < termBufferLength; i++) {
      wchar_t c = termBuffer[i];
      if (c <= L' ' || c == L'.') {
        int len = i - lastWordStart;
        if (len > 0) {
          processWord(termBuffer, lastWordStart, len, wordCount++);
          lastWordStart = i + 1;
          i++;
        }
      }
    }

    // process the last word
    if (lastWordStart < termBufferLength) {
      processWord(termBuffer, lastWordStart, termBufferLength - lastWordStart,
                  wordCount++);
    }

    if (wordCount > maxWordCount) {
      termAtt->copyBuffer(backup, 0, termBufferLength);
    }
  }

  return true;
}

void CapitalizationFilter::processWord(std::deque<wchar_t> &buffer, int offset,
                                       int length, int wordCount)
{
  if (length < 1) {
    return;
  }

  if (onlyFirstWord && wordCount > 0) {
    for (int i = 0; i < length; i++) {
      buffer[offset + i] = towlower(buffer[offset + i]);
    }
    return;
  }

  if (keep != nullptr && keep->contains(buffer, offset, length)) {
    if (wordCount == 0 && forceFirstLetter) {
      buffer[offset] = towupper(buffer[offset]);
    }
    return;
  }

  if (length < minWordLength) {
    return;
  }

  if (okPrefix != nullptr) {
    for (auto prefix : okPrefix) {
      if (length >= prefix.size()) { // don't bother checking if the buffer
                                     // length is less than the prefix
        bool match = true;
        for (int i = 0; i < prefix.size(); i++) {
          if (prefix[i] != buffer[offset + i]) {
            match = false;
            break;
          }
        }
        if (match == true) {
          return;
        }
      }
    }
  }

  // We know it has at least one character
  /*char[] chars = w.toCharArray();
  StringBuilder word = new StringBuilder( w.length() );
  word.append( Character.toUpperCase( chars[0] ) );*/
  buffer[offset] = towupper(buffer[offset]);

  for (int i = 1; i < length; i++) {
    buffer[offset + i] = towlower(buffer[offset + i]);
  }
  // return word.toString();
}
} // namespace org::apache::lucene::analysis::miscellaneous