#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

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

class TestVirtualMethod : public LuceneTestCase
{
  GET_CLASS_NAME(TestVirtualMethod)

private:
  static const std::shared_ptr<
      VirtualMethod<std::shared_ptr<TestVirtualMethod>>>
      publicTestMethod;
  static const std::shared_ptr<
      VirtualMethod<std::shared_ptr<TestVirtualMethod>>>
      protectedTestMethod;

public:
  virtual void publicTest(const std::wstring &test);

protected:
  virtual void protectedTest(int test);

public:
  class TestClass1;

public:
  class TestClass2 : public TestClass1
  {
    GET_CLASS_NAME(TestClass2)
  public:
    void protectedTest(int test) override;

  protected:
    std::shared_ptr<TestClass2> shared_from_this()
    {
      return std::static_pointer_cast<TestClass2>(
          TestClass1::shared_from_this());
    }
  };

public:
  class TestClass3 : public TestClass2
  {
    GET_CLASS_NAME(TestClass3)
  public:
    void publicTest(const std::wstring &test) override;

  protected:
    std::shared_ptr<TestClass3> shared_from_this()
    {
      return std::static_pointer_cast<TestClass3>(
          TestClass2::shared_from_this());
    }
  };

public:
  class TestClass4;

public:
  class TestClass5 : public TestClass4
  {
    GET_CLASS_NAME(TestClass5)

  protected:
    std::shared_ptr<TestClass5> shared_from_this()
    {
      return std::static_pointer_cast<TestClass5>(
          TestClass4::shared_from_this());
    }
  };

public:
  virtual void testGeneral();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"rawtypes","unchecked"}) public void
  // testExceptions()
  virtual void testExceptions();

protected:
  std::shared_ptr<TestVirtualMethod> shared_from_this()
  {
    return std::static_pointer_cast<TestVirtualMethod>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
class TestVirtualMethod::TestClass1 : public TestVirtualMethod
{
  GET_CLASS_NAME(TestVirtualMethod::TestClass1)
public:
  void publicTest(const std::wstring &test) override;

protected:
  void protectedTest(int test) override;

protected:
  std::shared_ptr<TestClass1> shared_from_this()
  {
    return std::static_pointer_cast<TestClass1>(
        TestVirtualMethod::shared_from_this());
  }
};
class TestVirtualMethod::TestClass4 : public TestVirtualMethod
{
  GET_CLASS_NAME(TestVirtualMethod::TestClass4)

protected:
  std::shared_ptr<TestClass4> shared_from_this()
  {
    return std::static_pointer_cast<TestClass4>(
        TestVirtualMethod::shared_from_this());
  }
};
