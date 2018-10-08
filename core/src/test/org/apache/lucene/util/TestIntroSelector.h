#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
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

class TestIntroSelector : public LuceneTestCase
{
  GET_CLASS_NAME(TestIntroSelector)

public:
  virtual void testSelect();

  virtual void testSlowSelect();

private:
  void doTestSelect(bool slow);

private:
  class IntroSelectorAnonymousInnerClass : public IntroSelector
  {
    GET_CLASS_NAME(IntroSelectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestIntroSelector> outerInstance;

    std::deque<std::optional<int>> actual;

  public:
    IntroSelectorAnonymousInnerClass(
        std::shared_ptr<TestIntroSelector> outerInstance,
        std::deque<std::optional<int>> &actual);

    std::optional<int> pivot;

  protected:
    void swap(int i, int j) override;

    void setPivot(int i) override;

    int comparePivot(int j) override;

  protected:
    std::shared_ptr<IntroSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntroSelectorAnonymousInnerClass>(
          IntroSelector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIntroSelector> shared_from_this()
  {
    return std::static_pointer_cast<TestIntroSelector>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
