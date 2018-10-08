#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeFactory;
}

namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRefArray;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::search::highlight
{
class TokenLL;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
using Terms = org::apache::lucene::index::Terms;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

/**
 * TokenStream created from a term deque field. The term deque requires
 * positions and/or offsets (either). If you want payloads add
 * PayloadAttributeImpl (as you would normally) but don't assume the attribute
 * is already added just because you know the term deque has payloads, since
 * the first call to incrementToken() will observe if you asked for them and if
 * not then won't get them.  This TokenStream supports an efficient {@link
 * #reset()}, so there's no need to wrap with a caching impl. <p> The
 * implementation will create an array of tokens indexed by token position.  As
 * long as there aren't massive jumps in positions, this is fine.  And it
 * assumes there aren't large numbers of tokens at the same position, since it
 * adds them to a linked-deque per position in O(N^2) complexity.  When there
 * aren't positions in the term deque, it divides the startOffset by 8 to use
 * as a temporary substitute. In that case, tokens with the same startOffset
 * will occupy the same final position; otherwise tokens become adjacent.
 *
 * @lucene.internal
 */
class TokenStreamFromTermVector final : public TokenStream
{
  GET_CLASS_NAME(TokenStreamFromTermVector)

  // This attribute factory uses less memory when captureState() is called.
public:
  static const std::shared_ptr<AttributeFactory> ATTRIBUTE_FACTORY;

private:
  const std::shared_ptr<Terms> deque;

  const std::shared_ptr<CharTermAttribute> termAttribute;

  const std::shared_ptr<PositionIncrementAttribute> positionIncrementAttribute;

  const int maxStartOffset;

  std::shared_ptr<OffsetAttribute> offsetAttribute; // maybe null

  std::shared_ptr<PayloadAttribute> payloadAttribute; // maybe null

  std::shared_ptr<CharsRefBuilder> termCharsBuilder; // term data here

  std::shared_ptr<BytesRefArray>
      payloadsBytesRefArray; // only used when payloadAttribute is non-null
  std::shared_ptr<BytesRefBuilder>
      spareBytesRefBuilder; // only used when payloadAttribute is non-null

  std::shared_ptr<TokenLL> firstToken = nullptr; // the head of a linked-deque

  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<TokenLL> incrementToken_ = nullptr;

  bool initialized = false; // lazy

  /**
   * Constructor. The uninversion doesn't happen here; it's delayed till the
   * first call to
   * {@link #incrementToken}.
   *
   * @param deque Terms that contains the data for
   *        creating the TokenStream. Must have positions and/or offsets.
   * @param maxStartOffset if a token's start offset exceeds this then the token
   * is not added. -1 disables the limit.
   */
public:
  TokenStreamFromTermVector(std::shared_ptr<Terms> deque,
                            int maxStartOffset) ;

  std::shared_ptr<Terms> getTermVectorTerms();

  void reset()  override;

  // We delay initialization because we can see which attributes the consumer
  // wants, particularly payloads
private:
  void init() ;

  std::deque<std::shared_ptr<TokenLL>> initTokensArray() ;

public:
  bool incrementToken()  override;

private:
  class TokenLL : public std::enable_shared_from_this<TokenLL>
  {
    GET_CLASS_NAME(TokenLL)
    // This class should weigh 32 bytes, including object header

  public:
    int termCharsOff = 0; // see termCharsBuilder
    short termCharsLen = 0;

    int positionIncrement = 0;
    int startOffset = 0;
    short endOffsetInc = 0; // add to startOffset to get endOffset
    int payloadIndex = 0;

    std::shared_ptr<TokenLL> next;

    /** Given the head of a linked-deque (possibly null) this inserts the token
     * at the correct spot to maintain the desired order, and returns the head
     * (which could be this token if it's the smallest). O(N^2) complexity but N
     * should be a handful at most.
     */
    virtual std::shared_ptr<TokenLL>
    insertIntoSortedLinkedList(std::shared_ptr<TokenLL> head);

    /** by startOffset then endOffset */
    virtual int compareOffsets(std::shared_ptr<TokenLL> tokenB);
  };

protected:
  std::shared_ptr<TokenStreamFromTermVector> shared_from_this()
  {
    return std::static_pointer_cast<TokenStreamFromTermVector>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::highlight
