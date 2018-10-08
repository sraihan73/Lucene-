#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::hunspell
{
class Dictionary;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::util
{
class CharsRef;
}
namespace org::apache::lucene::util
{
class IntsRef;
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
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
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
namespace org::apache::lucene::analysis::hunspell
{

using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using FST = org::apache::lucene::util::fst::FST;

/**
 * Stemmer uses the affix rules declared in the Dictionary to generate one or
 * more stems for a word.  It conforms to the algorithm in the original hunspell
 * algorithm, including recursive suffix stripping.
 */
class Stemmer final : public std::enable_shared_from_this<Stemmer>
{
  GET_CLASS_NAME(Stemmer)
private:
  const std::shared_ptr<Dictionary> dictionary;
  const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();
  const std::shared_ptr<StringBuilder> segment =
      std::make_shared<StringBuilder>();
  const std::shared_ptr<ByteArrayDataInput> affixReader;

  // used for normalization
  const std::shared_ptr<StringBuilder> scratchSegment =
      std::make_shared<StringBuilder>();
  std::deque<wchar_t> scratchBuffer = std::deque<wchar_t>(32);

  // it's '1' if we have no stem exceptions, otherwise every other form
  // is really an ID pointing to the exception table
  const int formStep;

  /**
   * Constructs a new Stemmer which will use the provided Dictionary to create
   * its stems.
   *
   * @param dictionary Dictionary that will be used to create the stems
   */
public:
  Stemmer(std::shared_ptr<Dictionary> dictionary);

  /**
   * Find the stem(s) of the provided word.
   *
   * @param word Word to find the stems for
   * @return List of stems for the word
   */
  std::deque<std::shared_ptr<CharsRef>> stem(const std::wstring &word);

  /**
   * Find the stem(s) of the provided word
   *
   * @param word Word to find the stems for
   * @return List of stems for the word
   */
  std::deque<std::shared_ptr<CharsRef>> stem(std::deque<wchar_t> &word,
                                              int length);

  // temporary buffers for case variants
private:
  std::deque<wchar_t> lowerBuffer = std::deque<wchar_t>(8);
  std::deque<wchar_t> titleBuffer = std::deque<wchar_t>(8);

  static constexpr int EXACT_CASE = 0;
  static constexpr int TITLE_CASE = 1;
  static constexpr int UPPER_CASE = 2;

  /** returns EXACT_CASE,TITLE_CASE, or UPPER_CASE type for the word */
  int caseOf(std::deque<wchar_t> &word, int length);

  /** folds titlecase variant of word to titleBuffer */
  void caseFoldTitle(std::deque<wchar_t> &word, int length);

  /** folds lowercase variant of word (title cased) to lowerBuffer */
  void caseFoldLower(std::deque<wchar_t> &word, int length);

  std::deque<std::shared_ptr<CharsRef>> doStem(std::deque<wchar_t> &word,
                                                int length, bool caseVariant);

  /**
   * Find the unique stem(s) of the provided word
   *
   * @param word Word to find the stems for
   * @return List of stems for the word
   */
public:
  std::deque<std::shared_ptr<CharsRef>> uniqueStems(std::deque<wchar_t> &word,
                                                     int length);

private:
  std::shared_ptr<CharsRef> newStem(std::deque<wchar_t> &buffer, int length,
                                    std::shared_ptr<IntsRef> forms, int formID);

  // ================================================= Helper Methods
  // ================================================

  // some state for traversing FSTs
public:
  std::deque<std::shared_ptr<FST::BytesReader>> const prefixReaders =
      std::deque<std::shared_ptr<FST::BytesReader>>(3);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.IntsRef>
  // prefixArcs[] = new org.apache.lucene.util.fst.FST.Arc[3];
  std::deque<FST::Arc<std::shared_ptr<IntsRef>>> const prefixArcs =
      std::deque<std::shared_ptr<FST::Arc>>(3);

  std::deque<std::shared_ptr<FST::BytesReader>> const suffixReaders =
      std::deque<std::shared_ptr<FST::BytesReader>>(3);
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) final
  // org.apache.lucene.util.fst.FST.Arc<org.apache.lucene.util.IntsRef>
  // suffixArcs[] = new org.apache.lucene.util.fst.FST.Arc[3];
  std::deque<FST::Arc<std::shared_ptr<IntsRef>>> const suffixArcs =
      std::deque<std::shared_ptr<FST::Arc>>(3);

  /**
   * Generates a deque of stems for the provided word
   *
   * @param word Word to generate the stems for
   * @param previous previous affix that was removed (so we dont remove same one
   * twice)
   * @param prevFlag Flag from a previous stemming step that need to be
   * cross-checked with any affixes in this recursive step
   * @param prefixFlag flag of the most inner removed prefix, so that when
   * removing a suffix, it's also checked against the word
   * @param recursionDepth current recursiondepth
   * @param doPrefix true if we should remove prefixes
   * @param doSuffix true if we should remove suffixes
   * @param previousWasPrefix true if the previous removal was a prefix:
   *        if we are removing a suffix, and it has no continuation
   * requirements, it's ok. but two prefixes (COMPLEXPREFIXES) or two suffixes
   * must have continuation requirements to recurse.
   * @param circumfix true if the previous prefix removal was signed as a
   * circumfix this means inner most suffix must also contain circumfix flag.
   * @param caseVariant true if we are searching for a case variant. if the word
   * has KEEPCASE flag it cannot succeed.
   * @return List of stems, or empty deque if no stems are found
   */
private:
  std::deque<std::shared_ptr<CharsRef>>
  stem(std::deque<wchar_t> &word, int length, int previous, int prevFlag,
       int prefixFlag, int recursionDepth, bool doPrefix, bool doSuffix,
       bool previousWasPrefix, bool circumfix,
       bool caseVariant) ;

  /** checks condition of the concatenation of two strings */
  // note: this is pretty stupid, we really should subtract strip from the
  // condition up front and just check the stem but this is a little bit more
  // complicated.
  bool checkCondition(int condition, std::deque<wchar_t> &c1, int c1off,
                      int c1len, std::deque<wchar_t> &c2, int c2off,
                      int c2len);

  /**
   * Applies the affix rule to the given word, producing a deque of stems if any
   * are found
   *
   * @param strippedWord Word the affix has been removed and the strip added
   * @param length valid length of stripped word
   * @param affix HunspellAffix representing the affix rule itself
   * @param prefixFlag when we already stripped a prefix, we cant simply recurse
   * and check the suffix, unless both are compatible so we must check
   * dictionary form against both to add it as a stem!
   * @param recursionDepth current recursion depth
   * @param prefix true if we are removing a prefix (false if it's a suffix)
   * @return List of stems for the word, or an empty deque if none are found
   */
public:
  std::deque<std::shared_ptr<CharsRef>>
  applyAffix(std::deque<wchar_t> &strippedWord, int length, int affix,
             int prefixFlag, int recursionDepth, bool prefix, bool circumfix,
             bool caseVariant) ;

  /**
   * Checks if the given flag cross checks with the given array of flags
   *
   * @param flag Flag to cross check with the array of flags
   * @param flags Array of flags to cross check against.  Can be {@code null}
   * @return {@code true} if the flag is found in the array or the array is
   * {@code null}, {@code false} otherwise
   */
private:
  bool hasCrossCheckedFlag(wchar_t flag, std::deque<wchar_t> &flags,
                           bool matchEmpty);
};

} // namespace org::apache::lucene::analysis::hunspell
