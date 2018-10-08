using namespace std;

#include "DictionaryToken.h"
#include "dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko
{
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;

DictionaryToken::DictionaryToken(KoreanTokenizer::Type type,
                                 shared_ptr<Dictionary> dictionary, int wordId,
                                 std::deque<wchar_t> &surfaceForm, int offset,
                                 int length, int startOffset, int endOffset)
    : Token(surfaceForm, offset, length, startOffset, endOffset),
      wordId(wordId), type(type), dictionary(dictionary)
{
}

wstring DictionaryToken::toString()
{
  return L"DictionaryToken(\"" + getSurfaceFormString() + L"\" pos=" +
         to_wstring(getStartOffset()) + L" length=" + to_wstring(getLength()) +
         L" posLen=" + to_wstring(getPositionLength()) + L" type=" + type +
         L" wordId=" + to_wstring(wordId) + L" leftID=" +
         to_wstring(dictionary->getLeftId(wordId)) + L")";
}

KoreanTokenizer::Type DictionaryToken::getType() { return type; }

bool DictionaryToken::isKnown() { return type == KoreanTokenizer::Type::KNOWN; }

bool DictionaryToken::isUnknown()
{
  return type == KoreanTokenizer::Type::UNKNOWN;
}

bool DictionaryToken::isUser() { return type == KoreanTokenizer::Type::USER; }

POS::Type DictionaryToken::getPOSType()
{
  return dictionary->getPOSType(wordId);
}

POS::Tag DictionaryToken::getLeftPOS()
{
  return dictionary->getLeftPOS(wordId);
}

POS::Tag DictionaryToken::getRightPOS()
{
  return dictionary->getRightPOS(wordId);
}

wstring DictionaryToken::getReading() { return dictionary->getReading(wordId); }

std::deque<std::shared_ptr<Dictionary::Morpheme>>
DictionaryToken::getMorphemes()
{
  return dictionary->getMorphemes(wordId, getSurfaceForm(), getOffset(),
                                  getLength());
}
} // namespace org::apache::lucene::analysis::ko