using namespace std;

#include "TokenStreamFromTermVector.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PackedTokenAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::PackedTokenAttributeImpl;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using Counter = org::apache::lucene::util::Counter;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
const shared_ptr<org::apache::lucene::util::AttributeFactory>
    TokenStreamFromTermVector::ATTRIBUTE_FACTORY =
        org::apache::lucene::util::AttributeFactory::getStaticImplementation(
            org::apache::lucene::util::AttributeFactory::
                DEFAULT_ATTRIBUTE_FACTORY,
            org::apache::lucene::analysis::tokenattributes::
                PackedTokenAttributeImpl::typeid);

TokenStreamFromTermVector::TokenStreamFromTermVector(
    shared_ptr<Terms> deque, int maxStartOffset) 
    : org::apache::lucene::analysis::TokenStream(ATTRIBUTE_FACTORY),
      deque(deque), termAttribute(addAttribute(CharTermAttribute::typeid)),
      positionIncrementAttribute(
          addAttribute(PositionIncrementAttribute::typeid)),
      maxStartOffset(maxStartOffset < 0 ? numeric_limits<int>::max()
                                        : maxStartOffset)
{
  assert((!hasAttribute(PayloadAttribute::typeid),
          L"AttributeFactory shouldn't have payloads *yet*"));
  if (!deque->hasPositions() && !deque->hasOffsets()) {
    throw invalid_argument(L"The term deque needs positions and/or offsets.");
  }
  assert(deque->hasFreqs());
}

shared_ptr<Terms> TokenStreamFromTermVector::getTermVectorTerms()
{
  return deque;
}

void TokenStreamFromTermVector::reset() 
{
  incrementToken_.reset();
  TokenStream::reset();
}

void TokenStreamFromTermVector::init() 
{
  assert(!initialized);
  short dpEnumFlags = PostingsEnum::POSITIONS;
  if (deque->hasOffsets()) {
    dpEnumFlags |= PostingsEnum::OFFSETS;
    offsetAttribute = addAttribute(OffsetAttribute::typeid);
  }
  if (deque->hasPayloads() && hasAttribute(PayloadAttribute::typeid)) {
    dpEnumFlags |= (PostingsEnum::OFFSETS |
                    PostingsEnum::PAYLOADS); // must ask for offsets too
    payloadAttribute = getAttribute(PayloadAttribute::typeid);
    payloadsBytesRefArray = make_shared<BytesRefArray>(Counter::newCounter());
    spareBytesRefBuilder = make_shared<BytesRefBuilder>();
  }

  // We put term data here
  termCharsBuilder = make_shared<CharsRefBuilder>();
  termCharsBuilder->grow(static_cast<int>(
      deque->size() * 7)); // 7 is over-estimate of average term len

  // Step 1: iterate termsEnum and create a token, placing into an array of
  // tokens by position

  std::deque<std::shared_ptr<TokenLL>> positionedTokens = initTokensArray();

  int lastPosition = -1;

  shared_ptr<TermsEnum> *const termsEnum = deque->begin();
  shared_ptr<BytesRef> termBytesRef;
  shared_ptr<PostingsEnum> dpEnum = nullptr;
  shared_ptr<CharsRefBuilder> tempCharsRefBuilder =
      make_shared<CharsRefBuilder>(); // only for UTF8->UTF16 call
  // int sumFreq = 0;
  while ((termBytesRef = termsEnum->next()) != nullptr) {
    // Grab the term (in same way as BytesRef.utf8ToString() but we don't want a
    // std::wstring obj)
    // note: if term vectors supported seek by ord then we might just keep an
    // int and seek by ord on-demand
    tempCharsRefBuilder->grow(termBytesRef->length);
    constexpr int termCharsLen =
        UnicodeUtil::UTF8toUTF16(termBytesRef, tempCharsRefBuilder->chars());
    constexpr int termCharsOff = termCharsBuilder->length();
    termCharsBuilder->append(tempCharsRefBuilder->chars(), 0, termCharsLen);

    dpEnum = termsEnum->postings(dpEnum, dpEnumFlags);
    assert(dpEnum !=
           nullptr); // presumably checked by TokenSources.hasPositions earlier
    dpEnum->nextDoc();
    constexpr int freq = dpEnum->freq();
    // sumFreq += freq;
    for (int j = 0; j < freq; j++) {
      int pos = dpEnum->nextPosition();
      shared_ptr<TokenLL> token = make_shared<TokenLL>();
      token->termCharsOff = termCharsOff;
      token->termCharsLen =
          static_cast<short>(min(termCharsLen, numeric_limits<short>::max()));
      if (offsetAttribute != nullptr) {
        token->startOffset = dpEnum->startOffset();
        if (token->startOffset > maxStartOffset) {
          continue; // filter this token out; exceeds threshold
        }
        token->endOffsetInc =
            static_cast<short>(min(dpEnum->endOffset() - token->startOffset,
                                   numeric_limits<short>::max()));
        if (pos == -1) {
          pos = token->startOffset >> 3; // divide by 8
        }
      }

      if (payloadAttribute != nullptr) {
        shared_ptr<BytesRef> *const payload = dpEnum->getPayload();
        token->payloadIndex =
            payload == nullptr ? -1 : payloadsBytesRefArray->append(payload);
      }

      // Add token to an array indexed by position
      if (positionedTokens.size() <= pos) {
        // grow, but not 2x since we think our original length estimate is close
        std::deque<std::shared_ptr<TokenLL>> newPositionedTokens(
            static_cast<int>((pos + 1) * 1.5f));
        System::arraycopy(positionedTokens, 0, newPositionedTokens, 0,
                          lastPosition + 1);
        positionedTokens = newPositionedTokens;
      }
      positionedTokens[pos] =
          token->insertIntoSortedLinkedList(positionedTokens[pos]);

      lastPosition = max(lastPosition, pos);
    }
  }

  //    System.out.println(std::wstring.format(
  //        "SumFreq: %5d Size: %4d SumFreq/size: %3.3f MaxPos: %4d
  //        MaxPos/SumFreq: %3.3f WastePct: %3.3f", sumFreq, deque.size(),
  //        (sumFreq / (float)deque.size()), lastPosition,
  //        ((float)lastPosition)/sumFreq,
  //        (originalPositionEstimate/(lastPosition + 1.0f))));

  // Step 2:  Link all Tokens into a linked-deque and set position increments as
  // we go

  int prevTokenPos = -1;
  shared_ptr<TokenLL> prevToken = nullptr;
  for (int pos = 0; pos <= lastPosition; pos++) {
    shared_ptr<TokenLL> token = positionedTokens[pos];
    if (token == nullptr) {
      continue;
    }
    // link
    if (prevToken != nullptr) {
      assert(prevToken->next == nullptr);
      prevToken->next = token; // concatenate linked-deque
    } else {
      assert(firstToken == nullptr);
      firstToken = token;
    }
    // set increments
    if (deque->hasPositions()) {
      token->positionIncrement = pos - prevTokenPos;
      while (token->next != nullptr) {
        token = token->next;
        token->positionIncrement = 0;
      }
    } else {
      token->positionIncrement = 1;
      while (token->next != nullptr) {
        prevToken = token;
        token = token->next;
        if (prevToken->startOffset == token->startOffset) {
          token->positionIncrement = 0;
        } else {
          token->positionIncrement = 1;
        }
      }
    }
    prevTokenPos = pos;
    prevToken = token;
  }

  initialized = true;
}

