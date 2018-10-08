using namespace std;

#include "PatternTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::pattern
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

PatternTokenizer::PatternTokenizer(shared_ptr<Pattern> pattern, int group)
    : PatternTokenizer(DEFAULT_TOKEN_ATTRIBUTE_FACTORY, pattern, group)
{
}

PatternTokenizer::PatternTokenizer(shared_ptr<AttributeFactory> factory,
                                   shared_ptr<Pattern> pattern, int group)
    : org::apache::lucene::analysis::Tokenizer(factory), group(group),
      matcher(pattern->matcher(L""))
{

  // Use "" instead of str so don't consume chars
  // (fillBuffer) from the input on throwing IAE below:

  // confusingly group count depends ENTIRELY on the pattern but is only
  // accessible via matcher
  if (group >= 0 && group > matcher->groupCount()) {
    throw invalid_argument(L"invalid group specified: pattern only has: " +
                           matcher->groupCount() + L" capturing groups");
  }
}

bool PatternTokenizer::incrementToken()
{
  if (index >= str->length()) {
    return false;
  }
  clearAttributes();
  if (group >= 0) {

    // match a specific group
    while (matcher->find()) {
      index = matcher->start(group);
      constexpr int endIndex = matcher->end(group);
      if (index == endIndex) {
        continue;
      }
      termAtt->setEmpty()->append(str, index, endIndex);
      offsetAtt->setOffset(correctOffset(index), correctOffset(endIndex));
      return true;
    }

    index = numeric_limits<int>::max(); // mark exhausted
    return false;

  } else {

    // std::wstring.split() functionality
    while (matcher->find()) {
      if (matcher->start() - index > 0) {
        // found a non-zero-length token
        termAtt->setEmpty()->append(str, index, matcher->start());
        offsetAtt->setOffset(correctOffset(index),
                             correctOffset(matcher->start()));
        index = matcher->end();
        return true;
      }

      index = matcher->end();
    }

    if (str->length() - index == 0) {
      index = numeric_limits<int>::max(); // mark exhausted
      return false;
    }

    termAtt->setEmpty()->append(str, index, str->length());
    offsetAtt->setOffset(correctOffset(index), correctOffset(str->length()));
    index = numeric_limits<int>::max(); // mark exhausted
    return true;
  }
}

void PatternTokenizer::end() 
{
  Tokenizer::end();
  constexpr int ofs = correctOffset(str->length());
  offsetAtt->setOffset(ofs, ofs);
}

PatternTokenizer::~PatternTokenizer()
{
  try {
    // C++ NOTE: There is no explicit call to the base class destructor in C++:
    //        super.close();
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    str->setLength(0);
    str->trimToSize();
  }
}

void PatternTokenizer::reset() 
{
  Tokenizer::reset();
  fillBuffer(input);
  matcher->reset(str);
  index = 0;
}

void PatternTokenizer::fillBuffer(shared_ptr<Reader> input) 
{
  int len;
  str->setLength(0);
  while ((len = input->read(buffer)) > 0) {
    str->append(buffer, 0, len);
  }
}
} // namespace org::apache::lucene::analysis::pattern