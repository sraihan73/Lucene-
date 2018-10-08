#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class AttributeSource;
}

namespace org::apache::lucene::util
{
class State;
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
namespace org::apache::lucene::analysis
{

using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;

// TODO: cut SynFilter over to this
// TODO: somehow add "nuke this input token" capability...

/** An abstract TokenFilter to make it easier to build graph
 *  token filters requiring some lookahead.  This class handles
 *  the details of buffering up tokens, recording them by
 *  position, restoring them, providing access to them, etc. */

template <typename T>
class LookaheadTokenFilter : public TokenFilter
{
  GET_CLASS_NAME(LookaheadTokenFilter)
  static_assert(std::is_base_of<LookaheadTokenFilter.Position, T>::value,
                L"T must inherit from LookaheadTokenFilter.Position");

private:
  static constexpr bool DEBUG = false;

protected:
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  // Position of last read input token:
  int inputPos = 0;

  // Position of next possible output token to return:
  int outputPos = 0;

  // True if we hit end from our input:
  bool end = false;

private:
  bool tokenPending = false;
  bool insertPending = false;

  /** Holds all state for a single position; subclass this
   *  to record other state at each position. */
protected:
  class Position : public std::enable_shared_from_this<Position>,
                   public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(Position)
    // Buffered input tokens at this position:
  public:
    const std::deque<std::shared_ptr<AttributeSource::State>> inputTokens =
        std::deque<std::shared_ptr<AttributeSource::State>>();

    // Next buffered token to be returned to consumer:
    int nextRead = 0;

    // Any token leaving from this position should have this startOffset:
    int startOffset = -1;

    // Any token arriving to this position should have this endOffset:
    int endOffset = -1;

    void reset() override;

    virtual void add(std::shared_ptr<AttributeSource::State> state);

    virtual std::shared_ptr<AttributeSource::State> nextState();
  };

protected:
  LookaheadTokenFilter(std::shared_ptr<TokenStream> input) : TokenFilter(input)
  {
  }

  /** Call this only from within afterPosition, to insert a new
   *  token.  After calling this you should set any
   *  necessary token you need. */
  virtual void insertToken() 
  {
    if (tokenPending) {
      positions->get(inputPos)->add(captureState());
      tokenPending = false;
    }
    assert(!insertPending);
    insertPending = true;
  }

  /** This is called when all input tokens leaving a given
   *  position have been returned.  Override this and
   *  call insertToken and then set whichever token's
   *  attributes you want, if you want to inject
   *  a token starting from this position. */
  virtual void afterPosition()  {}

  virtual T newPosition() = 0;

  const std::shared_ptr<RollingBuffer<T>> positions =
      std::make_shared<RollingBufferAnonymousInnerClass>();

private:
  class RollingBufferAnonymousInnerClass
      : public RollingBuffer<std::shared_ptr<T>>
  {
    GET_CLASS_NAME(RollingBufferAnonymousInnerClass)
  public:
    RollingBufferAnonymousInnerClass();

  protected:
    std::shared_ptr<T> newInstance() override;

  protected:
    std::shared_ptr<RollingBufferAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RollingBufferAnonymousInnerClass>(
          org.apache.lucene.util.RollingBuffer<T>::shared_from_this());
    }
  };

  /** Returns true if there is a new token. */
protected:
  virtual bool peekToken() 
  {
    if (DEBUG) {
      std::wcout << L"LTF.peekToken inputPos=" << inputPos << L" outputPos="
                 << outputPos << L" tokenPending=" << tokenPending << std::endl;
    }
    assert(!end);
    assert(inputPos == -1 || outputPos <= inputPos);
    if (tokenPending) {
      positions->get(inputPos)->add(captureState());
      tokenPending = false;
    }
    constexpr bool gotToken = input::incrementToken();
    if (DEBUG) {
      std::wcout << L"  input.incrToken() returned " << gotToken << std::endl;
    }
    if (gotToken) {
      inputPos += posIncAtt->getPositionIncrement();
      assert(inputPos >= 0);
      if (DEBUG) {
        std::wcout << L"  now inputPos=" << inputPos << std::endl;
      }

      std::shared_ptr<Position> *const startPosData = positions->get(inputPos);
      std::shared_ptr<Position> *const endPosData =
          positions->get(inputPos + posLenAtt->getPositionLength());

      constexpr int startOffset = offsetAtt->startOffset();
      if (startPosData->startOffset == -1) {
        startPosData->startOffset = startOffset;
      } else {
        // Make sure our input isn't messing up offsets:
        assert((startPosData->startOffset == startOffset,
                L"prev startOffset=" +
                    std::to_wstring(startPosData->startOffset) +
                    L" vs new startOffset=" + std::to_wstring(startOffset) +
                    L" inputPos=" + std::to_wstring(inputPos)));
      }

      constexpr int endOffset = offsetAtt->endOffset();
      if (endPosData->endOffset == -1) {
        endPosData->endOffset = endOffset;
      } else {
        // Make sure our input isn't messing up offsets:
        assert((endPosData->endOffset == endOffset,
                L"prev endOffset=" + std::to_wstring(endPosData->endOffset) +
                    L" vs new endOffset=" + std::to_wstring(endOffset) +
                    L" inputPos=" + std::to_wstring(inputPos)));
      }

      tokenPending = true;
    } else {
      end = true;
    }

    return gotToken;
  }

