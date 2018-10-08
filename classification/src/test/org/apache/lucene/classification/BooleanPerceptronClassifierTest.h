#pragma once
#include "ClassificationTestBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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

/**
 * Testcase for {@link
 * org.apache.lucene.classification.BooleanPerceptronClassifier}
 */
class BooleanPerceptronClassifierTest : public ClassificationTestBase<bool>
{
  GET_CLASS_NAME(BooleanPerceptronClassifierTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicUsage() throws Exception
  virtual void testBasicUsage() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testExplicitThreshold() throws Exception
  virtual void testExplicitThreshold() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicUsageWithQuery() throws Exception
  virtual void testBasicUsageWithQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testPerformance() throws Exception
  virtual void testPerformance() ;

protected:
  std::shared_ptr<BooleanPerceptronClassifierTest> shared_from_this()
  {
    return std::static_pointer_cast<BooleanPerceptronClassifierTest>(
        ClassificationTestBase<bool>::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification
