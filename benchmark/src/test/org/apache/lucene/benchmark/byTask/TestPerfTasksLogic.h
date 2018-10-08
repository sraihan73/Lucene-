#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/index/LogDocMergePolicy.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SerialMergeScheduler.h"
#include "../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::benchmark::byTask
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using LogDocMergePolicy = org::apache::lucene::index::LogDocMergePolicy;
using SerialMergeScheduler = org::apache::lucene::index::SerialMergeScheduler;

/**
 * Test very simply that perf tasks - simple algorithms - are doing what they
 * should.
 */
class TestPerfTasksLogic : public BenchmarkTestCase
{
  GET_CLASS_NAME(TestPerfTasksLogic)

public:
  void setUp()  override;

  /**
   * Test index creation logic
   */
  virtual void testIndexAndSearchTasks() ;

  /**
   * Test timed sequence task.
   */
  virtual void testTimedSearchTask() ;

  // disabled until we fix BG thread prio -- this test
  // causes build to hang
  virtual void testBGSearchTaskThreads() ;

  /**
   * Test Exhasting Doc Maker logic
   */
  virtual void testExhaustContentSource() ;

  // LUCENE-1994: test thread safety of SortableSingleDocMaker
  virtual void testDocMakerThreadSafety() ;

  /**
   * Test Parallel Doc Maker logic (for LUCENE-940)
   */
  virtual void testParallelDocMaker() ;

  /**
   * Test WriteLineDoc and LineDocSource.
   */
  virtual void testLineDocFile() ;

  /**
   * Test ReadTokensTask
   */
  virtual void testReadTokens() ;

  /**
   * Test that " {[AddDoc(4000)]: 4} : * " works corrcetly (for LUCENE-941)
   */
  virtual void testParallelExhausted() ;

  /**
   * Test that exhaust in loop works as expected (LUCENE-1115).
   */
  virtual void testExhaustedLooped() ;

  /**
   * Test that we can close IndexWriter with argument "false".
   */
  virtual void testCloseIndexFalse() ;

public:
  class MyMergeScheduler : public SerialMergeScheduler
  {
    GET_CLASS_NAME(MyMergeScheduler)
  public:
    bool called = false;
    MyMergeScheduler();

  protected:
    std::shared_ptr<MyMergeScheduler> shared_from_this()
    {
      return std::static_pointer_cast<MyMergeScheduler>(
          org.apache.lucene.index.SerialMergeScheduler::shared_from_this());
    }
  };

  /**
   * Test that we can set merge scheduler".
   */
public:
  virtual void testMergeScheduler() ;

public:
  class MyMergePolicy : public LogDocMergePolicy
  {
    GET_CLASS_NAME(MyMergePolicy)
  public:
    bool called = false;
    MyMergePolicy();

  protected:
    std::shared_ptr<MyMergePolicy> shared_from_this()
    {
      return std::static_pointer_cast<MyMergePolicy>(
          org.apache.lucene.index.LogDocMergePolicy::shared_from_this());
    }
  };

  /**
   * Test that we can set merge policy".
   */
public:
  virtual void testMergePolicy() ;

  /**
   * Test that IndexWriter settings stick.
   */
  virtual void testIndexWriterSettings() ;

  /**
   * Test indexing with facets tasks.
   */
  virtual void testIndexingWithFacets() ;

  /**
   * Test that we can call forceMerge(maxNumSegments).
   */
  virtual void testForceMerge() ;

  /**
   * Test disabling task count (LUCENE-1136).
   */
  virtual void testDisableCounting() ;

private:
  void doTestDisableCounting(bool disable) ;

  std::deque<std::wstring> disableCountingLines(bool disable);

  /**
   * Test that we can change the Locale in the runData,
   * that it is parsed as we expect.
   */
public:
  virtual void testLocale() ;

private:
  std::deque<std::wstring> getLocaleConfig(const std::wstring &localeParam);

  /**
   * Test that we can create CollationAnalyzers.
   */
public:
  virtual void testCollator() ;

private:
  void assertEqualCollation(std::shared_ptr<Analyzer> a1,
                            std::shared_ptr<Analyzer> a2,
                            const std::wstring &text) ;

  std::deque<std::wstring>
  getCollatorConfig(const std::wstring &localeParam,
                    const std::wstring &collationParam);

  /**
   * Test that we can create shingle analyzers using AnalyzerFactory.
   */
public:
  virtual void testShingleAnalyzer() ;

private:
  std::deque<std::wstring>
  getAnalyzerFactoryConfig(const std::wstring &name,
                           const std::wstring &params);

public:
  virtual void testAnalyzerFactory() ;

private:
  std::wstring getReuters20LinesFile();

protected:
  std::shared_ptr<TestPerfTasksLogic> shared_from_this()
  {
    return std::static_pointer_cast<TestPerfTasksLogic>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask
