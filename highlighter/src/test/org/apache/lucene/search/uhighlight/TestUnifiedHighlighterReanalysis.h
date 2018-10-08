#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/MockAnalyzer.h"

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

using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestUnifiedHighlighterReanalysis : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterReanalysis)

private:
  std::shared_ptr<MockAnalyzer> indexAnalyzer = std::make_shared<MockAnalyzer>(
      random(), MockTokenizer::SIMPLE,
      true); // whitespace, punctuation, lowercase;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithoutIndexSearcher() throws
  // java.io.IOException
  virtual void testWithoutIndexSearcher() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test(expected = IllegalStateException.class) public void
  // testIndexSearcherNullness() throws java.io.IOException
  virtual void testIndexSearcherNullness() ;

protected:
  std::shared_ptr<TestUnifiedHighlighterReanalysis> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterReanalysis>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
