#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/lv/Affix.h"

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
namespace org::apache::lucene::analysis::lv
{

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Light stemmer for Latvian.
 * <p>
 * This is a light version of the algorithm in Karlis Kreslin's PhD thesis
 * <i>A stemming algorithm for Latvian</i> with the following modifications:
 * <ul>
 *   <li>Only explicitly stems noun and adjective morphology
 *   <li>Stricter length/vowel checks for the resulting stems (verb etc suffix
 * stripping is removed) <li>Removes only the primary inflectional suffixes:
 * case and number for nouns ; case, number, gender, and definitiveness for
 * adjectives. <li>Palatalization is only handled when a declension II,V,VI noun
 * suffix is removed.
 * </ul>
 */
class LatvianStemmer : public std::enable_shared_from_this<LatvianStemmer>
{
  GET_CLASS_NAME(LatvianStemmer)
  /**
   * Stem a latvian word. returns the new adjusted length.
   */
public:
  virtual int stem(std::deque<wchar_t> &s, int len);

  static std::deque<std::shared_ptr<Affix>> const affixes;

public:
  class Affix : public std::enable_shared_from_this<Affix>
  {
    GET_CLASS_NAME(Affix)
  public:
    std::deque<wchar_t> affix; // suffix
    int vc = 0;                 // vowel count of the suffix
    bool palatalizes = false;   // true if we should fire palatalization rules.

    Affix(const std::wstring &affix, int vc, bool palatalizes);
  };

  /**
   * Most cases are handled except for the ambiguous ones:
   * <ul>
   *  <li> s -&gt; š
   *  <li> t -&gt; š
   *  <li> d -&gt; ž
   *  <li> z -&gt; ž
   * </ul>
   */
private:
  int unpalatalize(std::deque<wchar_t> &s, int len);

  /**
   * Count the vowels in the string, we always require at least
   * one in the remaining stem to accept it.
   */
  int numVowels(std::deque<wchar_t> &s, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/lv/
