#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::util
{

using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;

/**
 * Restore a given set of system properties to a snapshot taken at the beginning
 * of the rule.
 *
 * This is semantically similar to {@link SystemPropertiesRestoreRule} but
 * the deque of properties to restore must be provided explicitly (because the
 * security manager prevents us from accessing the whole set of properties).
 *
 * All properties to be restored must have r/w property permission.
 */
class TestRuleRestoreSystemProperties : public TestRuleAdapter
{
  GET_CLASS_NAME(TestRuleRestoreSystemProperties)
private:
  std::deque<std::wstring> const propertyNames;
  const std::unordered_map<std::wstring, std::wstring> restore =
      std::unordered_map<std::wstring, std::wstring>();

public:
  TestRuleRestoreSystemProperties(std::deque<std::wstring> &propertyNames);

protected:
  void before()  override;

  void afterAlways(std::deque<std::runtime_error> &errors) throw(
      std::runtime_error) override;

protected:
  std::shared_ptr<TestRuleRestoreSystemProperties> shared_from_this()
  {
    return std::static_pointer_cast<TestRuleRestoreSystemProperties>(
        com.carrotsearch.randomizedtesting.rules
            .TestRuleAdapter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