std::deque<std::shared_ptr<TokenLL>>
TokenStreamFromTermVector::initTokensArray() 
{
  // Estimate the number of position slots we need from term stats.  We use some
  // estimation factors taken from
  //  Wikipedia that reduce the likelihood of needing to expand the array.
  int sumTotalTermFreq = static_cast<int>(deque->getSumTotalTermFreq());
  if (sumTotalTermFreq ==
      -1) { // unfortunately term vectors seem to not have this stat
    int size = static_cast<int>(deque->size());
    if (size == -1) { // doesn't happen with term vectors, it seems, but pick a
                      // default any way
      size = 128;
    }
    sumTotalTermFreq = static_cast<int>(size * 2.4);
  }
  constexpr int originalPositionEstimate = static_cast<int>(
      sumTotalTermFreq * 1.5); // less than 1 in 10 docs exceed this

  // This estimate is based on maxStartOffset. Err on the side of this being
  // larger than needed.
  constexpr int offsetLimitPositionEstimate =
      static_cast<int>(maxStartOffset / 5.0);

  // Take the smaller of the two estimates, but no smaller than 64
  return std::deque<std::shared_ptr<TokenLL>>(
      max(64, min(originalPositionEstimate, offsetLimitPositionEstimate)));
}

bool TokenStreamFromTermVector::incrementToken() 
{
  if (incrementToken_ == nullptr) {
    if (!initialized) {
      init();
      assert(initialized);
    }
    incrementToken_ = firstToken;
    if (incrementToken_ == nullptr) {
      return false;
    }
  } else if (incrementToken_->next != nullptr) {
    incrementToken_ = incrementToken_->next;
  } else {
    return false;
  }
  clearAttributes();
  termAttribute->copyBuffer(termCharsBuilder->chars(),
                            incrementToken_->termCharsOff,
                            incrementToken_->termCharsLen);
  positionIncrementAttribute->setPositionIncrement(
      incrementToken_->positionIncrement);
  if (offsetAttribute != nullptr) {
    offsetAttribute->setOffset(incrementToken_->startOffset,
                               incrementToken_->startOffset +
                                   incrementToken_->endOffsetInc);
  }
  if (payloadAttribute != nullptr) {
    if (incrementToken_->payloadIndex == -1) {
      payloadAttribute->setPayload(nullptr);
    } else {
      payloadAttribute->setPayload(payloadsBytesRefArray->get(
          spareBytesRefBuilder, incrementToken_->payloadIndex));
    }
  }
  return true;
}

shared_ptr<TokenLL>
TokenStreamFromTermVector::TokenLL::insertIntoSortedLinkedList(
    shared_ptr<TokenLL> head)
{
  assert(next == nullptr);
  if (head == nullptr) {
    return shared_from_this();
  } else if (this->compareOffsets(head) <= 0) {
    this->next = head;
    return shared_from_this();
  }
  shared_ptr<TokenLL> prev = head;
  while (prev->next != nullptr && this->compareOffsets(prev->next) > 0) {
    prev = prev->next;
  }
  this->next = prev->next;
  prev->next = shared_from_this();
  return head;
}

int TokenStreamFromTermVector::TokenLL::compareOffsets(
    shared_ptr<TokenLL> tokenB)
{
  int cmp = Integer::compare(this->startOffset, tokenB->startOffset);
  if (cmp == 0) {
    cmp = Short::compare(this->endOffsetInc, tokenB->endOffsetInc);
  }
  return cmp;
}
} // namespace org::apache::lucene::search::highlight