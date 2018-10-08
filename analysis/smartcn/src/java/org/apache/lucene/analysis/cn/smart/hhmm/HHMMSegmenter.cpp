using namespace std;

#include "HHMMSegmenter.h"
#include "../CharType.h"
#include "../Utility.h"
#include "../WordType.h"
#include "BiSegGraph.h"
#include "SegGraph.h"
#include "SegToken.h"
#include "WordDictionary.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{
using CharType = org::apache::lucene::analysis::cn::smart::CharType;
using Utility = org::apache::lucene::analysis::cn::smart::Utility;
using WordType = org::apache::lucene::analysis::cn::smart::WordType;
shared_ptr<WordDictionary> HHMMSegmenter::wordDict =
    WordDictionary::getInstance();

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") private SegGraph
// createSegGraph(std::wstring sentence)
shared_ptr<SegGraph> HHMMSegmenter::createSegGraph(const wstring &sentence)
{
  int i = 0, j;
  int length = sentence.length();
  int foundIndex;
  std::deque<int> charTypeArray = getCharTypes(sentence);
  shared_ptr<StringBuilder> wordBuf = make_shared<StringBuilder>();
  shared_ptr<SegToken> token;
  int frequency = 0; // the number of times word appears.
  bool hasFullWidth;
  int wordType;
  std::deque<wchar_t> charArray;

  shared_ptr<SegGraph> segGraph = make_shared<SegGraph>();
  while (i < length) {
    hasFullWidth = false;
    switch (charTypeArray[i]) {
    case CharType::SPACE_LIKE:
      i++;
      break;
    case CharType::SURROGATE: {
      int state = Character::codePointAt(sentence, i);
      int count = Character::charCount(state);
      charArray = std::deque<wchar_t>(count);
      sentence.getChars(i, i + count, charArray, 0);
      token = make_shared<SegToken>(charArray, i, i + count,
                                    WordType::CHINESE_WORD, 0);
      segGraph->addToken(token);
      i += count;
      break;
    }
    case CharType::HANZI:
      j = i + 1;
      wordBuf->remove(0, wordBuf->length());
      // It doesn't matter if a single Chinese character (Hanzi) can form a
      // phrase or not, it will store that single Chinese character (Hanzi) in
      // the SegGraph.  Otherwise, it will cause word division.
      wordBuf->append(sentence[i]);
      charArray = std::deque<wchar_t>{sentence[i]};
      frequency = wordDict->getFrequency(charArray);
      token = make_shared<SegToken>(charArray, i, j, WordType::CHINESE_WORD,
                                    frequency);
      segGraph->addToken(token);

      foundIndex = wordDict->getPrefixMatch(charArray);
      while (j <= length && foundIndex != -1) {
        if (wordDict->isEqual(charArray, foundIndex) && charArray.size() > 1) {
          // It is the phrase we are looking for; In other words, we have found
          // a phrase SegToken from i to j.  It is not a monosyllabic word
          // (single word).
          frequency = wordDict->getFrequency(charArray);
          token = make_shared<SegToken>(charArray, i, j, WordType::CHINESE_WORD,
                                        frequency);
          segGraph->addToken(token);
        }

        while (j < length && charTypeArray[j] == CharType::SPACE_LIKE) {
          j++;
        }

        if (j < length && charTypeArray[j] == CharType::HANZI) {
          wordBuf->append(sentence[j]);
          charArray = std::deque<wchar_t>(wordBuf->length());
          wordBuf->getChars(0, charArray.size(), charArray, 0);
          // idArray has been found (foundWordIndex!=-1) as a prefix before.
          // Therefore, idArray after it has been lengthened can only appear
          // after foundWordIndex. So start searching after foundWordIndex.
          foundIndex = wordDict->getPrefixMatch(charArray, foundIndex);
          j++;
        } else {
          break;
        }
      }
      i++;
      break;
    case CharType::FULLWIDTH_LETTER:
      hasFullWidth = true; // intentional fallthrough
    case CharType::LETTER:
      j = i + 1;
      while (j < length && (charTypeArray[j] == CharType::LETTER ||
                            charTypeArray[j] == CharType::FULLWIDTH_LETTER)) {
        if (charTypeArray[j] == CharType::FULLWIDTH_LETTER) {
          hasFullWidth = true;
        }
        j++;
      }
      // Found a Token from i to j. Type is LETTER char string.
      charArray = Utility::STRING_CHAR_ARRAY;
      frequency = wordDict->getFrequency(charArray);
      wordType = hasFullWidth ? WordType::FULLWIDTH_STRING : WordType::STRING;
      token = make_shared<SegToken>(charArray, i, j, wordType, frequency);
      segGraph->addToken(token);
      i = j;
      break;
    case CharType::FULLWIDTH_DIGIT:
      hasFullWidth = true; // intentional fallthrough
    case CharType::DIGIT:
      j = i + 1;
      while (j < length && (charTypeArray[j] == CharType::DIGIT ||
                            charTypeArray[j] == CharType::FULLWIDTH_DIGIT)) {
        if (charTypeArray[j] == CharType::FULLWIDTH_DIGIT) {
          hasFullWidth = true;
        }
        j++;
      }
      // Found a Token from i to j. Type is NUMBER char string.
      charArray = Utility::NUMBER_CHAR_ARRAY;
      frequency = wordDict->getFrequency(charArray);
      wordType = hasFullWidth ? WordType::FULLWIDTH_NUMBER : WordType::NUMBER;
      token = make_shared<SegToken>(charArray, i, j, wordType, frequency);
      segGraph->addToken(token);
      i = j;
      break;
    case CharType::DELIMITER:
      j = i + 1;
      // No need to search the weight for the punctuation.  Picking the highest
      // frequency will work.
      frequency = Utility::MAX_FREQUENCE;
      charArray = std::deque<wchar_t>{sentence[i]};
      token = make_shared<SegToken>(charArray, i, j, WordType::DELIMITER,
                                    frequency);
      segGraph->addToken(token);
      i = j;
      break;
    default:
      j = i + 1;
      // Treat the unrecognized char symbol as unknown string.
      // For example, any symbol not in GB2312 is treated as one of these.
      charArray = Utility::STRING_CHAR_ARRAY;
      frequency = wordDict->getFrequency(charArray);
      token =
          make_shared<SegToken>(charArray, i, j, WordType::STRING, frequency);
      segGraph->addToken(token);
      i = j;
      break;
    }
  }

  // Add two more Tokens: "beginning xx beginning"
  charArray = Utility::START_CHAR_ARRAY;
  frequency = wordDict->getFrequency(charArray);
  token = make_shared<SegToken>(charArray, -1, 0, WordType::SENTENCE_BEGIN,
                                frequency);
  segGraph->addToken(token);

  // "end xx end"
  charArray = Utility::END_CHAR_ARRAY;
  frequency = wordDict->getFrequency(charArray);
  token = make_shared<SegToken>(charArray, length, length + 1,
                                WordType::SENTENCE_END, frequency);
  segGraph->addToken(token);

  return segGraph;
}

std::deque<int> HHMMSegmenter::getCharTypes(const wstring &sentence)
{
  int length = sentence.length();
  std::deque<int> charTypeArray(length);
  // the type of each character by position
  for (int i = 0; i < length; i++) {
    charTypeArray[i] = Utility::getCharType(sentence[i]);
  }

  return charTypeArray;
}

deque<std::shared_ptr<SegToken>>
HHMMSegmenter::process(const wstring &sentence)
{
  shared_ptr<SegGraph> segGraph = createSegGraph(sentence);
  shared_ptr<BiSegGraph> biSegGraph = make_shared<BiSegGraph>(segGraph);
  deque<std::shared_ptr<SegToken>> shortPath = biSegGraph->getShortPath();
  return shortPath;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm