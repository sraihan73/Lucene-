#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
}

namespace org::apache::lucene::analysis
{
template <typename V>
class CharArrayMap;
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

/**
 * Loader for text files that represent a deque of stopwords.
 *
 * @see IOUtils to obtain {@link Reader} instances
 * @lucene.internal
 */
class WordlistLoader : public std::enable_shared_from_this<WordlistLoader>
{
  GET_CLASS_NAME(WordlistLoader)

private:
  static constexpr int INITIAL_CAPACITY = 16;

  /** no instance */
  WordlistLoader();

  /**
   * Reads lines from a Reader and adds every line as an entry to a CharArraySet
   * (omitting leading and trailing whitespace). Every line of the Reader should
   * contain only one word. The words need to be in lowercase if you make use of
   * an Analyzer which uses LowerCaseFilter (like StandardAnalyzer).
   *
   * @param reader Reader containing the wordlist
   * @param result the {@link CharArraySet} to fill with the readers words
   * @return the given {@link CharArraySet} with the reader's words
   */
public:
  static std::shared_ptr<CharArraySet>
  getWordSet(std::shared_ptr<Reader> reader,
             std::shared_ptr<CharArraySet> result) ;

  /**
   * Reads lines from a Reader and adds every line as an entry to a CharArraySet
   * (omitting leading and trailing whitespace). Every line of the Reader should
   * contain only one word. The words need to be in lowercase if you make use of
   * an Analyzer which uses LowerCaseFilter (like StandardAnalyzer).
   *
   * @param reader Reader containing the wordlist
   * @return A {@link CharArraySet} with the reader's words
   */
  static std::shared_ptr<CharArraySet>
  getWordSet(std::shared_ptr<Reader> reader) ;

  /**
   * Reads lines from a Reader and adds every non-comment line as an entry to a
   * CharArraySet (omitting leading and trailing whitespace). Every line of the
   * Reader should contain only one word. The words need to be in lowercase if
   * you make use of an Analyzer which uses LowerCaseFilter (like
   * StandardAnalyzer).
   *
   * @param reader Reader containing the wordlist
   * @param comment The string representing a comment.
   * @return A CharArraySet with the reader's words
   */
  static std::shared_ptr<CharArraySet>
  getWordSet(std::shared_ptr<Reader> reader,
             const std::wstring &comment) ;

  /**
   * Reads lines from a Reader and adds every non-comment line as an entry to a
   * CharArraySet (omitting leading and trailing whitespace). Every line of the
   * Reader should contain only one word. The words need to be in lowercase if
   * you make use of an Analyzer which uses LowerCaseFilter (like
   * StandardAnalyzer).
   *
   * @param reader Reader containing the wordlist
   * @param comment The string representing a comment.
   * @param result the {@link CharArraySet} to fill with the readers words
   * @return the given {@link CharArraySet} with the reader's words
   */
  static std::shared_ptr<CharArraySet>
  getWordSet(std::shared_ptr<Reader> reader, const std::wstring &comment,
             std::shared_ptr<CharArraySet> result) ;

  /**
   * Reads stopwords from a stopword deque in Snowball format.
   * <p>
   * The snowball format is the following:
   * <ul>
   * <li>Lines may contain multiple words separated by whitespace.
   * <li>The comment character is the vertical line (&#124;).
   * <li>Lines may contain trailing comments.
   * </ul>
   *
   * @param reader Reader containing a Snowball stopword deque
   * @param result the {@link CharArraySet} to fill with the readers words
   * @return the given {@link CharArraySet} with the reader's words
   */
  static std::shared_ptr<CharArraySet>
  getSnowballWordSet(std::shared_ptr<Reader> reader,
                     std::shared_ptr<CharArraySet> result) ;

  /**
   * Reads stopwords from a stopword deque in Snowball format.
   * <p>
   * The snowball format is the following:
   * <ul>
   * <li>Lines may contain multiple words separated by whitespace.
   * <li>The comment character is the vertical line (&#124;).
   * <li>Lines may contain trailing comments.
   * </ul>
   *
   * @param reader Reader containing a Snowball stopword deque
   * @return A {@link CharArraySet} with the reader's words
   */
  static std::shared_ptr<CharArraySet>
  getSnowballWordSet(std::shared_ptr<Reader> reader) ;

  /**
   * Reads a stem dictionary. Each line contains:
   * <pre>word<b>\t</b>stem</pre>
   * (i.e. two tab separated words)
   *
   * @return stem dictionary that overrules the stemming algorithm
   * @throws IOException If there is a low-level I/O error.
   */
  static std::shared_ptr<CharArrayMap<std::wstring>> getStemDict(
      std::shared_ptr<Reader> reader,
      std::shared_ptr<CharArrayMap<std::wstring>> result) ;

  /**
   * Accesses a resource by name and returns the (non comment) lines containing
   * data using the given character encoding.
   *
   * <p>
   * A comment line is any line that starts with the character "#"
   * </p>
   *
   * @return a deque of non-blank non-comment lines with whitespace trimmed
   * @throws IOException If there is a low-level I/O error.
   */
  static std::deque<std::wstring>
  getLines(std::shared_ptr<InputStream> stream,
           std::shared_ptr<Charset> charset) ;

private:
  static std::shared_ptr<BufferedReader>
  getBufferedReader(std::shared_ptr<Reader> reader);
};

} // namespace org::apache::lucene::analysis
