#pragma once
#include "PatternConsumer.h"
#include "TernaryTree.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/ByteVector.h"

#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/TernaryTree.h"
#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/Hyphenation.h"

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
namespace org::apache::lucene::analysis::compound::hyphenation
{

using org::xml::sax::InputSource;

/**
 * This tree structure stores the hyphenation patterns in an efficient way for
 * fast lookup. It provides the provides the method to hyphenate a word.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class HyphenationTree : public TernaryTree, public PatternConsumer
{
  GET_CLASS_NAME(HyphenationTree)

  /**
   * value space: stores the interletter values
   */
protected:
  std::shared_ptr<ByteVector> vspace;

  /**
   * This map_obj stores hyphenation exceptions
   */
  std::unordered_map<std::wstring, std::deque<std::any>> stoplist;

  /**
   * This map_obj stores the character classes
   */
  std::shared_ptr<TernaryTree> classmap;

  /**
   * Temporary map_obj to store interletter values on pattern loading.
   */
private:
  std::shared_ptr<TernaryTree> ivalues;

public:
  HyphenationTree();

  /**
   * Packs the values by storing them in 4 bits, two values into a byte Values
   * range is from 0 to 9. We use zero as terminator, so we'll add 1 to the
   * value.
   *
   * @param values a string of digits from '0' to '9' representing the
   *        interletter values.
   * @return the index into the vspace array where the packed values are stored.
   */
protected:
  virtual int packValues(const std::wstring &values);

  virtual std::wstring unpackValues(int k);

  /**
   * Read hyphenation patterns from an XML file.
   *
   * @param source the InputSource for the file
   * @throws IOException In case the parsing fails
   */
public:
  virtual void
  loadPatterns(std::shared_ptr<InputSource> source) ;

  virtual std::wstring findPattern(const std::wstring &pat);

  /**
   * std::wstring compare, returns 0 if equal or t is a substring of s
   */
protected:
  virtual int hstrcmp(std::deque<wchar_t> &s, int si, std::deque<wchar_t> &t,
                      int ti);

  virtual std::deque<char> getValues(int k);

  /**
   * <p>
   * Search for all possible partial matches of word starting at index an update
   * interletter values. In other words, it does something like:
   * </p>
   * <code>
   * for(i=0; i&lt;patterns.length; i++) {
   * if ( word.substring(index).startsWidth(patterns[i]) )
   * update_interletter_values(patterns[i]);
   * }
   * </code>
   * <p>
   * But it is done in an efficient way since the patterns are stored in a
   * ternary tree. In fact, this is the whole purpose of having the tree: doing
   * this search without having to test every single pattern. The number of
   * patterns for languages such as English range from 4000 to 10000. Thus,
   * doing thousands of string comparisons for each word to hyphenate would be
   * really slow without the tree. The tradeoff is memory, but using a ternary
   * tree instead of a trie, almost halves the the memory used by Lout or TeX.
   * It's also faster than using a hash table
   * </p>
   *
   * @param word null terminated word to match
   * @param index start index from word
   * @param il interletter values array to update
   */
  virtual void searchPatterns(std::deque<wchar_t> &word, int index,
                              std::deque<char> &il);

  /**
   * Hyphenate word and return a Hyphenation object.
   *
   * @param word the word to be hyphenated
   * @param remainCharCount Minimum number of characters allowed before the
   *        hyphenation point.
   * @param pushCharCount Minimum number of characters allowed after the
   *        hyphenation point.
   * @return a {@link Hyphenation Hyphenation} object representing the
   *         hyphenated word or null if word is not hyphenated.
   */
public:
  virtual std::shared_ptr<Hyphenation>
  hyphenate(const std::wstring &word, int remainCharCount, int pushCharCount);

  /**
   * w = "****nnllllllnnn*****", where n is a non-letter, l is a letter, all n
   * may be absent, the first n is at offset, the first l is at offset +
   * iIgnoreAtBeginning; word = ".llllll.'\0'***", where all l in w are copied
   * into word. In the first part of the routine len = w.length, in the second
   * part of the routine len = word.length. Three indices are used: index(w),
   * the index in w, index(word), the index in word, letterindex(word), the
   * index in the letter part of word. The following relations exist: index(w) =
   * offset + i - 1 index(word) = i - iIgnoreAtBeginning letterindex(word) =
   * index(word) - 1 (see first loop). It follows that: index(w) - index(word) =
   * offset - 1 + iIgnoreAtBeginning index(w) = letterindex(word) + offset +
   * iIgnoreAtBeginning
   */

  /**
   * Hyphenate word and return an array of hyphenation points.
   *
   * @param w char array that contains the word
   * @param offset Offset to first character in word
   * @param len Length of word
   * @param remainCharCount Minimum number of characters allowed before the
   *        hyphenation point.
   * @param pushCharCount Minimum number of characters allowed after the
   *        hyphenation point.
   * @return a {@link Hyphenation Hyphenation} object representing the
   *         hyphenated word or null if word is not hyphenated.
   */
  virtual std::shared_ptr<Hyphenation> hyphenate(std::deque<wchar_t> &w,
                                                 int offset, int len,
                                                 int remainCharCount,
                                                 int pushCharCount);

  /**
   * Add a character class to the tree. It is used by
   * {@link PatternParser PatternParser} as callback to add character classes.
GET_CLASS_NAME(to)
   * Character classes define the valid word characters for hyphenation. If a
   * word contains a character not defined in any of the classes, it is not
   * hyphenated. It also defines a way to normalize the characters in order to
   * compare them with the stored patterns. Usually pattern files use only lower
   * case characters, in this case a class for letter 'a', for example, should
   * be defined as "aA", the first character being the normalization char.
   */
  void addClass(const std::wstring &chargroup) override;

  /**
   * Add an exception to the tree. It is used by
   * {@link PatternParser PatternParser} class as callback to store the
   * hyphenation exceptions.
   *
   * @param word normalized word
   * @param hyphenatedword a deque of alternating strings and
   *        {@link Hyphen hyphen} objects.
   */
  void addException(const std::wstring &word,
                    std::deque<std::any> &hyphenatedword) override;

  /**
   * Add a pattern to the tree. Mainly, to be used by
   * {@link PatternParser PatternParser} class as callback to add a pattern to
   * the tree.
   *
   * @param pattern the hyphenation pattern
   * @param ivalue interletter weight values indicating the desirability and
   *        priority of hyphenating at a given point within the pattern. It
   *        should contain only digit characters. (i.e. '0' to '9').
   */
  void addPattern(const std::wstring &pattern,
                  const std::wstring &ivalue) override;

  void printStats(std::shared_ptr<PrintStream> out) override;

protected:
  std::shared_ptr<HyphenationTree> shared_from_this()
  {
    return std::static_pointer_cast<HyphenationTree>(
        TernaryTree::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
