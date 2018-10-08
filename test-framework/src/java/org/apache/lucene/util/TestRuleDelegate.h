#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>

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

using org::junit::rules::TestRule;
using org::junit::runner::Description;
using org::junit::runners::model::Statement;

/**
 * A {@link TestRule} that delegates to another {@link TestRule} via a delegate
 * contained in a an {@link AtomicReference}.
 */
template <typename T>
class TestRuleDelegate final
    : public std::enable_shared_from_this<TestRuleDelegate>,
      public TestRule
{
  GET_CLASS_NAME(TestRuleDelegate)
  static_assert(std::is_base_of<org.junit.rules.TestRule, T>::value,
                L"T must inherit from org.junit.rules.TestRule");

private:
  std::shared_ptr<AtomicReference<T>> delegate_;

  TestRuleDelegate(std::shared_ptr<AtomicReference<T>> delegate_)
  {
    this->delegate_ = delegate_;
  }

public:
  std::shared_ptr<Statement> apply(std::shared_ptr<Statement> s,
                                   std::shared_ptr<Description> d) override
  {
    return delegate_->get().apply(s, d);
  }

  template <typename T>
  static std::shared_ptr<TestRuleDelegate<T>>
  of(std::shared_ptr<AtomicReference<T>> delegate_)
  {
    static_assert(std::is_base_of<org.junit.rules.TestRule, T>::value,
                  L"T must inherit from org.junit.rules.TestRule");

    return std::make_shared<TestRuleDelegate<T>>(delegate_);
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
