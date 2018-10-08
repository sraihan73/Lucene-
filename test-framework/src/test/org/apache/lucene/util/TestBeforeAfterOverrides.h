#pragma once
#include "stringhelper.h"
#include <memory>

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

class TestBeforeAfterOverrides : public WithNestedTests
{
  GET_CLASS_NAME(TestBeforeAfterOverrides)
public:
  TestBeforeAfterOverrides();

public:
  class Before1 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(Before1)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Before public void before()
    virtual void before();

    virtual void testEmpty();

  protected:
    std::shared_ptr<Before1> shared_from_this()
    {
      return std::static_pointer_cast<Before1>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  class Before2 : public Before1
  {
    GET_CLASS_NAME(Before2)

  protected:
    std::shared_ptr<Before2> shared_from_this()
    {
      return std::static_pointer_cast<Before2>(Before1::shared_from_this());
    }
  };

public:
  class Before3 : public Before2
  {
    GET_CLASS_NAME(Before3)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @Override @Before public void before()
    void before() override;

  protected:
    std::shared_ptr<Before3> shared_from_this()
    {
      return std::static_pointer_cast<Before3>(Before2::shared_from_this());
    }
  };

public:
  class After1 : public WithNestedTests::AbstractNestedTest
  {
    GET_CLASS_NAME(After1)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @After public void after()
    virtual void after();

    virtual void testEmpty();

  protected:
    std::shared_ptr<After1> shared_from_this()
    {
      return std::static_pointer_cast<After1>(
          WithNestedTests.AbstractNestedTest::shared_from_this());
    }
  };

public:
  class After2 : public Before1
  {
    GET_CLASS_NAME(After2)

  protected:
    std::shared_ptr<After2> shared_from_this()
    {
      return std::static_pointer_cast<After2>(Before1::shared_from_this());
    }
  };

public:
  class After3 : public Before2
  {
    GET_CLASS_NAME(After3)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @After public void after()
    virtual void after();

  protected:
    std::shared_ptr<After3> shared_from_this()
    {
      return std::static_pointer_cast<After3>(Before2::shared_from_this());
    }
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBefore()
  virtual void testBefore();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAfter()
  virtual void testAfter();

protected:
  std::shared_ptr<TestBeforeAfterOverrides> shared_from_this()
  {
    return std::static_pointer_cast<TestBeforeAfterOverrides>(
        WithNestedTests::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
