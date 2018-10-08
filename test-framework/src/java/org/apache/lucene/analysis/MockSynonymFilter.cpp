using namespace std;

#include "MockSynonymFilter.h"

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
using AttributeSource = org::apache::lucene::util::AttributeSource;

MockSynonymFilter::MockSynonymFilter(shared_ptr<TokenStream> input)
    : TokenFilter(input)
{
}

void MockSynonymFilter::reset() 
{
  TokenFilter::reset();
  tokenQueue.clear();
  endOfInput = false;
}

bool MockSynonymFilter::incrementToken() 
{
  if (tokenQueue.size() > 0) {
    tokenQueue.erase(tokenQueue.begin()).copyTo(shared_from_this());
    return true;
  }
  if (endOfInput == false && input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    if (termAtt->toString()->equals(L"dogs")) {
      addSynonymAndRestoreOrigToken(L"dog", 1, offsetAtt->endOffset());
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    else if (termAtt->toString()->equals(L"guinea")) {
      shared_ptr<AttributeSource> firstSavedToken = cloneAttributes();
      if (input->incrementToken()) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        if (termAtt->toString()->equals(L"pig")) {
          shared_ptr<AttributeSource> secondSavedToken = cloneAttributes();
          int secondEndOffset = offsetAtt->endOffset();
          firstSavedToken->copyTo(shared_from_this());
          addSynonym(L"cavy", 2, secondEndOffset);
          tokenQueue.push_back(secondSavedToken);
        }
        // C++ TODO: There is no native C++ equivalent to 'toString':
        else if (termAtt->toString()->equals(L"dogs")) {
          tokenQueue.push_back(cloneAttributes());
          addSynonym(L"dog", 1, offsetAtt->endOffset());
        }
      } else {
        endOfInput = true;
      }
      firstSavedToken->copyTo(shared_from_this());
    }
    return true;
  } else {
    endOfInput = true;
    return false;
  }
}

void MockSynonymFilter::addSynonym(const wstring &synonymText, int posLen,
                                   int endOffset)
{
  termAtt->setEmpty()->append(synonymText);
  posIncAtt->setPositionIncrement(0);
  posLenAtt->setPositionLength(posLen);
  offsetAtt->setOffset(offsetAtt->startOffset(), endOffset);
  tokenQueue.push_back(cloneAttributes());
}

void MockSynonymFilter::addSynonymAndRestoreOrigToken(
    const wstring &synonymText, int posLen, int endOffset)
{
  shared_ptr<AttributeSource> origToken = cloneAttributes();
  addSynonym(synonymText, posLen, endOffset);
  origToken->copyTo(shared_from_this());
}
} // namespace org::apache::lucene::analysis