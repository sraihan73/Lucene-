#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::search::highlight
{
class InvalidTokenOffsetsException;
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

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

// LUCENE-2874

/** Tests {@link org.apache.lucene.search.highlight.TokenSources} and
 *  {@link org.apache.lucene.search.highlight.TokenStreamFromTermVector}
 * indirectly from that.
 */
class TokenSourcesTest : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TokenSourcesTest)
private:
  static const std::wstring FIELD;

private:
  class OverlappingTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(OverlappingTokenStream)
  private:
    std::deque<std::shared_ptr<Token>> tokens;

    int i = -1;

    const std::shared_ptr<CharTermAttribute> termAttribute =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAttribute =
        addAttribute(OffsetAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute>
        positionIncrementAttribute =
            addAttribute(PositionIncrementAttribute::typeid);

  public:
    bool incrementToken() override;

    void reset() override;

  protected:
    std::shared_ptr<OverlappingTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<OverlappingTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  virtual void testOverlapWithOffset() throw(IOException,
                                             InvalidTokenOffsetsException);

  virtual void
  testOverlapWithPositionsAndOffset() throw(IOException,
                                            InvalidTokenOffsetsException);

  virtual void
  testOverlapWithOffsetExactPhrase() throw(IOException,
                                           InvalidTokenOffsetsException);

  virtual void testOverlapWithPositionsAndOffsetExactPhrase() throw(
      IOException, InvalidTokenOffsetsException);

  virtual void
  testTermVectorWithoutOffsetsDoesntWork() throw(IOException,
                                                 InvalidTokenOffsetsException);

  int curOffset = 0;

  /** Just make a token with the text, and set the payload
   *  to the text as well.  Offets increment "naturally". */
private:
  std::shared_ptr<Token> getToken(const std::wstring &text);

  // LUCENE-5294
public:
  virtual void testPayloads() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Repeat(iterations = 10) public void
  // testRandomizedRoundTrip() throws Exception
  virtual void testRandomizedRoundTrip() ;

  virtual void testMaxStartOffsetConsistency() ;

protected:
  std::shared_ptr<TokenSourcesTest> shared_from_this()
  {
    return std::static_pointer_cast<TokenSourcesTest>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::highlight
