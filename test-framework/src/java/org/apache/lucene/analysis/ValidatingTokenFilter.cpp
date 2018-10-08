using namespace std;

#include "ValidatingTokenFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;

ValidatingTokenFilter::ValidatingTokenFilter(shared_ptr<TokenStream> in_,
                                             const wstring &name)
    : TokenFilter(in_), name(name)
{
}

bool ValidatingTokenFilter::incrementToken() 
{

  // System.out.println(name + ": incrementToken()");

  if (!input->incrementToken()) {
    return false;
  }

  int startOffset = 0;
  int endOffset = 0;
  int posLen = 0;

  // System.out.println(name + ": " + this);

  if (posIncAtt != nullptr) {
    pos += posIncAtt->getPositionIncrement();
    if (pos == -1) {
      throw make_shared<IllegalStateException>(name +
                                               L": first posInc must be > 0");
    }
  }

  if (offsetAtt != nullptr) {
    startOffset = offsetAtt->startOffset();
    endOffset = offsetAtt->endOffset();

    if (offsetAtt->startOffset() < lastStartOffset) {
      throw make_shared<IllegalStateException>(
          name + L": offsets must not go backwards startOffset=" +
          to_wstring(startOffset) + L" is < lastStartOffset=" +
          to_wstring(lastStartOffset));
    }
    lastStartOffset = offsetAtt->startOffset();
  }

  posLen = posLenAtt == nullptr ? 1 : posLenAtt->getPositionLength();

  if (offsetAtt != nullptr && posIncAtt != nullptr) {

    if (posToStartOffset.find(pos) == posToStartOffset.end()) {
      // First time we've seen a token leaving from this position:
      posToStartOffset.emplace(pos, startOffset);
      // System.out.println(name + "  + s " + pos + " -> " + startOffset);
    } else {
      // We've seen a token leaving from this position
      // before; verify the startOffset is the same:
      // System.out.println(name + "  + vs " + pos + " -> " + startOffset);
      constexpr int oldStartOffset = posToStartOffset[pos];
      if (oldStartOffset != startOffset) {
        throw make_shared<IllegalStateException>(
            name + L": inconsistent startOffset at pos=" + to_wstring(pos) +
            L": " + to_wstring(oldStartOffset) + L" vs " +
            to_wstring(startOffset) + L"; token=" + termAtt);
      }
    }

    constexpr int endPos = pos + posLen;

    if (posToEndOffset.find(endPos) == posToEndOffset.end()) {
      // First time we've seen a token arriving to this position:
      posToEndOffset.emplace(endPos, endOffset);
      // System.out.println(name + "  + e " + endPos + " -> " + endOffset);
    } else {
      // We've seen a token arriving to this position
      // before; verify the endOffset is the same:
      // System.out.println(name + "  + ve " + endPos + " -> " + endOffset);
      constexpr int oldEndOffset = posToEndOffset[endPos];
      if (oldEndOffset != endOffset) {
        throw make_shared<IllegalStateException>(
            name + L": inconsistent endOffset at pos=" + to_wstring(endPos) +
            L": " + to_wstring(oldEndOffset) + L" vs " + to_wstring(endOffset) +
            L"; token=" + termAtt);
      }
    }
  }

  return true;
}

void ValidatingTokenFilter::end() 
{
  TokenFilter::end();

  // TODO: what else to validate

  // TODO: check that endOffset is >= max(endOffset)
  // we've seen
}

void ValidatingTokenFilter::reset() 
{
  TokenFilter::reset();
  pos = -1;
  posToStartOffset.clear();
  posToEndOffset.clear();
  lastStartOffset = 0;
}
} // namespace org::apache::lucene::analysis