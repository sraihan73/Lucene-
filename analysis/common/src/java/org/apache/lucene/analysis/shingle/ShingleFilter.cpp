using namespace std;

#include "ShingleFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

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
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
const wstring ShingleFilter::DEFAULT_FILLER_TOKEN = L"_";
const wstring ShingleFilter::DEFAULT_TOKEN_TYPE = L"shingle";
const wstring ShingleFilter::DEFAULT_TOKEN_SEPARATOR = L" ";

ShingleFilter::ShingleFilter(shared_ptr<TokenStream> input, int minShingleSize,
                             int maxShingleSize)
    : org::apache::lucene::analysis::TokenFilter(input)
{
  setMaxShingleSize(maxShingleSize);
  setMinShingleSize(minShingleSize);
}

ShingleFilter::ShingleFilter(shared_ptr<TokenStream> input, int maxShingleSize)
    : ShingleFilter(input, DEFAULT_MIN_SHINGLE_SIZE, maxShingleSize)
{
}

ShingleFilter::ShingleFilter(shared_ptr<TokenStream> input)
    : ShingleFilter(input, DEFAULT_MIN_SHINGLE_SIZE, DEFAULT_MAX_SHINGLE_SIZE)
{
}

ShingleFilter::ShingleFilter(shared_ptr<TokenStream> input,
                             const wstring &tokenType)
    : ShingleFilter(input, DEFAULT_MIN_SHINGLE_SIZE, DEFAULT_MAX_SHINGLE_SIZE)
{
  setTokenType(tokenType);
}

void ShingleFilter::setTokenType(const wstring &tokenType)
{
  this->tokenType = tokenType;
}

void ShingleFilter::setOutputUnigrams(bool outputUnigrams)
{
  this->outputUnigrams = outputUnigrams;
  gramSize = make_shared<CircularSequence>(shared_from_this());
}

void ShingleFilter::setOutputUnigramsIfNoShingles(
    bool outputUnigramsIfNoShingles)
{
  this->outputUnigramsIfNoShingles = outputUnigramsIfNoShingles;
}

void ShingleFilter::setMaxShingleSize(int maxShingleSize)
{
  if (maxShingleSize < 2) {
    throw invalid_argument(L"Max shingle size must be >= 2");
  }
  this->maxShingleSize = maxShingleSize;
}

void ShingleFilter::setMinShingleSize(int minShingleSize)
{
  if (minShingleSize < 2) {
    throw invalid_argument(L"Min shingle size must be >= 2");
  }
  if (minShingleSize > maxShingleSize) {
    throw invalid_argument(L"Min shingle size must be <= max shingle size");
  }
  this->minShingleSize = minShingleSize;
  gramSize = make_shared<CircularSequence>(shared_from_this());
}

void ShingleFilter::setTokenSeparator(const wstring &tokenSeparator)
{
  this->tokenSeparator = L"" == tokenSeparator ? L"" : tokenSeparator;
}

void ShingleFilter::setFillerToken(const wstring &fillerToken)
{
  this->fillerToken.clear() == fillerToken ? std::deque<wchar_t>(0)
                                           : fillerToken.toCharArray();
}

bool ShingleFilter::incrementToken() 
{
  bool tokenAvailable = false;
  int builtGramSize = 0;
  if (gramSize->atMinValue() || inputWindow.size() < gramSize->getValue()) {
    shiftInputWindow();
    gramBuilder->setLength(0);
  } else {
    builtGramSize = gramSize->getPreviousValue();
  }
  if (inputWindow.size() >= gramSize->getValue()) {
    bool isAllFiller = true;
    shared_ptr<InputWindowToken> nextToken = nullptr;
    deque<std::shared_ptr<InputWindowToken>>::const_iterator iter =
        inputWindow.begin();
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    for (int gramNum = 1;
         iter.hasNext() && builtGramSize < gramSize->getValue(); ++gramNum) {
      // C++ TODO: Java iterators are only converted within the context of
      // 'while' and 'for' loops:
      nextToken = iter.next();
      if (builtGramSize < gramNum) {
        if (builtGramSize > 0) {
          gramBuilder->append(tokenSeparator);
        }
        gramBuilder->append(nextToken->termAtt->buffer(), 0,
                            nextToken->termAtt->length());
        ++builtGramSize;
      }
      if (isAllFiller && nextToken->isFiller) {
        if (gramNum == gramSize->getValue()) {
          gramSize->advance();
        }
      } else {
        isAllFiller = false;
      }
    }
    if (!isAllFiller && builtGramSize == gramSize->getValue()) {
      inputWindow.front().attSource::copyTo(shared_from_this());
      posIncrAtt->setPositionIncrement(isOutputHere ? 0 : 1);
      termAtt->setEmpty()->append(gramBuilder);
      if (gramSize->getValue() > 1) {
        typeAtt->setType(tokenType);
        noShingleOutput = false;
      }
      offsetAtt->setOffset(offsetAtt->startOffset(),
                           nextToken->offsetAtt->endOffset());
      if (outputUnigrams) {
        posLenAtt->setPositionLength(builtGramSize);
      } else {
        // position length for this token is the number of position created by
        // shingles of smaller size.
        posLenAtt->setPositionLength(
            max(1, (builtGramSize - minShingleSize) + 1));
      }
      isOutputHere = true;
      gramSize->advance();
      tokenAvailable = true;
    }
  }
  return tokenAvailable;
}

