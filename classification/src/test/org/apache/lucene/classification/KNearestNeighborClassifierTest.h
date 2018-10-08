#pragma once
#include "ClassificationTestBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::classification
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Testcase for {@link KNearestNeighborClassifier}
 */
class KNearestNeighborClassifierTest
    : public ClassificationTestBase<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(KNearestNeighborClassifierTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicUsage() throws Exception
  virtual void testBasicUsage() ;

  /**
   * This test is for the scenario where in the first topK results from the MLT
   * query, we have the same number of results per class. But the results for a
   * class have a better ranking in comparison with the results of the second
   * class. So we would expect a greater score for the best ranked class.
   *
   * @throws Exception if any error happens
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRankedClasses() throws Exception
  virtual void testRankedClasses() ;

  /**
   * This test is for the scenario where in the first topK results from the MLT
   * query, we have less results for the expected class than the results for the
   * bad class. But the results for the expected class have a better score in
   * comparison with the results of the second class. So we would expect a
   * greater score for the best ranked class.
   *
   * @throws Exception if any error happens
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testUnbalancedClasses() throws Exception
  virtual void testUnbalancedClasses() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicUsageWithQuery() throws Exception
  virtual void testBasicUsageWithQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPerformance() throws Exception
  virtual void testPerformance() ;

protected:
  std::shared_ptr<KNearestNeighborClassifierTest> shared_from_this()
  {
    return std::static_pointer_cast<KNearestNeighborClassifierTest>(
        ClassificationTestBase<
            org.apache.lucene.util.BytesRef>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/classification/
