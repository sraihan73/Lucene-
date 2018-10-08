using namespace std;

#include "DecompoundToken.h"
#include "dict/Dictionary.h"

namespace org::apache::lucene::analysis::ko
{
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;

DecompoundToken::DecompoundToken(POS::Tag posTag, const wstring &surfaceForm,
                                 int startOffset, int endOffset)
    : Token(surfaceForm.toCharArray(), 0, surfaceForm.length(), startOffset,
            endOffset),
      posTag(posTag)
{
}

wstring DecompoundToken::toString()
{
  return L"DecompoundToken(\"" + getSurfaceFormString() + L"\" pos=" +
         to_wstring(getStartOffset()) + L" length=" + to_wstring(getLength()) +
         L" startOffset=" + to_wstring(getStartOffset()) + L" endOffset=" +
         to_wstring(getEndOffset()) + L")";
}

POS::Type DecompoundToken::getPOSType() { return POS::Type::MORPHEME; }

POS::Tag DecompoundToken::getLeftPOS() { return posTag; }

POS::Tag DecompoundToken::getRightPOS() { return posTag; }

wstring DecompoundToken::getReading() { return L""; }

std::deque<std::shared_ptr<Dictionary::Morpheme>>
DecompoundToken::getMorphemes()
{
  return nullptr;
}
} // namespace org::apache::lucene::analysis::ko