#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Sorter;
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

class BaseSortTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseSortTestCase)

public:
  class Entry : public std::enable_shared_from_this<Entry>,
                public java::lang::Comparable<std::shared_ptr<Entry>>
  {
    GET_CLASS_NAME(Entry)

  public:
    const int value;
    const int ord;

    Entry(int value, int ord);

    int compareTo(std::shared_ptr<Entry> other) override;
  };

private:
  const bool stable;

public:
  BaseSortTestCase(bool stable);

  virtual std::shared_ptr<Sorter>
  newSorter(std::deque<std::shared_ptr<Entry>> &arr) = 0;

  virtual void assertSorted(std::deque<std::shared_ptr<Entry>> &original,
                            std::deque<std::shared_ptr<Entry>> &sorted);

  virtual void test(std::deque<std::shared_ptr<Entry>> &arr);

public:
  class Strategy final
  {
    GET_CLASS_NAME(Strategy)
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        RANDOM
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = new Entry(random().nextInt(), i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        RANDOM_LOW_CARDINALITY
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = new Entry(random().nextInt(6), i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        ASCENDING
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = i == 0 ? new Entry(random().nextInt(6), 0) : new
    //                Entry(arr[i - 1].value + random().nextInt(6), i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        DESCENDING
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = i == 0 ? new Entry(random().nextInt(6), 0) : new
    //                Entry(arr[i - 1].value - random().nextInt(6), i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        STRICTLY_DESCENDING
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = i == 0 ? new Entry(random().nextInt(6), 0) : new
    //                Entry(arr[i - 1].value - TestUtil.nextInt(random(), 1, 5),
    //                i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        ASCENDING_SEQUENCES
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = i == 0 ? new Entry(random().nextInt(6), 0) : new
    //                Entry(rarely() ? random().nextInt(1000) : arr[i - 1].value
    //                + random().nextInt(6), i);
    //            }
    //        },
    // C++ TODO: Enum value-specific class bodies are not converted by Java to
    // C++ Converter:
    //        MOSTLY_ASCENDING
    //        {
    //            public void set(Entry[] arr, int i)
    //            {
    //                arr[i] = i == 0 ? new Entry(random().nextInt(6), 0) : new
    //                Entry(arr[i - 1].value + TestUtil.nextInt(random(), -8,
    //                10), i);
    //            }
    //        };

  private:
    static std::deque<Strategy> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum { GET_CLASS_NAME(InnerEnum) };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

    Strategy(const std::wstring &name, InnerEnum innerEnum);

  public:
    virtual void set(std::deque<std::shared_ptr<Entry>> &arr, int i) = 0;

  public:
    bool operator==(const Strategy &other);

    bool operator!=(const Strategy &other);

    static std::deque<Strategy> values();

    int ordinal();

    std::wstring toString();

    static Strategy valueOf(const std::wstring &name);
  };

public:
  virtual void test(Strategy strategy, int length);

  virtual void test(Strategy strategy);

  virtual void testEmpty();

  virtual void testOne();

  virtual void testTwo();

  virtual void testRandom();

  virtual void testRandomLowCardinality();

  virtual void testAscending();

  virtual void testAscendingSequences();

  virtual void testDescending();

  virtual void testStrictlyDescending();

protected:
  std::shared_ptr<BaseSortTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseSortTestCase>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