  /** Call this when you are done looking ahead; it will set
   *  the next token to return.  Return the bool back to
   *  the caller. */
  virtual bool nextToken() 
  {
    // System.out.println("  nextToken: tokenPending=" + tokenPending);
    if (DEBUG) {
      std::wcout << L"LTF.nextToken inputPos=" << inputPos << L" outputPos="
                 << outputPos << L" tokenPending=" << tokenPending << std::endl;
    }

    std::shared_ptr<Position> posData = positions->get(outputPos);

    // While loop here in case we have to
    // skip over a hole from the input:
    while (true) {

      // System.out.println("    check buffer @ outputPos=" +
      // outputPos + " inputPos=" + inputPos + " nextRead=" +
      // posData.nextRead + " vs size=" +
      // posData.inputTokens.size());

      // See if we have a previously buffered token to
      // return at the current position:
      if (posData->nextRead < posData->inputTokens.size()) {
        if (DEBUG) {
          std::wcout << L"  return previously buffered token" << std::endl;
        }
        // This position has buffered tokens to serve up:
        if (tokenPending) {
          positions->get(inputPos)->add(captureState());
          tokenPending = false;
        }
        restoreState(positions->get(outputPos).nextState());
        // System.out.println("      return!");
        return true;
      }

      if (inputPos == -1 || outputPos == inputPos) {
        // No more buffered tokens:
        // We may still get input tokens at this position
        // System.out.println("    break buffer");
        if (tokenPending) {
          // Fast path: just return token we had just incr'd,
          // without having captured/restored its state:
          if (DEBUG) {
            std::wcout << L"  pass-through: return pending token" << std::endl;
          }
          tokenPending = false;
          return true;
        } else if (end || !peekToken()) {
          if (DEBUG) {
            std::wcout << L"  END" << std::endl;
          }
          afterPosition();
          if (insertPending) {
            // Subclass inserted a token at this same
            // position:
            if (DEBUG) {
              std::wcout << L"  return inserted token" << std::endl;
            }
            assert(insertedTokenConsistent());
            insertPending = false;
            return true;
          }

          return false;
        }
      } else {
        if (posData->startOffset != -1) {
          // This position had at least one token leaving
          if (DEBUG) {
            std::wcout << L"  call afterPosition" << std::endl;
          }
          afterPosition();
          if (insertPending) {
            // Subclass inserted a token at this same
            // position:
            if (DEBUG) {
              std::wcout << L"  return inserted token" << std::endl;
            }
            assert(insertedTokenConsistent());
            insertPending = false;
            return true;
          }
        }

        // Done with this position; move on:
        outputPos++;
        if (DEBUG) {
          std::wcout << L"  next position: outputPos=" << outputPos
                     << std::endl;
        }
        positions::freeBefore(outputPos);
        posData = positions->get(outputPos);
      }
    }
  }

  // If subclass inserted a token, make sure it had in fact
  // looked ahead enough:
private:
  bool insertedTokenConsistent()
  {
    constexpr int posLen = posLenAtt->getPositionLength();
    std::shared_ptr<Position> *const endPosData =
        positions->get(outputPos + posLen);
    assert(endPosData->endOffset != -1);
    assert((offsetAtt->endOffset() == endPosData->endOffset,
            L"offsetAtt.endOffset=" + std::to_wstring(offsetAtt->endOffset()) +
                L" vs expected=" + std::to_wstring(endPosData->endOffset)));
    return true;
  }

  // TODO: end()?
  // TODO: close()?

public:
  void reset()  override
  {
    TokenFilter::reset();
    positions::reset();
    inputPos = -1;
    outputPos = 0;
    tokenPending = false;
    end = false;
  }

protected:
  std::shared_ptr<LookaheadTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<LookaheadTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
