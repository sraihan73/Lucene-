#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "POS.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ko::dict
{
class Dictionary;
}

namespace org::apache::lucene::analysis::ko::dict
{
class TokenInfoFST;
}
namespace org::apache::lucene::analysis::ko::dict
{
class TokenInfoDictionary;
}
namespace org::apache::lucene::analysis::ko::dict
{
class UnknownDictionary;
}
namespace org::apache::lucene::analysis::ko::dict
{
class ConnectionCosts;
}
namespace org::apache::lucene::analysis::ko::dict
{
class UserDictionary;
}
namespace org::apache::lucene::analysis::ko::dict
{
class CharacterDefinition;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
class BytesReader;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::analysis::util
{
class RollingCharBuffer;
}
namespace org::apache::lucene::analysis::ko
{
class WrappedPositionArray;
}
namespace org::apache::lucene::analysis::ko
{
class Token;
}
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
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis::ko::tokenattributes
{
class PartOfSpeechAttribute;
}
namespace org::apache::lucene::analysis::ko::tokenattributes
{
class ReadingAttribute;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
}
namespace org::apache::lucene::analysis::ko
{
class GraphvizFormatter;
}
namespace org::apache::lucene::analysis::ko
{
class POS;
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
namespace org::apache::lucene::analysis::ko
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharacterDefinition =
    org::apache::lucene::analysis::ko::dict::CharacterDefinition;
using ConnectionCosts =
    org::apache::lucene::analysis::ko::dict::ConnectionCosts;
using Dictionary = org::apache::lucene::analysis::ko::dict::Dictionary;
using TokenInfoDictionary =
    org::apache::lucene::analysis::ko::dict::TokenInfoDictionary;
using TokenInfoFST = org::apache::lucene::analysis::ko::dict::TokenInfoFST;
using UnknownDictionary =
    org::apache::lucene::analysis::ko::dict::UnknownDictionary;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
using PartOfSpeechAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::PartOfSpeechAttribute;
using ReadingAttribute =
    org::apache::lucene::analysis::ko::tokenattributes::ReadingAttribute;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using RollingCharBuffer =
    org::apache::lucene::analysis::util::RollingCharBuffer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using IntsRef = org::apache::lucene::util::IntsRef;
using FST = org::apache::lucene::util::fst::FST;

/**
 * Tokenizer for Korean that uses morphological analysis.
 * <p>
 * This tokenizer sets a number of additional attributes:
 * <ul>
 *   <li>{@link PartOfSpeechAttribute} containing part-of-speech.
 *   <li>{@link ReadingAttribute} containing reading.
 * </ul>
 * <p>
 * This tokenizer uses a rolling Viterbi search to find the
 * least cost segmentation (path) of the incoming characters.
 * @lucene.experimental
 */
class KoreanTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(KoreanTokenizer)

  /**
   * Token type reflecting the original source of this token
   */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /**
     * Known words from the system dictionary.
     */
    KNOWN,
    /**
     * Unknown words (heuristically segmented).
     */
    UNKNOWN,
    /**
     * Known words from the user dictionary.
     */
    USER
  };

  /**
   * Decompound mode: this determines how the tokenizer handles
   * {@link POS.Type#COMPOUND}, {@link POS.Type#INFLECT} and {@link
   * POS.Type#PREANALYSIS} tokens.
   */
public:
  enum class DecompoundMode {
    GET_CLASS_NAME(DecompoundMode)
    /**
     * No decomposition for compound.
     */
    NONE,

    /**
     * Decompose compounds and discards the original form (default).
     */
    DISCARD,

    /**
     * Decompose compounds and keeps the original form.
     */
    MIXED
  };

  /**
   * Default mode for the decompound of tokens ({@link DecompoundMode#DISCARD}.
   */
public:
  static constexpr DecompoundMode DEFAULT_DECOMPOUND = DecompoundMode::DISCARD;

private:
  static constexpr bool VERBOSE = false;

  // For safety:
  static constexpr int MAX_UNKNOWN_WORD_LENGTH = 1024;
  static constexpr int MAX_BACKTRACE_GAP = 1024;

  const std::shared_ptr<EnumMap<Type, std::shared_ptr<Dictionary>>>
      dictionaryMap =
          std::make_shared<EnumMap<Type, std::shared_ptr<Dictionary>>>(
              Type::typeid);

  const std::shared_ptr<TokenInfoFST> fst;
  const std::shared_ptr<TokenInfoDictionary> dictionary;
  const std::shared_ptr<UnknownDictionary> unkDictionary;
  const std::shared_ptr<ConnectionCosts> costs;
  const std::shared_ptr<UserDictionary> userDictionary;
  const std::shared_ptr<CharacterDefinition> characterDefinition;

  const std::shared_ptr<FST::Arc<int64_t>> arc =
      std::make_shared<FST::Arc<int64_t>>();
  const std::shared_ptr<FST::BytesReader> fstReader;
  const std::shared_ptr<IntsRef> wordIdRef = std::make_shared<IntsRef>();

