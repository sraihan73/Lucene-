#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <unordered_set>
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
namespace org::apache::lucene::util
{

class TestMSBRadixSorter : public LuceneTestCase
{
  GET_CLASS_NAME(TestMSBRadixSorter)

private:
  void test(std::deque<std::shared_ptr<BytesRef>> &refs, int len);

private:
  class MSBRadixSorterAnonymousInnerClass : public MSBRadixSorter
  {
    GET_CLASS_NAME(MSBRadixSorterAnonymousInnerClass)
  private:
    std::shared_ptr<TestMSBRadixSorter> outerInstance;

    std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> refs;
    int finalMaxLength = 0;

  public:
    MSBRadixSorterAnonymousInnerClass(
        std::shared_ptr<TestMSBRadixSorter> outerInstance, int maxLength,
        std::deque<std::shared_ptr<org::apache::lucene::util::BytesRef>> &refs,
        int finalMaxLength);

  protected:
    int byteAt(int i, int k) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<MSBRadixSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MSBRadixSorterAnonymousInnerClass>(
          MSBRadixSorter::shared_from_this());
    }
  };

public:
  virtual void testEmpty();

  virtual void testOneValue();

  virtual void testTwoValues();

private:
  void testRandom(int commonPrefixLen, int maxLen);

public:
  virtual void testRandom();

  virtual void testRandomWithLotsOfDuplicates();

  virtual void testRandomWithSharedPrefix();

  virtual void testRandomWithSharedPrefixAndLotsOfDuplicates();

  virtual void testRandom2();

protected:
  std::shared_ptr<TestMSBRadixSorter> shared_from_this()
  {
    return std::static_pointer_cast<TestMSBRadixSorter>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
