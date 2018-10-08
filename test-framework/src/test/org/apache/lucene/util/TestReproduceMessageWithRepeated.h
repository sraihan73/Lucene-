#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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

/**
 * Test reproduce message is right with {@link Repeat} annotation.
 */
class TestReproduceMessageWithRepeated : public WithNestedTests
{
  GET_CLASS_NAME(TestReproduceMessageWithRepeated)
public:
  class Nested : public AbstractNestedTest
  {
    GET_CLASS_NAME(Nested)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Test @Repeat(iterations = 10) public void testMe()
    virtual void testMe();

  protected:
    std::shared_ptr<Nested> shared_from_this()
    {
      return std::static_pointer_cast<Nested>(
          AbstractNestedTest::shared_from_this());
    }
  };

public:
  TestReproduceMessageWithRepeated();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testRepeatedMessage() throws Exception
  virtual void testRepeatedMessage() ;

private:
  std::wstring runAndReturnSyserr();

protected:
  std::shared_ptr<TestReproduceMessageWithRepeated> shared_from_this()
  {
    return std::static_pointer_cast<TestReproduceMessageWithRepeated>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
