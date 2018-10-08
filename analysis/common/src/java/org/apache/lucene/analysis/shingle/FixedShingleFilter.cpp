using namespace std;

#include "FixedShingleFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

namespace org::apache::lucene::analysis::shingle
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

FixedShingleFilter::FixedShingleFilter(shared_ptr<TokenStream> input,
                                       int shingleSize)
    : FixedShingleFilter(input, shingleSize, L" ", L"_")
{
}

FixedShingleFilter::FixedShingleFilter(shared_ptr<TokenStream> input,
                                       int shingleSize,
                                       const wstring &tokenSeparator,
                                       const wstring &fillerToken)
    : org::apache::lucene::analysis::TokenFilter(input),
      shingleSize(shingleSize), tokenSeparator(tokenSeparator)
{
  if (shingleSize <= 1 || shingleSize > MAX_SHINGLE_SIZE) {
    throw invalid_argument(L"Shingle size must be between 2 and " +
                           to_wstring(MAX_SHINGLE_SIZE) + L", got " +
                           to_wstring(shingleSize));
  }
  this->gapToken->termAtt->setEmpty()->append(fillerToken);
  this->currentShingleTokens = std::deque<std::shared_ptr<Token>>(shingleSize);
}

bool FixedShingleFilter::incrementToken() 
{
  int posInc = 0;
  if (nextShingle() == false) {
    shared_ptr<Token> nextRoot = nextTokenInStream(currentShingleTokens[0]);
    if (nextRoot == endToken) {
      return false;
    }
    recycleToken(currentShingleTokens[0]);
    if (resetShingleRoot(nextRoot) == false) {
      return false;
    }
    posInc = currentShingleTokens[0]->posInc();
  }
  clearAttributes();
  incAtt->setPositionIncrement(posInc);
  offsetAtt->setOffset(currentShingleTokens[0]->startOffset(),
                       lastTokenInShingle()->endOffset());
  termAtt->setEmpty();
  termAtt->append(currentShingleTokens[0]->term());
  typeAtt->setType(L"shingle");
  for (int i = 1; i < shingleSize; i++) {
    termAtt->append(tokenSeparator)->append(currentShingleTokens[i]->term());
  }
  return true;
}

void FixedShingleFilter::reset() 
{
  TokenFilter::reset();
  this->tokenPool->clear();
  this->currentShingleTokens[0].reset();
  this->inputStreamExhausted = false;
  this->currentShingleStackSize = 0;
}

void FixedShingleFilter::end() 
{
  if (inputStreamExhausted == false) {
    finishInnerStream();
  }
  clearAttributes();
  this->offsetAtt->setOffset(0, endToken->endOffset());
}

void FixedShingleFilter::finishInnerStream() 
{
  input->end();
  inputStreamExhausted = true;
  // check for gaps at the end of the tokenstream
  endToken->posIncAtt->setPositionIncrement(
      this->incAtt->getPositionIncrement());
  shared_ptr<OffsetAttribute> inputOffsets =
      input->getAttribute(OffsetAttribute::typeid);
  endToken->offsetAtt->setOffset(inputOffsets->startOffset(),
                                 inputOffsets->endOffset());
}

shared_ptr<Token> FixedShingleFilter::lastTokenInShingle()
{
  int lastTokenIndex = shingleSize - 1;
  while (currentShingleTokens[lastTokenIndex] == gapToken) {
    lastTokenIndex--;
  }
  return currentShingleTokens[lastTokenIndex];
}

bool FixedShingleFilter::resetShingleRoot(shared_ptr<Token> token) throw(
    IOException)
{
  this->currentShingleTokens[0] = token;
  for (int i = 1; i < shingleSize; i++) {
    shared_ptr<Token> current =
        nextTokenInGraph(this->currentShingleTokens[i - 1]);
    if (current == endToken) {
      if (endToken->posInc() + i >= shingleSize) {
        // end tokens are a special case, because their posIncs are always
        // due to stopwords.  Therefore, we can happily append gap tokens
        // to the end of the current shingle
        for (int j = i; j < shingleSize; j++) {
          this->currentShingleTokens[i] = gapToken;
          i++;
        }
        return true;
      }
      return false;
    }
    if (current->posInc() > 1) {
      // insert gaps into the shingle deque
      for (int j = 1; j < current->posInc(); j++) {
        this->currentShingleTokens[i] = gapToken;
        i++;
        if (i >= shingleSize) {
          return true;
        }
      }
    }
    this->currentShingleTokens[i] = current;
  }
  return true;
}

