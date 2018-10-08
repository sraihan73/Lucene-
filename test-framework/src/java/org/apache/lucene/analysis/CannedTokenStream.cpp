using namespace std;

#include "CannedTokenStream.h"

namespace org::apache::lucene::analysis
{
using Token = org::apache::lucene::analysis::Token;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

CannedTokenStream::CannedTokenStream(deque<Token> &tokens)
    : CannedTokenStream(0, 0, tokens)
{
}

CannedTokenStream::CannedTokenStream(int finalPosInc, int finalOffset,
                                     deque<Token> &tokens)
    : TokenStream(Token::TOKEN_ATTRIBUTE_FACTORY), tokens(tokens),
      finalOffset(finalOffset), finalPosInc(finalPosInc)
{
}

void CannedTokenStream::end() 
{
  TokenStream::end();
  posIncrAtt->setPositionIncrement(finalPosInc);
  offsetAtt->setOffset(finalOffset, finalOffset);
}

bool CannedTokenStream::incrementToken()
{
  if (upto < tokens.size()) {
    clearAttributes();
    // NOTE: this looks weird, casting offsetAtt to Token, but because we are
    // using the Token class's AttributeFactory, all attributes are in fact
    // backed by the Token class, so we just copy the current token into our
    // Token:
    tokens[upto++]->copyTo(std::static_pointer_cast<Token>(offsetAtt));
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis