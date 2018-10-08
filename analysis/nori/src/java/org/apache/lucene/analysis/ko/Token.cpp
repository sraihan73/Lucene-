using namespace std;

#include "Token.h"
#include "dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko
{
using Morpheme = org::apache::lucene::analysis::ko::dict::Dictionary::Morpheme;

Token::Token(std::deque<wchar_t> &surfaceForm, int offset, int length,
             int startOffset, int endOffset)
    : surfaceForm(surfaceForm), offset(offset), length(length),
      startOffset(startOffset), endOffset(endOffset)
{
}

std::deque<wchar_t> Token::getSurfaceForm() { return surfaceForm; }

int Token::getOffset() { return offset; }

int Token::getLength() { return length; }

wstring Token::getSurfaceFormString()
{
  return wstring(surfaceForm, offset, length);
}

int Token::getStartOffset() { return startOffset; }

int Token::getEndOffset() { return endOffset; }

void Token::setPositionIncrement(int posIncr) { this->posIncr = posIncr; }

int Token::getPositionIncrement() { return posIncr; }

void Token::setPositionLength(int posLen) { this->posLen = posLen; }

int Token::getPositionLength() { return posLen; }
} // namespace org::apache::lucene::analysis::ko