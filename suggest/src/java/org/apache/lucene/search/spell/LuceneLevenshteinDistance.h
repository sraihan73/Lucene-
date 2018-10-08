#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

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
namespace org::apache::lucene::search::spell
{

using IntsRef = org::apache::lucene::util::IntsRef;

/**
 *  Damerau-Levenshtein (optimal string alignment) implemented in a consistent
 *  way as Lucene's FuzzyTermsEnum with the transpositions option enabled.
 *
 *  Notes:
 *  <ul>
 *    <li> This metric treats full unicode codepoints as characters
 *    <li> This metric scales raw edit distances into a floating point score
 *         based upon the shortest of the two terms
 *    <li> Transpositions of two adjacent codepoints are treated as primitive
 *         edits.
 *    <li> Edits are applied in parallel: for example, "ab" and "bca" have
 *         distance 3.
 *  </ul>
 *
 *  NOTE: this class is not particularly efficient. It is only intended
 *  for merging results from multiple DirectSpellCheckers.
 */
class LuceneLevenshteinDistance final
    : public std::enable_shared_from_this<LuceneLevenshteinDistance>,
      public StringDistance
{
  GET_CLASS_NAME(LuceneLevenshteinDistance)

  /**
   * Creates a new comparator, mimicing the behavior of Lucene's internal
   * edit distance.
   */
public:
  LuceneLevenshteinDistance();

  float getDistance(const std::wstring &target,
                    const std::wstring &other) override;

private:
  static std::shared_ptr<IntsRef> toIntsRef(const std::wstring &s);

public:
  bool equals(std::any obj) override;
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
