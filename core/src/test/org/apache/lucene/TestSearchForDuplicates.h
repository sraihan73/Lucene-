#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/ScoreDoc.h"

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
namespace org::apache::lucene
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSearchForDuplicates : public LuceneTestCase
{
  GET_CLASS_NAME(TestSearchForDuplicates)

public:
  static const std::wstring PRIORITY_FIELD;
  static const std::wstring ID_FIELD;
  static const std::wstring HIGH_PRIORITY;
  static const std::wstring MED_PRIORITY;
  static const std::wstring LOW_PRIORITY;

  /** This test compares search results when using and not using compound
   *  files.
   *
   *  TODO: There is rudimentary search result validation as well, but it is
   *        simply based on asserting the output observed in the old test case,
   *        without really knowing if the output is correct. Someone needs to
   *        validate this output and make any changes to the checkHits method.
   */
  virtual void testRun() ;

private:
  void doTest(std::shared_ptr<Random> random, std::shared_ptr<PrintWriter> out,
              bool useCompoundFiles, int MAX_DOCS) ;

  void printHits(std::shared_ptr<PrintWriter> out,
                 std::deque<std::shared_ptr<ScoreDoc>> &hits,
                 std::shared_ptr<IndexSearcher> searcher) ;

  void checkHits(std::deque<std::shared_ptr<ScoreDoc>> &hits,
                 int expectedCount,
                 std::shared_ptr<IndexSearcher> searcher) ;

protected:
  std::shared_ptr<TestSearchForDuplicates> shared_from_this()
  {
    return std::static_pointer_cast<TestSearchForDuplicates>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/
