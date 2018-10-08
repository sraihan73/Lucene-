using namespace std;

#include "CannedBinaryTokenStream.h"

namespace org::apache::lucene::analysis
{
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;

CannedBinaryTokenStream::BinaryToken::BinaryToken(shared_ptr<BytesRef> term)
{
  this->term = term;
  this->posInc = 1;
  this->posLen = 1;
}

CannedBinaryTokenStream::BinaryToken::BinaryToken(shared_ptr<BytesRef> term,
                                                  int posInc, int posLen)
{
  this->term = term;
  this->posInc = posInc;
  this->posLen = posLen;
}

CannedBinaryTokenStream::CannedBinaryTokenStream(deque<BinaryToken> &tokens)
    : TokenStream(), tokens(tokens)
{
}

bool CannedBinaryTokenStream::incrementToken()
{
  if (upto < tokens.size()) {
    shared_ptr<BinaryToken> *const token = tokens[upto++];
    // TODO: can we just capture/restoreState so
    // we get all attrs...?
    clearAttributes();
    termAtt->setBytesRef(token->term);
    posIncrAtt->setPositionIncrement(token->posInc);
    posLengthAtt->setPositionLength(token->posLen);
    offsetAtt->setOffset(token->startOffset, token->endOffset);
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis