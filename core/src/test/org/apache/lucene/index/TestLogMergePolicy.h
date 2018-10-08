#pragma once
#include "stringhelper.h"
#include <memory>

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

class TestLogMergePolicy : public BaseMergePolicyTestCase
{
  GET_CLASS_NAME(TestLogMergePolicy)

public:
  std::shared_ptr<MergePolicy> mergePolicy() override;

  virtual void testDefaultForcedMergeMB();

protected:
  std::shared_ptr<TestLogMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestLogMergePolicy>(
        BaseMergePolicyTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index