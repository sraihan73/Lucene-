#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
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

using namespace org::apache::lucene::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.RamUsageEstimator.*;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.RamUsageTester.sizeOf;

class TestRamUsageEstimator : public LuceneTestCase
{
  GET_CLASS_NAME(TestRamUsageEstimator)
public:
  virtual void testSanity();

  virtual void testStaticOverloads();

  virtual void testReferenceSize();

  virtual void testHotspotBean();

  /** Helper to print out current settings for debugging {@code
   * -Dtests.verbose=true} */
  virtual void testPrintValues();

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private static class Holder
  class Holder : public std::enable_shared_from_this<Holder>
  {
  public:
    int64_t field1 = 5000LL;
    std::wstring name = L"name";
    std::shared_ptr<Holder> holder;
    int64_t field2 = 0, field3 = 0, field4 = 0;

    Holder();

    Holder(const std::wstring &name, int64_t field1);
  };

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unused") private static class
  // HolderSubclass extends Holder
  class HolderSubclass : public Holder
  {
  public:
    char foo = 0;
    int bar = 0;

  protected:
    std::shared_ptr<HolderSubclass> shared_from_this()
    {
      GET_CLASS_NAME(>)
      return std::static_pointer_cast<HolderSubclass>(
          Holder::shared_from_this());
    }
  };

private:
  class HolderSubclass2 : public Holder
  {
    GET_CLASS_NAME(HolderSubclass2)
    // empty, only inherits all fields -> size should be identical to superclass

  protected:
    std::shared_ptr<HolderSubclass2> shared_from_this()
    {
      GET_CLASS_NAME(2 >)
      return std::static_pointer_cast<HolderSubclass2>(
          Holder::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestRamUsageEstimator> shared_from_this()
  {
    return std::static_pointer_cast<TestRamUsageEstimator>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
