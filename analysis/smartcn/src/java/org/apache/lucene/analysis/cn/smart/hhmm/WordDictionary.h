#pragma once
#include "AbstractDictionary.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::cn::smart::hhmm
{

/**
 * SmartChineseAnalyzer Word Dictionary
 * @lucene.experimental
 */
class WordDictionary : public AbstractDictionary
{
  GET_CLASS_NAME(WordDictionary)

private:
  WordDictionary();

  static std::shared_ptr<WordDictionary> singleInstance;

  /**
   * Large prime number for hash function
   */
public:
  static constexpr int PRIME_INDEX_LENGTH = 12071;

  /**
   * wordIndexTable guarantees to hash all Chinese characters in Unicode into
   * PRIME_INDEX_LENGTH array. There will be conflict, but in reality this
   * program only handles the 6768 characters found in GB2312 plus some
   * ASCII characters. Therefore in order to guarantee better precision, it is
   * necessary to retain the original symbol in the charIndexTable.
   */
private:
  std::deque<short> wordIndexTable;

  std::deque<wchar_t> charIndexTable;

  /**
   * To avoid taking too much space, the data structure needed to store the
   * lexicon requires two multidimensional arrays to store word and frequency.
   * Each word is placed in a char[]. Each char represents a Chinese char or
   * other symbol.  Each frequency is put into an int. These two arrays
   * correspond to each other one-to-one. Therefore, one can use
   * wordItem_charArrayTable[i][j] to look up word from lexicon, and
   * wordItem_frequencyTable[i][j] to look up the corresponding frequency.
   */
  std::deque<std::deque<std::deque<wchar_t>>> wordItem_charArrayTable;

  std::deque<std::deque<int>> wordItem_frequencyTable;

  // static Logger log = Logger.getLogger(WordDictionary.class);

  /**
   * Get the singleton dictionary instance.
   * @return singleton
   */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<WordDictionary> getInstance();

  /**
   * Attempt to load dictionary from provided directory, first trying
   * coredict.mem, failing back on coredict.dct
   *
   * @param dctFileRoot path to dictionary directory
   */
  virtual void load(const std::wstring &dctFileRoot);

  /**
   * Load coredict.mem internally from the jar file.
   *
   * @throws IOException If there is a low-level I/O error.
   */
  virtual void load() ;

private:
  bool loadFromObj(std::shared_ptr<Path> serialObj);

  void loadFromObjectInputStream(
      std::shared_ptr<InputStream>
          serialObjectInputStream) ;

  void saveToObj(std::shared_ptr<Path> serialObj);

  /**
   * Load the datafile into this WordDictionary
   *
   * @param dctFilePath path to word dictionary (coredict.dct)
   * @return number of words read
   * @throws IOException If there is a low-level I/O error.
   */
  int loadMainDataFromFile(const std::wstring &dctFilePath) ;

  /**
   * The original lexicon puts all information with punctuation into a
   * chart (from 1 to 3755). Here it then gets expanded, separately being
   * placed into the chart that has the corresponding symbol.
   */
  void expandDelimiterData();

  /*
   * since we aren't doing POS-tagging, merge the frequencies for entries of the
   * same word (with different POS)
   */
  void mergeSameWords();

  void sortEachItems();

  /*
   * Calculate character c's position in hash table,
   * then initialize the value of that position in the address table.
   */
  bool setTableIndex(wchar_t c, int j);

  short getAvaliableTableIndex(wchar_t c);

  short getWordItemTableIndex(wchar_t c);

  /**
   * Look up the text string corresponding with the word char array,
   * and return the position of the word deque.
   *
   * @param knownHashIndex already figure out position of the first word
   *   symbol charArray[0] in hash table. If not calculated yet, can be
   *   replaced with function int findInTable(char[] charArray).
   * @param charArray look up the char array corresponding with the word.
   * @return word location in word array.  If not found, then return -1.
   */
  int findInTable(short knownHashIndex, std::deque<wchar_t> &charArray);

  /**
   * Find the first word in the dictionary that starts with the supplied prefix
   *
   * @see #getPrefixMatch(char[], int)
   * @param charArray input prefix
   * @return index of word, or -1 if not found
   */
public:
  virtual int getPrefixMatch(std::deque<wchar_t> &charArray);

  /**
   * Find the nth word in the dictionary that starts with the supplied prefix
   *
   * @see #getPrefixMatch(char[])
   * @param charArray input prefix
   * @param knownStart relative position in the dictionary to start
   * @return index of word, or -1 if not found
   */
  virtual int getPrefixMatch(std::deque<wchar_t> &charArray, int knownStart);

  /**
   * Get the frequency of a word from the dictionary
   *
   * @param charArray input word
   * @return word frequency, or zero if the word is not found
   */
  virtual int getFrequency(std::deque<wchar_t> &charArray);

  /**
   * Return true if the dictionary entry at itemIndex for table charArray[0] is
   * charArray
   *
   * @param charArray input word
   * @param itemIndex item index for table charArray[0]
   * @return true if the entry exists
   */
  virtual bool isEqual(std::deque<wchar_t> &charArray, int itemIndex);

protected:
  std::shared_ptr<WordDictionary> shared_from_this()
  {
    return std::static_pointer_cast<WordDictionary>(
        AbstractDictionary::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/cn/smart/hhmm/
