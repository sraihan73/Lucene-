#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergePolicy;
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
namespace org::apache::lucene::index
{

class TestNoMergePolicy : public BaseMergePolicyTestCase
{
  GET_CLASS_NAME(TestNoMergePolicy)

public:
  std::shared_ptr<MergePolicy> mergePolicy() override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNoMergePolicy() throws Exception
  virtual void testNoMergePolicy() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testFinalSingleton() throws Exception
  virtual void testFinalSingleton() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testMethodsOverridden() throws Exception
  virtual void testMethodsOverridden() ;

protected:
  std::shared_ptr<TestNoMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestNoMergePolicy>(
        BaseMergePolicyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