  const std::shared_ptr<FST::BytesReader> userFSTReader;
  const std::shared_ptr<TokenInfoFST> userFST;

  const DecompoundMode mode;
  const bool outputUnknownUnigrams;

  const std::shared_ptr<RollingCharBuffer> buffer =
      std::make_shared<RollingCharBuffer>();

  const std::shared_ptr<WrappedPositionArray> positions =
      std::make_shared<WrappedPositionArray>();

  // True once we've hit the EOF from the input reader:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool end_ = false;

  // Last absolute position we backtraced from:
  int lastBackTracePos = 0;

  // Next absolute position to process:
  int pos = 0;

  // Already parsed, but not yet passed to caller, tokens:
  const std::deque<std::shared_ptr<Token>> pending =
      std::deque<std::shared_ptr<Token>>();

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLengthAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<PartOfSpeechAttribute> posAtt =
      addAttribute(PartOfSpeechAttribute::typeid);
  const std::shared_ptr<ReadingAttribute> readingAtt =
      addAttribute(ReadingAttribute::typeid);

  /**
   * Creates a new KoreanTokenizer with default parameters.
   * <p>
   * Uses the default AttributeFactory.
   */
public:
  KoreanTokenizer();

  /**
   * Create a new KoreanTokenizer.
   *
   * @param factory the AttributeFactory to use
   * @param userDictionary Optional: if non-null, user dictionary.
   * @param mode Decompound mode.
   * @param outputUnknownUnigrams If true outputs unigrams for unknown words.
   */
  KoreanTokenizer(std::shared_ptr<AttributeFactory> factory,
                  std::shared_ptr<UserDictionary> userDictionary,
                  DecompoundMode mode, bool outputUnknownUnigrams);

private:
  std::shared_ptr<GraphvizFormatter> dotOut;

  /** Expert: set this to produce graphviz (dot) output of
   *  the Viterbi lattice */
public:
  void setGraphvizFormatter(std::shared_ptr<GraphvizFormatter> dotOut);

  virtual ~KoreanTokenizer();

  void reset()  override;

private:
  void resetState();

public:
  void end()  override;

  // Holds all back pointers arriving to this position:
public:
  class Position final : public std::enable_shared_from_this<Position>
  {
    GET_CLASS_NAME(Position)

  public:
    int pos = 0;

    int count = 0;

    // maybe single int array * 5?
    std::deque<int> costs = std::deque<int>(8);
    std::deque<int> lastRightID = std::deque<int>(8);
    std::deque<int> backPos = std::deque<int>(8);
    std::deque<int> backWordPos = std::deque<int>(8);
    std::deque<int> backIndex = std::deque<int>(8);
    std::deque<int> backID = std::deque<int>(8);
    std::deque<Type> backType = std::deque<Type>(8);

    void grow();

    void add(int cost, int lastRightID, int backPos, int backRPos,
             int backIndex, int backID, Type backType);

    void reset();
  };

  /**
   * Returns the space penalty associated with the provided {@link POS.Tag}.
   *
   * @param leftPOS the left part of speech of the current token.
   * @param numSpaces the number of spaces before the current token.
   */
private:
  int computeSpacePenalty(POS::Tag leftPOS, int numSpaces);

  void add(std::shared_ptr<Dictionary> dict,
           std::shared_ptr<Position> fromPosData, int wordPos, int endPos,
           int wordID, Type type) ;

public:
  bool incrementToken()  override;

  // TODO: make generic'd version of this "circular array"?
  // It's a bit tricky because we do things to the Position
  // (eg, set .pos = N on reuse)...
public:
  class WrappedPositionArray final
      : public std::enable_shared_from_this<WrappedPositionArray>
  {
    GET_CLASS_NAME(WrappedPositionArray)
  private:
    std::deque<std::shared_ptr<Position>> positions =
        std::deque<std::shared_ptr<Position>>(8);

  public:
    WrappedPositionArray();

    // Next array index to write to in positions:
  private:
    int nextWrite = 0;

    // Next position to write:
    int nextPos = 0;

    // How many valid Position instances are held in the
    // positions array:
    int count = 0;

  public:
    void reset();

    /** Get Position instance for this absolute position;
     *  this is allowed to be arbitrarily far "in the
     *  future" but cannot be before the last freeBefore. */
    std::shared_ptr<Position> get(int pos);

    int getNextPos();

    // For assert:
  private:
    bool inBounds(int pos);

    int getIndex(int pos);

  public:
    void freeBefore(int pos);
  };

  /* Incrementally parse some more characters.  This runs
   * the viterbi search forwards "enough" so that we
   * generate some more tokens.  How much forward depends on
   * the chars coming in, since some chars could cause
   * longer-lasting ambiguity in the parsing.  Once the
   * ambiguity is resolved, then we back trace, produce
   * the pending tokens, and return. */
private:
  void parse() ;

  // the pendi