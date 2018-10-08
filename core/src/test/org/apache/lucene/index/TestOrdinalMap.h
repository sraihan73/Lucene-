#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/RamUsageTester.h"

#include  "core/src/java/org/apache/lucene/util/Accumulator.h"

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestOrdinalMap : public LuceneTestCase
{
  GET_CLASS_NAME(TestOrdinalMap)

private:
  static const std::shared_ptr<Field> ORDINAL_MAP_OWNER_FIELD;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestOrdinalMap::StaticConstructor staticConstructor;

  static const std::shared_ptr<RamUsageTester::Accumulator>
      ORDINAL_MAP_ACCUMULATOR;

private:
  class AccumulatorAnonymousInnerClass : public RamUsageTester::Accumulator
  {
    GET_CLASS_NAME(AccumulatorAnonymousInnerClass)
  public:
    AccumulatorAnonymousInnerClass();

    int64_t accumulateObject(
        std::any o, int64_t shallowSize,
        std::unordered_map<std::shared_ptr<Field>, std::any> &fieldValues,
        std::shared_ptr<java::util::std::deque<std::any>> queue) override;

  protected:
    std::shared_ptr<AccumulatorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AccumulatorAnonymousInnerClass>(
          org.apache.lucene.util.RamUsageTester
              .Accumulator::shared_from_this());
    }
  };

public:
  virtual void testRamBytesUsed() ;

protected:
  std::shared_ptr<TestOrdinalMap> shared_from_this()
  {
    return std::static_pointer_cast<TestOrdinalMap>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