shared_ptr<InputWindowToken> ShingleFilter::getNextToken(
    shared_ptr<InputWindowToken> target) 
{
  shared_ptr<InputWindowToken> newTarget = target;
  if (numFillerTokensToInsert > 0) {
    if (nullptr == target) {
      newTarget = make_shared<InputWindowToken>(
          nextInputStreamToken->cloneAttributes());
    } else {
      nextInputStreamToken->copyTo(target->attSource);
    }
    // A filler token occupies no space
    newTarget->offsetAtt->setOffset(newTarget->offsetAtt->startOffset(),
                                    newTarget->offsetAtt->startOffset());
    newTarget->termAtt->copyBuffer(fillerToken, 0, fillerToken.size());
    newTarget->isFiller = true;
    --numFillerTokensToInsert;
  } else if (isNextInputStreamToken) {
    if (nullptr == target) {
      newTarget = make_shared<InputWindowToken>(
          nextInputStreamToken->cloneAttributes());
    } else {
      nextInputStreamToken->copyTo(target->attSource);
    }
    isNextInputStreamToken = false;
    newTarget->isFiller = false;
  } else if (!exhausted) {
    if (input->incrementToken()) {
      if (nullptr == target) {
        newTarget = make_shared<InputWindowToken>(cloneAttributes());
      } else {
        this->copyTo(target->attSource);
      }
      if (posIncrAtt->getPositionIncrement() > 1) {
        // Each output shingle must contain at least one input token,
        // so no more than (maxShingleSize - 1) filler tokens will be inserted.
        numFillerTokensToInsert =
            min(posIncrAtt->getPositionIncrement() - 1, maxShingleSize - 1);
        // Save the current token as the next input stream token
        if (nullptr == nextInputStreamToken) {
          nextInputStreamToken = cloneAttributes();
        } else {
          this->copyTo(nextInputStreamToken);
        }
        isNextInputStreamToken = true;
        // A filler token occupies no space
        newTarget->offsetAtt->setOffset(offsetAtt->startOffset(),
                                        offsetAtt->startOffset());
        newTarget->termAtt->copyBuffer(fillerToken, 0, fillerToken.size());
        newTarget->isFiller = true;
        --numFillerTokensToInsert;
      } else {
        newTarget->isFiller = false;
      }
    } else {
      exhausted = true;
      input->end();
      endState = captureState();
      numFillerTokensToInsert =
          min(posIncrAtt->getPositionIncrement(), maxShingleSize - 1);
      if (numFillerTokensToInsert > 0) {
        nextInputStreamToken =
            make_shared<AttributeSource>(getAttributeFactory());
        nextInputStreamToken->addAttribute(CharTermAttribute::typeid);
        shared_ptr<OffsetAttribute> newOffsetAtt =
            nextInputStreamToken->addAttribute(OffsetAttribute::typeid);
        newOffsetAtt->setOffset(offsetAtt->endOffset(), offsetAtt->endOffset());
        // Recurse/loop just once:
        return getNextToken(target);
      } else {
        newTarget.reset();
      }
    }
  } else {
    newTarget.reset();
  }
  return newTarget;
}

void ShingleFilter::end() 
{
  if (!exhausted) {
    TokenFilter::end();
  } else {
    restoreState(endState);
  }
}

void ShingleFilter::shiftInputWindow() 
{
  shared_ptr<InputWindowToken> firstToken = nullptr;
  if (inputWindow.size() > 0) {
    firstToken = inputWindow.pop_front();
  }
  while (inputWindow.size() < maxShingleSize) {
    if (nullptr != firstToken) { // recycle the firstToken, if available
      if (nullptr != getNextToken(firstToken)) {
        inputWindow.push_back(firstToken); // the firstToken becomes the last
        firstToken.reset();
      } else {
        break; // end of input stream
      }
    } else {
      shared_ptr<InputWindowToken> nextToken = getNextToken(nullptr);
      if (nullptr != nextToken) {
        inputWindow.push_back(nextToken);
      } else {
        break; // end of input stream
      }
    }
  }
  if (outputUnigramsIfNoShingles && noShingleOutput && gramSize->minValue > 1 &&
      inputWindow.size() < minShingleSize) {
    gramSize->minValue = 1;
  }
  gramSize->reset();
  isOutputHere = false;
}

void ShingleFilter::reset() 
{
  TokenFilter::reset();
  gramSize->reset();
  inputWindow.clear();
  nextInputStreamToken.reset();
  isNextInputStreamToken = false;
  numFillerTokensToInsert = 0;
  isOutputHere = false;
  noShingleOutput = true;
  exhausted = false;
  endState.reset();
  if (outputUnigramsIfNoShingles && !outputUnigrams) {
    // Fix up gramSize if minValue was reset for outputUnigramsIfNoShingles
    gramSize->minValue = minShingleSize;
  }
}

ShingleFilter::CircularSequence::CircularSequence(
    shared_ptr<ShingleFilter> outerInstance)
    : outerInstance(outerInstance)
{
  minValue = outerInstance->outputUnigrams ? 1 : outerInstance->minShingleSize;
  reset();
}

int ShingleFilter::CircularSequence::getValue() { return value; }

void ShingleFilter::CircularSequence::advance()
{
  previousValue = value;
  if (value == 1) {
    value = outerInstance->minShingleSize;
  } else if (value == outerInstance->maxShingleSize) {
    reset();
  } else {
    ++value;
  }
}

void ShingleFilter::CircularSequence::reset()
{
  previousValue = value = minValue;
}

bool ShingleFilter::CircularSequence::atMinValue() { return value == minValue; }

int ShingleFilter::CircularSequence::getPreviousValue()
{
  return previousValue;
}

ShingleFilter::InputWindowToken::InputWindowToken(
    shared_ptr<AttributeSource> attSource)
    : attSource(attSource),
      termAtt(attSource->getAttribute(CharTermAttribute::typeid)),
      offsetAtt(attSource->getAttribute(OffsetAttribute::typeid))
{
}
} // namespace org::apache::lucene::analysis::shingle