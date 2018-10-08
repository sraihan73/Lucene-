#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class DateRangePrefixTree;
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
namespace org::apache::lucene::spatial::prefix::tree
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class DateRangePrefixTreeTest : public LuceneTestCase
{
  GET_CLASS_NAME(DateRangePrefixTreeTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory(argumentFormatting = "calendar=%s")
  // public static Iterable<Object[]> parameters()
  static std::deque<std::deque<std::any>> parameters();

private:
  const std::shared_ptr<DateRangePrefixTree> tree;

public:
  DateRangePrefixTreeTest(const std::wstring &suiteName,
                          std::shared_ptr<Calendar> templateCal);

  virtual void testRoundTrip() ;

  virtual void testToStringISO8601();

  // copies from DateRangePrefixTree
private:
  static std::deque<int> const CAL_FIELDS;

  void roundTrip(std::shared_ptr<Calendar> calOrig) ;

public:
  virtual void testShapeRelations() ;

  virtual void testShapeRangeOptimizer() ;

protected:
  std::shared_ptr<DateRangePrefixTreeTest> shared_from_this()
  {
    return std::static_pointer_cast<DateRangePrefixTreeTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};
} // namespace org::apache::lucene::spatial::prefix::tree
