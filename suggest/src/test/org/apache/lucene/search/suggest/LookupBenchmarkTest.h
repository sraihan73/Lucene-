#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class Input;
}

namespace org::apache::lucene::search::suggest
{
class Lookup;
}
namespace org::apache::lucene::search::suggest
{
class BenchmarkResult;
}
namespace org::apache::lucene::search::suggest
{
class Average;
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
namespace org::apache::lucene::search::suggest
{

using AnalyzingInfixSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingInfixSuggester;
using AnalyzingSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingSuggester;
using BlendedInfixSuggester =
    org::apache::lucene::search::suggest::analyzing::BlendedInfixSuggester;
using FreeTextSuggester =
    org::apache::lucene::search::suggest::analyzing::FreeTextSuggester;
using FuzzySuggester =
    org::apache::lucene::search::suggest::analyzing::FuzzySuggester;
using FSTCompletionLookup =
    org::apache::lucene::search::suggest::fst::FSTCompletionLookup;
using WFSTCompletionLookup =
    org::apache::lucene::search::suggest::fst::WFSTCompletionLookup;
using JaspellLookup =
    org::apache::lucene::search::suggest::jaspell::JaspellLookup;
using TSTLookup = org::apache::lucene::search::suggest::tst::TSTLookup;
using namespace org::apache::lucene::util;

/**
 * Benchmarks tests for implementations of {@link Lookup} interface.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore("COMMENT ME TO RUN BENCHMARKS!") public class
// LookupBenchmarkTest extends LuceneTestCase
class LookupBenchmarkTest : public LuceneTestCase
{
private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") private final
  // java.util.List<Class> benchmarkClasses =
  // java.util.Arrays.asList(org.apache.lucene.search.suggest.analyzing.FuzzySuggester.class,
  // org.apache.lucene.search.suggest.analyzing.AnalyzingSuggester.class,
  // org.apache.lucene.search.suggest.analyzing.AnalyzingInfixSuggester.class,
  // org.apache.lucene.search.suggest.jaspell.JaspellLookup.class,
  // org.apache.lucene.search.suggest.tst.TSTLookup.class,
  // org.apache.lucene.search.suggest.fst.FSTCompletionLookup.class,
  // org.apache.lucene.search.suggest.fst.WFSTCompletionLookup.class,
  // org.apache.lucene.search.suggest.analyzing.BlendedInfixSuggester.class,
  // org.apache.lucene.search.suggest.analyzing.FreeTextSuggester.class);
  const std::deque<std::type_info> benchmarkClasses = Arrays::asList(
      FuzzySuggester::typeid, AnalyzingSuggester::typeid,
      AnalyzingInfixSuggester::typeid, JaspellLookup::typeid, TSTLookup::typeid,
      FSTCompletionLookup::typeid, WFSTCompletionLookup::typeid,
      BlendedInfixSuggester::typeid, FreeTextSuggester::typeid);

  static constexpr int rounds = 15;
  static constexpr int warmup = 5;

  const int num = 7;
  const bool onlyMorePopular = false;

  static const std::shared_ptr<Random> random;

  /**
   * Input term/weight pairs.
   */
  static std::deque<std::shared_ptr<Input>> dictionaryInput;

  /**
   * Benchmark term/weight pairs (randomized order).
   */
  static std::deque<std::shared_ptr<Input>> benchmarkInput;

  /**
   * Loads terms and frequencies from Wikipedia (cached).
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setup() throws Exception
  static void setup() ;

  static const std::shared_ptr<Charset> UTF_8;

  /**
   * Collect the multilingual input for benchmarks/ tests.
   */
  static std::deque<std::shared_ptr<Input>>
  readTop50KWiki() ;

  /**
   * Test construction time.
   */
  virtual void testConstructionTime() ;

private:
  class CallableAnonymousInnerClass : public Callable<int>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass)
  private:
    std::shared_ptr<LookupBenchmarkTest> outerInstance;

    std::type_info cls;

  public:
    CallableAnonymousInnerClass(
        std::shared_ptr<LookupBenchmarkTest> outerInstance, std::type_info cls);

    std::optional<int> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass>(
          java.util.concurrent.Callable<int>::shared_from_this());
    }
  };

  /**
   * Test memory required for the storage.
   */
public:
  virtual void testStorageNeeds() ;

  /**
   * Create {@link Lookup} instance and populate it.
   */
private:
  std::shared_ptr<Lookup> buildLookup(
      std::type_info cls,
      std::deque<std::shared_ptr<Input>> &input) ;

  /**
   * Test performance of lookup on full hits.
   */
public:
  virtual void testPerformanceOnFullHits() ;

  /**
   * Test performance of lookup on longer term prefixes (6-9 letters or
   * shorter).
   */
  virtual void testPerformanceOnPrefixes6_9() ;

  /**
   * Test performance of lookup on short term prefixes (2-4 letters or shorter).
   */
  virtual void testPerformanceOnPrefixes2_4() ;

  /**
   * Run the actual benchmark.
   */
  virtual void
  runPerformanceTest(int const minPrefixLen, int const maxPrefixLen,
                     int const num,
                     bool const onlyMorePopular) ;

private:
  class CallableAnonymousInnerClass2 : public Callable<int>
  {
    GET_CLASS_NAME(CallableAnonymousInnerClass2)
  private:
    std::shared_ptr<LookupBenchmarkTest> outerInstance;

    int num = 0;
    bool onlyMorePopular = false;
    std::shared_ptr<org::apache::lucene::search::suggest::Lookup> lookup;
    std::deque<std::wstring> input;

  public:
    CallableAnonymousInnerClass2(
        std::shared_ptr<LookupBenchmarkTest> outerInstance, int num,
        bool onlyMorePopular,
        std::shared_ptr<org::apache::lucene::search::suggest::Lookup> lookup,
        std::deque<std::wstring> &input);

    std::optional<int> call()  override;

  protected:
    std::shared_ptr<CallableAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<CallableAnonymousInnerClass2>(
          java.util.concurrent.Callable<int>::shared_from_this());
    }
  };

  /**
   * Do the measurements.
   */
private:
  std::shared_ptr<BenchmarkResult>
  measure(std::shared_ptr<Callable<int>> callable);

  /** Guard against opts. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private static volatile int
  // guard; C++ TODO: 'volatile' has a different meaning in C++:
  static int guard;

private:
  class BenchmarkResult : public std::enable_shared_from_this<BenchmarkResult>
  {
    GET_CLASS_NAME(BenchmarkResult)
    /** Average time per round (ms). */
  public:
    const std::shared_ptr<Average> average;

    BenchmarkResult(std::deque<double> &times, int warmup, int rounds);
  };

protected:
  std::shared_ptr<LookupBenchmarkTest> shared_from_this()
  {
    return std::static_pointer_cast<LookupBenchmarkTest>(
        LuceneTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::search::suggest
