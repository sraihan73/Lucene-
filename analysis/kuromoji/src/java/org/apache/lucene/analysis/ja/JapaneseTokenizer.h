#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class Dictionary;
}

namespace org::apache::lucene::analysis::ja::dict
{
class TokenInfoFST;
}
namespace org::apache::lucene::analysis::ja::dict
{
class TokenInfoDictionary;
}
namespace org::apache::lucene::analysis::ja::dict
{
class UnknownDictionary;
}
namespace org::apache::lucene::analysis::ja::dict
{
class ConnectionCosts;
}
namespace org::apache::lucene::analysis::ja::dict
{
class UserDictionary;
}
namespace org::apache::lucene::analysis::ja::dict
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
namespace org::apache::lucene::analysis::ja
{
class WrappedPositionArray;
}
namespace org::apache::lucene::analysis::ja
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
namespace org::apache::lucene::util
{
class AttributeFactory;
}
namespace org::apache::lucene::analysis::ja
{
class GraphvizFormatter;
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
namespace org::apache::lucene::analysis::ja
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharacterDefinition =
    org::apache::lucene::analysis::ja::dict::CharacterDefinition;
using ConnectionCosts =
    org::apache::lucene::analysis::ja::dict::ConnectionCosts;
using Dictionary = org::apache::lucene::analysis::ja::dict::Dictionary;
using TokenInfoDictionary =
    org::apache::lucene::analysis::ja::dict::TokenInfoDictionary;
using TokenInfoFST = org::apache::lucene::analysis::ja::dict::TokenInfoFST;
using UnknownDictionary =
    org::apache::lucene::analysis::ja::dict::UnknownDictionary;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using namespace org::apache::lucene::analysis::ja::tokenattributes;
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

// TODO: somehow factor out a reusable viterbi search here,
// so other decompounders/tokenizers can reuse...

/**
 * Tokenizer for Japanese that uses morphological analysis.
 * <p>
 * This tokenizer sets a number of additional attributes:
 * <ul>
 *   <li>{@link BaseFormAttribute} containing base form for inflected
 *       adjectives and verbs.
 *   <li>{@link PartOfSpeechAttribute} containing part-of-speech.
 *   <li>{@link ReadingAttribute} containing reading and pronunciation.
 *   <li>{@link InflectionAttribute} containing additional part-of-speech
 *       information for inflected forms.
 * </ul>
 * <p>
 * This tokenizer uses a rolling Viterbi search to find the
 * least cost segmentation (path) of the incoming characters.
 * For tokens that appear to be compound (&gt; length 2 for all
 * Kanji, or &gt; length 7 for non-Kanji), we see if there is a
 * 2nd best segmentation of that token after applying
 * penalties to the long tokens.  If so, and the Mode is
 * {@link Mode#SEARCH}, we output the alternate segmentation
 * as well.
 */
class JapaneseTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(JapaneseTokenizer)

  /**
   * Tokenization mode: this determines how the tokenizer handles
   * compound and unknown words.
   */
public:
  enum class Mode {
    GET_CLASS_NAME(Mode)
    /**
     * Ordinary segmentation: no decomposition for compounds,
     */
    NORMAL,

    /**
     * Segmentation geared towards search: this includes a
     * decompounding process for long nouns, also including
     * the full compound token as a synonym.
     */
    SEARCH,

    /**
     * Extended mode outputs unigrams for unknown words.
     * @lucene.experimental
     */
    EXTENDED
  };

  /**
   * Default tokenization mode. Currently this is {@link Mode#SEARCH}.
   */
public:
  static constexpr Mode DEFAULT_MODE = Mode::SEARCH;

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

private:
  static constexpr bool VERBOSE = false;

  static constexpr int SEARCH_MODE_KANJI_LENGTH = 2;

  static constexpr int SEARCH_MODE_OTHER_LENGTH =
      7; // Must be >= SEARCH_MODE_KANJI_LENGTH

  static constexpr int SEARCH_MODE_KANJI_PENALTY = 3000;

  static constexpr int SEARCH_MODE_OTHER_PENALTY = 1700;

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

  const std::shared_ptr<RollingCharBuffer> buffer =
      std::make_shared<RollingCharBuffer>();

