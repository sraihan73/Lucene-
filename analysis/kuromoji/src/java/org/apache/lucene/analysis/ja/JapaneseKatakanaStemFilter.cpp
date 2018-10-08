using namespace std;

#include "JapaneseKatakanaStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"

namespace org::apache::lucene::analysis::ja
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

JapaneseKatakanaStemFilter::JapaneseKatakanaStemFilter(
    shared_ptr<TokenStream> input, int minimumLength)
    : org::apache::lucene::analysis::TokenFilter(input),
      minimumKatakanaLength(minimumLength)
{
}

JapaneseKatakanaStemFilter::JapaneseKatakanaStemFilter(
    shared_ptr<TokenStream> input)
    : JapaneseKatakanaStemFilter(input, DEFAULT_MINIMUM_LENGTH)
{
}

bool JapaneseKatakanaStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword()) {
      termAttr->setLength(stem(termAttr->buffer(), termAttr->length()));
    }
    return true;
  } else {
    return false;
  }
}

int JapaneseKatakanaStemFilter::stem(std::deque<wchar_t> &term, int length)
{
  if (length < minimumKatakanaLength) {
    return length;
  }

  if (!isKatakana(term, length)) {
    return length;
  }

  if (term[length - 1] == HIRAGANA_KATAKANA_PROLONGED_SOUND_MARK) {
    return length - 1;
  }

  return length;
}

bool JapaneseKatakanaStemFilter::isKatakana(std::deque<wchar_t> &term,
                                            int length)
{
  for (int i = 0; i < length; i++) {
    // NOTE: Test only identifies full-width characters -- half-widths are
    // supported
    if (Character::UnicodeBlock::of(term[i]) !=
        Character::UnicodeBlock::KATAKANA) {
      return false;
    }
  }
  return true;
}
} // namespace org::apache::lucene::analysis::ja