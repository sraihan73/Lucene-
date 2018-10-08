#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
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

class TestArrayUtil : public LuceneTestCase
{
  GET_CLASS_NAME(TestArrayUtil)

  // Ensure ArrayUtil.getNextSize gives linear amortized cost of realloc/copy
public:
  virtual void testGrowth();

  virtual void testMaxSize();

  virtual void testTooBig();

  virtual void testExactLimit();

  virtual void testInvalidElementSizes();

private:
  static int parseInt(const std::wstring &s);

public:
  virtual void testParseInt() ;

private:
  std::deque<std::optional<int>> createRandomArray(int maxSize);

public:
  virtual void testIntroSort();

private:
  std::deque<std::optional<int>> createSparseRandomArray(int maxSize);

  // This is a test for LUCENE-3054 (which fails without the merge sort fall
  // back with stack overflow in most cases)
public:
  virtual void testQuickToHeapSortFallback();

  virtual void testTimSort();

public:
  class Item : public std::enable_shared_from_this<Item>,
               public Comparable<std::shared_ptr<Item>>
  {
    GET_CLASS_NAME(Item)
  public:
    const int val, order;

    Item(int val, int order);

    int compareTo(std::shared_ptr<Item> other) override;

    virtual std::wstring toString();
  };

public:
  virtual void testMergeSortStability();

  virtual void testTimSortStability();

  // should produce no exceptions
  virtual void testEmptyArraySort();

  virtual void testSelect();

private:
  void doTestSelect();

protected:
  std::shared_ptr<TestArrayUtil> shared_from_this()
  {
    return std::static_pointer_cast<TestArrayUtil>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
