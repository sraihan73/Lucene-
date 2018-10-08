#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::search
{
class Query;
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

namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class LengthGoalBreakIteratorTest : public LuceneTestCase
{
  GET_CLASS_NAME(LengthGoalBreakIteratorTest)
private:
  static const std::wstring FIELD;

  // We test LengthGoalBreakIterator as it is used by the UnifiedHighlighter
  // instead of directly, because it is
  //  not a general purpose BreakIterator.  A unit test of it directly wouldn't
  //  give as much confidence.

  const std::shared_ptr<Analyzer> analyzer = std::make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE,
      true); // whitespace, punctuation, lowercase

  // We do a '.' BreakIterator and test varying the length goal.
  //                      0         1
  //                      01234567890123456789
public:
  static const std::wstring CONTENT;

  virtual void testTargetLen() ;

  virtual void testMinLen() ;

  virtual void testDefaultSummaryTargetLen() ;

private:
  std::shared_ptr<Query> query(const std::wstring &qStr);

  std::wstring highlightClosestToLen(const std::wstring &content,
                                     std::shared_ptr<Query> query,
                                     int lengthGoal) ;

  std::wstring highlightMinLen(const std::wstring &content,
                               std::shared_ptr<Query> query,
                               int lengthGoal) ;

protected:
  std::shared_ptr<LengthGoalBreakIteratorTest> shared_from_this()
  {
    return std::static_pointer_cast<LengthGoalBreakIteratorTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::uhighlight
