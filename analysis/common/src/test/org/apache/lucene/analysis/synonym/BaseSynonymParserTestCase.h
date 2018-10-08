#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

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
namespace org::apache::lucene::analysis::synonym
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Base class for testing synonym parsers.
 */
class BaseSynonymParserTestCase : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(BaseSynonymParserTestCase)
  /**
   * Helper method to validate synonym parsing.
   *
   * @param synonynMap  the generated synonym map_obj after parsing
   * @param word        word (phrase) we are validating the synonyms for. Should
   * be the value that comes out of the analyzer. All spaces will be replaced by
   * word separators.
   * @param includeOrig if synonyms should include original
   * @param synonyms    actual synonyms. All word separators are replaced with a
   * single space.
   */
public:
  static void assertEntryEquals(
      std::shared_ptr<SynonymMap> synonynMap, const std::wstring &word,
      bool includeOrig,
      std::deque<std::wstring> &synonyms) ;

  /**
   * Validates that there are no synonyms for the given word.
   * @param synonynMap  the generated synonym map_obj after parsing
   * @param word        word (phrase) we are validating the synonyms for. Should
   * be the value that comes out of the analyzer. All spaces will be replaced by
   * word separators.
   */
  static void assertEntryAbsent(std::shared_ptr<SynonymMap> synonynMap,
                                const std::wstring &word) ;

  static void
  assertEntryEquals(std::shared_ptr<SynonymMap> synonynMap,
                    const std::wstring &word, bool includeOrig,
                    const std::wstring &synonym) ;

  static void assertAnalyzesToPositions(
      std::shared_ptr<Analyzer> a, const std::wstring &input,
      std::deque<std::wstring> &output, std::deque<std::wstring> &types,
      std::deque<int> &posIncrements,
      std::deque<int> &posLengths) ;

protected:
  std::shared_ptr<BaseSynonymParserTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseSynonymParserTestCase>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
