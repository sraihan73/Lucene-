#pragma once
#include "stringhelper.h"
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Unit tests for {@link DocValuesStatsCollector}. */
class TestDocValuesStatsCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocValuesStatsCollector)

public:
  virtual void testNoDocsWithField() ;

  virtual void testOneDoc() ;

  virtual void testDocsWithLongValues() ;

  virtual void testDocsWithDoubleValues() ;

  virtual void testDocsWithMultipleLongValues() ;

  virtual void testDocsWithMultipleDoubleValues() ;

  virtual void testDocsWithSortedValues() ;

  virtual void testDocsWithSortedSetValues() ;

private:
  static std::shared_ptr<LongStream>
  getPositiveValues(std::deque<int64_t> &values);

  static std::shared_ptr<DoubleStream>
  getPositiveValues(std::deque<double> &values);

  static std::shared_ptr<LongStream>
  getZeroValues(std::deque<int64_t> &values);

  static std::shared_ptr<DoubleStream>
  getZeroValues(std::deque<double> &values);

  static double computeVariance(std::deque<int64_t> &values, double mean,
                                int count);

  static double computeVariance(std::deque<double> &values, double mean,
                                int count);

  template <>
  static std::shared_ptr<LongStream>
  filterAndFlatValues(std::deque<std::deque<int64_t>> &values,
                      std::function<T1> p);

  template <>
  static std::shared_ptr<DoubleStream>
  filterAndFlatValues(std::deque<std::deque<double>> &values,
                      std::function<T1> p);

  static double computeVariance(std::shared_ptr<LongStream> values, double mean,
                                int count);

  static double computeVariance(std::shared_ptr<DoubleStream> values,
                                double mean, int count);

  template <typename T>
  static std::shared_ptr<Stream<T>> nonNull(std::deque<T> &values);

  template <typename T>
  static std::shared_ptr<Stream<T>> isNull(std::deque<T> &values);

  template <typename T, typename T1>
  static std::shared_ptr<Stream<T>> filterValues(std::deque<T> &values,
                                                 std::function<T1> p);

  static int computeExpMissing(int numDocsWithoutField, int numIndexedDocs,
                               std::shared_ptr<IndexReader> reader);

protected:
  std::shared_ptr<TestDocValuesStatsCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestDocValuesStatsCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