bool FixedShingleFilter::nextShingle() 
{
  return currentShingleTokens[0] != nullptr && advanceStack();
}

bool FixedShingleFilter::lastInStack(shared_ptr<Token> token) 
{
  shared_ptr<Token> next = nextTokenInStream(token);
  return next == endToken || next->posInc() != 0;
}

bool FixedShingleFilter::advanceStack() 
{
  for (int i = shingleSize - 1; i >= 1; i--) {
    if (currentShingleTokens[i] != gapToken &&
        lastInStack(currentShingleTokens[i]) == false) {
      currentShingleTokens[i] = nextTokenInStream(currentShingleTokens[i]);
      for (int j = i + 1; j < shingleSize; j++) {
        currentShingleTokens[j] = nextTokenInGraph(currentShingleTokens[j - 1]);
      }
      if (currentShingleStackSize++ > MAX_SHINGLE_STACK_SIZE) {
        throw make_shared<IllegalStateException>(
            L"Too many shingles (> " + to_wstring(MAX_SHINGLE_STACK_SIZE) +
            L") at term [" + currentShingleTokens[0]->term() + L"]");
      }
      return true;
    }
  }
  currentShingleStackSize = 0;
  return false;
}

shared_ptr<Token> FixedShingleFilter::newToken()
{
  shared_ptr<Token> token = tokenPool->size() == 0
                                ? make_shared<Token>(this->cloneAttributes())
                                : tokenPool->removeFirst();
  token->reset(shared_from_this());
  return token;
}

void FixedShingleFilter::recycleToken(shared_ptr<Token> token)
{
  if (token == nullptr) {
    return;
  }
  token->nextToken.reset();
  tokenPool->add(token);
}

int FixedShingleFilter::instantiatedTokenCount()
{
  int tokenCount = tokenPool->size() + 1;
  if (currentShingleTokens[0] == endToken ||
      currentShingleTokens[0] == nullptr) {
    return tokenCount;
  }
  for (shared_ptr<Token> t = currentShingleTokens[0];
       t != endToken && t != nullptr; t = t->nextToken) {
    tokenCount++;
  }
  return tokenCount;
}

shared_ptr<Token>
FixedShingleFilter::nextTokenInGraph(shared_ptr<Token> token) 
{
  do {
    token = nextTokenInStream(token);
    if (token == endToken) {
      return endToken;
    }
  } while (token->posInc() == 0);
  return token;
}

shared_ptr<Token> FixedShingleFilter::nextTokenInStream(
    shared_ptr<Token> token) 
{
  if (token != nullptr && token->nextToken != nullptr) {
    return token->nextToken;
  }
  if (input->incrementToken() == false) {
    finishInnerStream();
    if (token == nullptr) {
      return endToken;
    } else {
      token->nextToken = endToken;
      return endToken;
    }
  }
  if (token == nullptr) {
    return newToken();
  }
  token->nextToken = newToken();
  return token->nextToken;
}

FixedShingleFilter::Token::Token(shared_ptr<AttributeSource> attSource)
    : attSource(attSource),
      posIncAtt(attSource->addAttribute(PositionIncrementAttribute::typeid)),
      termAtt(attSource->addAttribute(CharTermAttribute::typeid)),
      offsetAtt(attSource->addAttribute(OffsetAttribute::typeid))
{
}

int FixedShingleFilter::Token::posInc()
{
  return this->posIncAtt->getPositionIncrement();
}

shared_ptr<std::wstring> FixedShingleFilter::Token::term()
{
  return this->termAtt;
}

int FixedShingleFilter::Token::startOffset()
{
  return this->offsetAtt->startOffset();
}

int FixedShingleFilter::Token::endOffset()
{
  return this->offsetAtt->endOffset();
}

void FixedShingleFilter::Token::reset(shared_ptr<AttributeSource> attSource)
{
  attSource->copyTo(this->attSource);
  this->nextToken.reset();
}

wstring FixedShingleFilter::Token::toString()
{
  return term() + L"(" + to_wstring(startOffset()) + L"," +
         to_wstring(endOffset()) + L") " + to_wstring(posInc());
}
} // namespace org::apache::lucene::analysis::shingle