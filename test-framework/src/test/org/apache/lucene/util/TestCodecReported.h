#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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

class TestCodecReported : public WithNestedTests
{
  GET_CLASS_NAME(TestCodecReported)
public:
  TestCodecReported();

public:
  class Nested1 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Nested1)
  public:
    static std::wstring codecName;

    virtual void testDummy();

  protected:
    std::shared_ptr<Nested1> shared_from_this()
    {
      return std::static_pointer_cast<Nested1>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testCorrectCodecReported()
  virtual void testCorrectCodecReported();

protected:
  std::shared_ptr<TestCodecReported> shared_from_this()
  {
    return std::static_pointer_cast<TestCodecReported>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