  const std::shared_ptr<WrappedPositionArray> positions =
      std::make_shared<WrappedPositionArray>();

  const bool discardPunctuation;
  const bool searchMode;
  const bool extendedMode;
  const bool outputCompounds;
  bool outputNBest = false;

  // Allowable cost difference for N-best output:
  int nBestCost = 0;

  // True once we've hit the EOF from the input reader:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool end_ = false;

  // Last absolute position we backtraced from:
  int lastBackTracePos = 0;

  // Position of last token we returned; we use this to
  // figure out whether to set posIncr to 0 or 1:
  int lastTokenPos = 0;

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
  const std::shared_ptr<BaseFormAttribute> basicFormAtt =
      addAttribute(BaseFormAttribute::typeid);
  const std::shared_ptr<PartOfSpeechAttribute> posAtt =
      addAttribute(PartOfSpeechAttribute::typeid);
  const std::shared_ptr<ReadingAttribute> readingAtt =
      addAttribute(ReadingAttribute::typeid);
  const std::shared_ptr<InflectionAttribute> inflectionAtt =
      addAttribute(InflectionAttribute::typeid);

  /**
   * Create a new JapaneseTokenizer.
   * <p>
   * Uses the default AttributeFactory.
   *
   * @param userDictionary Optional: if non-null, user dictionary.
   * @param discardPunctuation true if punctuation tokens should be dropped from
   * the output.
   * @param mode tokenization mode.
   */
public:
  JapaneseTokenizer(std::shared_ptr<UserDictionary> userDictionary,
                    bool discardPunctuation, Mode mode);

  /**
   * Create a new JapaneseTokenizer.
   *
   * @param factory the AttributeFactory to use
   * @param userDictionary Optional: if non-null, user dictionary.
   * @param discardPunctuation true if punctuation tokens should be dropped from
   * the output.
   * @param mode tokenization mode.
   */
  JapaneseTokenizer(std::shared_ptr<AttributeFactory> factory,
                    std::shared_ptr<UserDictionary> userDictionary,
                    bool discardPunctuation, Mode mode);

private:
  std::shared_ptr<GraphvizFormatter> dotOut;

  /** Expert: set this to produce graphviz (dot) output of
   *  the Viterbi lattice */
public:
  void setGraphvizFormatter(std::shared_ptr<GraphvizFormatter> dotOut);

  virtual ~JapaneseTokenizer();

  void reset()  override;

private:
  void resetState();

public:
  void end()  override;

  // Returns the added cost that a 2nd best segmentation is
  // allowed to have.  Ie, if we see path with cost X,
  // ending in a compound word, and this method returns
  // threshold > 0, then we will also find the 2nd best
  // segmentation and if its path score is within this
  // threshold of X, we'll include it in the output:
private:
  int computeSecondBestThreshold(int pos, int length) ;

  int computePenalty(int pos, int length) ;

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
    std::deque<int> backIndex = std::deque<int>(8);
    std::deque<int> backID = std::deque<int>(8);
    std::deque<Type> backType = std::deque<Type>(8);

    // Only used when finding 2nd best segmentation under a
    // too-long token:
    int forwardCount = 0;
    std::deque<int> forwardPos = std::deque<int>(8);
    std::deque<int> forwardID = std::deque<int>(8);
    std::deque<int> forwardIndex = std::deque<int>(8);
    std::deque<Type> forwardType = std::deque<Type>(8);

    void grow();

    void growForward();

    void add(int cost, int lastRightID, int backPos, int backIndex, int backID,
             Type backType);

    void addForward(int forwardPos, int forwardIndex, int forwardID,
                    Type forwardType);

    void reset();
  };

private:
  void add(std::shared_ptr<Dictionary> dict,
           std::shared_ptr<Position> fromPosData, int endPos, int wordID,
           Type type, bool addPenalty) ;

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

  // Eliminates arcs from the lattice that are compound
  // tokens (have a penalty) or are not congruent with the
  // compound token we've matched (ie, span across the
  // startPos).  This should be fairly efficient, because we
  // just keep the already intersected structure of the
  // graph, eg we don't have to consult the FSTs again:

  void pruneAndRescore(int startPos, int endPos,
                       int bestStartIDX) ;

  // yet another lattice data structure
