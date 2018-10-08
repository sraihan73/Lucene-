#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::util
{

class TestRadixSelector : public LuceneTestCase
{
  GET_CLASS_NAME(TestRadixSelector)

public:
  virtual void testSelect();

private:
  void doTestSelect();

public:
  virtual void testSharedPrefixes();

private:
  void doTestSharedPrefixes();

  void doTest(std::deque<std::shared_ptr<BytesRef>> &arr, int from, int to,
              int maxLen);

private:
  class RadixSelectorAnonymousInnerClass : public RadixSelector
  {
    GET_CLASS_NAME(RadixSelectorAnonymousInnerClass)
  private:
    std::shared_ptr<TestRadixSelector> outerInstance;

    int k = 0;
    std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> actual;
    int enforcedMaxLen = 0;

  public:
    RadixSelectorAnonymousInnerClass(
        std::shared_ptr<TestRadixSelector> outerInstance, int k,
        std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>>
            &actual,
        int enforcedMaxLen);

  protected:
    void swap(int i, int j) override;

    int byteAt(int i, int k) override;

  protected:
    std::shared_ptr<RadixSelectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RadixSelectorAnonymousInnerClass>(
          RadixSelector::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestRadixSelector> shared_from_this()
  {
    return std::static_pointer_cast<TestRadixSelector>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
