#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/surround/query/SingleFieldTestDb.h"

#include  "core/src/java/org/apache/lucene/queryparser/surround/query/BasicQueryFactory.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

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
namespace org::apache::lucene::queryparser::surround::query
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Scorer = org::apache::lucene::search::Scorer;
using SimpleCollector = org::apache::lucene::search::SimpleCollector;
using org::junit::Assert;

class BooleanQueryTst : public std::enable_shared_from_this<BooleanQueryTst>
{
  GET_CLASS_NAME(BooleanQueryTst)
public:
  std::wstring queryText;
  std::deque<int> const expectedDocNrs;
  std::shared_ptr<SingleFieldTestDb> dBase;
  std::wstring fieldName;
  std::shared_ptr<Assert> testCase;
  std::shared_ptr<BasicQueryFactory> qf;
  bool verbose = true;

  BooleanQueryTst(const std::wstring &queryText,
                  std::deque<int> &expectedDocNrs,
                  std::shared_ptr<SingleFieldTestDb> dBase,
                  const std::wstring &fieldName,
                  std::shared_ptr<Assert> testCase,
                  std::shared_ptr<BasicQueryFactory> qf);

  virtual void setVerbose(bool verbose);

public:
  class TestCollector : public SimpleCollector
  { // FIXME: use check hits from Lucene tests
    GET_CLASS_NAME(TestCollector)
  private:
    std::shared_ptr<BooleanQueryTst> outerInstance;

  public:
    int totalMatched = 0;
    std::deque<bool> encountered;

  private:
    std::shared_ptr<Scorer> scorer = nullptr;
    int docBase = 0;

  public:
    TestCollector(std::shared_ptr<BooleanQueryTst> outerInstance);

    void setScorer(std::shared_ptr<Scorer> scorer)  override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    void collect(int docNr)  override;

    bool needsScores() override;

    virtual void checkNrHits();

  protected:
    std::shared_ptr<TestCollector> shared_from_this()
    {
      return std::static_pointer_cast<TestCollector>(
          org.apache.lucene.search.SimpleCollector::shared_from_this());
    }
  };

public:
  virtual void doTest() ;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/query/