private:
  class Lattice final : public std::enable_shared_from_this<Lattice>
  {
    GET_CLASS_NAME(Lattice)
  public:
    std::deque<wchar_t> fragment;
    std::shared_ptr<EnumMap<Type, std::shared_ptr<Dictionary>>> dictionaryMap;
    bool useEOS = false;

    int rootCapacity = 0;
    int rootSize = 0;
    int rootBase = 0;

    // root pointers of node chain by leftChain_ that have same start offset.
    std::deque<int> lRoot;
    // root pointers of node chain by rightChain_ that have same end offset.
    std::deque<int> rRoot;

    int capacity = 0;
    int nodeCount = 0;

    // The variables below are elements of lattice node that indexed by node
    // number.
    std::deque<Type> nodeDicType;
    std::deque<int> nodeWordID;
    // nodeMark - -1:excluded, 0:unused, 1:bestpath, 2:2-best-path, ...
    // N:N-best-path
    std::deque<int> nodeMark;
    std::deque<int> nodeLeftID;
    std::deque<int> nodeRightID;
    std::deque<int> nodeWordCost;
    std::deque<int> nodeLeftCost;
    std::deque<int> nodeRightCost;
    // nodeLeftNode, nodeRightNode - are left/right node number with minimum
    // cost path.
    std::deque<int> nodeLeftNode;
    std::deque<int> nodeRightNode;
    // nodeLeft, nodeRight - start/end offset
    std::deque<int> nodeLeft;
    std::deque<int> nodeRight;
    std::deque<int> nodeLeftChain;
    std::deque<int> nodeRightChain;

  private:
    void setupRoot(int baseOffset, int lastOffset);

    // Reserve at least N nodes.
    void reserve(int n);

    void setupNodePool(int n);

    int addNode(Type dicType, int wordID, int left, int right);

    // Sum of positions.get(i).count in [beg, end) range.
    // using stream:
    //   return IntStream.range(beg, end).map_obj(i ->
    //   positions.get(i).count).sum();
    int positionCount(std::shared_ptr<WrappedPositionArray> positions, int beg,
                      int end);

  public:
    void setup(std::deque<wchar_t> &fragment,
               std::shared_ptr<EnumMap<Type, std::shared_ptr<Dictionary>>>
                   dictionaryMap,
               std::shared_ptr<WrappedPositionArray> positions, int prevOffset,
               int endOffset, bool useEOS);

    // set mark = -1 for unreachable nodes.
    void markUnreachable();

    int connectionCost(std::shared_ptr<ConnectionCosts> costs, int left,
                       int right);

    void calcLeftCost(std::shared_ptr<ConnectionCosts> costs);

    void calcRightCost(std::shared_ptr<ConnectionCosts> costs);

    // Mark all nodes that have same text and different par-of-speech or
    // reading.
    void markSameSpanNode(int refNode, int value);

    std::deque<int> bestPathNodeList();

  private:
    int cost(int node);

  public:
    std::deque<int> nBestNodeList(int N);

    int bestCost();

    int probeDelta(int start, int end);

    void debugPrint();
  };

private:
  std::shared_ptr<Lattice> lattice = nullptr;

  void registerNode(int node, std::deque<wchar_t> &fragment);

  // Sort pending tokens, and set position increment values.
  void fixupPendingList();

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<Token>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<JapaneseTokenizer> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<JapaneseTokenizer> outerInstance);

    int compare(std::shared_ptr<Token> a, std::shared_ptr<Token> b);
  };

private:
  int probeDelta(const std::wstring &inText,
                 const std::wstring &requiredToken) ;

public:
  int calcNBestCost(const std::wstring &examples);

  void setNBestCost(int value);

private:
  void backtraceNBest(std::shared_ptr<Position> endPosData,
                      bool const useEOS) ;

  // Backtrace from the provided position, back to the last
  // time we back-traced, accumulating the resulting tokens to
  // the pending deque.  The pending deque is then in-reverse
  // (last token should be returned first).
  void backtrace(std::shared_ptr<Position> endPosData,
                 int const fromIDX) ;

public:
  std::shared_ptr<Dictionary> getDict(Type type);

private:
  static bool isPunctuation(wchar_t ch);

protected:
  std::shared_ptr<JapaneseTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
