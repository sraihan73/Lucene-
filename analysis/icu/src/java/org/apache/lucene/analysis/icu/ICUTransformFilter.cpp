using namespace std;

#include "ICUTransformFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::icu
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using com::ibm::icu::text::Replaceable;
using com::ibm::icu::text::Transliterator;
using com::ibm::icu::text::UnicodeSet;
using com::ibm::icu::text::UTF16;

ICUTransformFilter::ICUTransformFilter(shared_ptr<TokenStream> input,
                                       shared_ptr<Transliterator> transform)
    : org::apache::lucene::analysis::TokenFilter(input), transform(transform)
{

  /*
   * This is cheating, but speeds things up a lot.
   * If we wanted to use pkg-private APIs we could probably do better.
   */
  if (transform->getFilter() == nullptr &&
      std::dynamic_pointer_cast<com::ibm::icu::text::RuleBasedTransliterator>(
          transform) != nullptr) {
    shared_ptr<UnicodeSet> *const sourceSet = transform->getSourceSet();
    if (sourceSet != nullptr && !sourceSet->isEmpty()) {
      transform->setFilter(sourceSet);
    }
  }
}

bool ICUTransformFilter::incrementToken() 
{
  /*
   * Wrap around replaceable. clear the positions, and transliterate.
   */
  if (input->incrementToken()) {
    replaceableAttribute->setText(termAtt);

    constexpr int length = termAtt->length();
    position->start = 0;
    position->limit = length;
    position->contextStart = 0;
    position->contextLimit = length;

    transform->filteredTransliterate(replaceableAttribute, position, false);
    return true;
  } else {
    return false;
  }
}

void ICUTransformFilter::ReplaceableTermAttribute::setText(
    shared_ptr<CharTermAttribute> token)
{
  this->token = token;
  this->buffer = token->buffer();
  this->length_ = token->length();
}

int ICUTransformFilter::ReplaceableTermAttribute::char32At(int pos)
{
  return UTF16->charAt(buffer, 0, length_, pos);
}

wchar_t ICUTransformFilter::ReplaceableTermAttribute::charAt(int pos)
{
  return buffer[pos];
}

void ICUTransformFilter::ReplaceableTermAttribute::copy(int start, int limit,
                                                        int dest)
{
  std::deque<wchar_t> text(limit - start);
  getChars(start, limit, text, 0);
  replace(dest, dest, text, 0, limit - start);
}

void ICUTransformFilter::ReplaceableTermAttribute::getChars(
    int srcStart, int srcLimit, std::deque<wchar_t> &dst, int dstStart)
{
  System::arraycopy(buffer, srcStart, dst, dstStart, srcLimit - srcStart);
}

bool ICUTransformFilter::ReplaceableTermAttribute::hasMetaData()
{
  return false;
}

int ICUTransformFilter::ReplaceableTermAttribute::length() { return length_; }

void ICUTransformFilter::ReplaceableTermAttribute::replace(int start, int limit,
                                                           const wstring &text)
{
  constexpr int charsLen = text.length();
  constexpr int newLength = shiftForReplace(start, limit, charsLen);
  // insert the replacement text
  text.getChars(0, charsLen, buffer, start);
  token->setLength(length_ = newLength);
}

void ICUTransformFilter::ReplaceableTermAttribute::replace(
    int start, int limit, std::deque<wchar_t> &text, int charsStart,
    int charsLen)
{
  // shift text if necessary for the replacement
  constexpr int newLength = shiftForReplace(start, limit, charsLen);
  // insert the replacement text
  System::arraycopy(text, charsStart, buffer, start, charsLen);
  token->setLength(length_ = newLength);
}

int ICUTransformFilter::ReplaceableTermAttribute::shiftForReplace(int start,
                                                                  int limit,
                                                                  int charsLen)
{
  constexpr int replacementLength = limit - start;
  constexpr int newLength = length_ - replacementLength + charsLen;
  // resize if necessary
  if (newLength > length_) {
    buffer = token->resizeBuffer(newLength);
  }
  // if the substring being replaced is longer or shorter than the
  // replacement, need to shift things around
  if (replacementLength != charsLen && limit < length_) {
    System::arraycopy(buffer, limit, buffer, start + charsLen, length_ - limit);
  }
  return newLength;
}
} // namespace org::apache::lucene::analysis::icu