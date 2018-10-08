#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::core
{
class TypeTokenFilter;
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
namespace org::apache::lucene::analysis::core
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestTypeTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestTypeTokenFilter)

public:
  virtual void testTypeFilter() ;

  /**
   * Test Position increments applied by TypeTokenFilter with and without
   * enabling this option.
   */
  virtual void testStopPositons() ;

private:
  void testPositons(std::shared_ptr<TypeTokenFilter> stpf) ;

public:
  virtual void testTypeFilterWhitelist() ;

  // print debug info depending on VERBOSE
private:
  static void log(const std::wstring &s);

protected:
  std::shared_ptr<TestTypeTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestTypeTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
