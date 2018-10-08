using namespace std;

#include "Token.h"
#include "dict/Dictionary.h"

namespace org::apache::lucene::analysis::ja
{
using Type = org::apache::lucene::analysis::ja::JapaneseTokenizer::Type;
using Dictionary = org::apache::lucene::analysis::ja::dict::Dictionary;

Token::Token(int wordId, std::deque<wchar_t> &surfaceForm, int offset,
             int length, Type type, int position,
             shared_ptr<Dictionary> dictionary)
    : dictionary(dictionary), wordId(wordId), surfaceForm(surfaceForm),
      offset(offset), length(length), position(position), type(type)
{
}

wstring Token::toString()
{
  return L"Token(\"" + wstring(surfaceForm, offset, length) + L"\" pos=" +
         to_wstring(position) + L" length=" + to_wstring(length) + L" posLen=" +
         to_wstring(positionLength) + L" type=" + type + L" wordId=" +
         to_wstring(wordId) + L" leftID=" +
         to_wstring(dictionary->getLeftId(wordId)) + L")";
}

std::deque<wchar_t> Token::getSurfaceForm() { return surfaceForm; }

int Token::getOffset() { return offset; }

int Token::getLength() { return length; }

wstring Token::getSurfaceFormString()
{
  return wstring(surfaceForm, offset, length);
}

wstring Token::getReading()
{
  return dictionary->getReading(wordId, surfaceForm, offset, length);
}

wstring Token::getPronunciation()
{
  return dictionary->getPronunciation(wordId, surfaceForm, offset, length);
}

wstring Token::getPartOfSpeech() { return dictionary->getPartOfSpeech(wordId); }

wstring Token::getInflectionType()
{
  return dictionary->getInflectionType(wordId);
}

wstring Token::getInflectionForm()
{
  return dictionary->getInflectionForm(wordId);
}

wstring Token::getBaseForm()
{
  return dictionary->getBaseForm(wordId, surfaceForm, offset, length);
}

Type Token::getType() { return type; }

bool Token::isKnown() { return type == Type::KNOWN; }

bool Token::isUnknown() { return type == Type::UNKNOWN; }

bool Token::isUser() { return type == Type::USER; }

int Token::getPosition() { return position; }

void Token::setPositionLength(int positionLength)
{
  this->positionLength = positionLength;
}

int Token::getPositionLength() { return positionLength; }
} // namespace org::apache::lucene::analysis::ja