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

class TestFilterIterator : public LuceneTestCase
{
  GET_CLASS_NAME(TestFilterIterator)

private:
  static const std::shared_ptr<Set<std::wstring>> set;

  template <typename T1>
  static void assertNoMore(std::shared_ptr<Iterator<T1>> it);

public:
  virtual void testEmpty();

private:
  class FilterIteratorAnonymousInnerClass
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testA1();

private:
  class FilterIteratorAnonymousInnerClass2
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass2(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass2>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testA2();

private:
  class FilterIteratorAnonymousInnerClass3
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass3(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass3>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testB1();

private:
  class FilterIteratorAnonymousInnerClass4
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass4(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass4>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testB2();

private:
  class FilterIteratorAnonymousInnerClass5
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass5)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass5(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass5>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testAll1();

private:
  class FilterIteratorAnonymousInnerClass6
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass6)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass6(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass6>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testAll2();

private:
  class FilterIteratorAnonymousInnerClass7
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass7)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass7(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass7>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

public:
  virtual void testUnmodifiable();

private:
  class FilterIteratorAnonymousInnerClass8
      : public FilterIterator<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(FilterIteratorAnonymousInnerClass8)
  private:
    std::shared_ptr<TestFilterIterator> outerInstance;

  public:
    FilterIteratorAnonymousInnerClass8(
        std::shared_ptr<TestFilterIterator> outerInstance,
        std::shared_ptr<UnknownType> iterator);

  protected:
    bool predicateFunction(const std::wstring &s) override;

  protected:
    std::shared_ptr<FilterIteratorAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<FilterIteratorAnonymousInnerClass8>(
          FilterIterator<std::wstring, std::wstring>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestFilterIterator> shared_from_this()
  {
    return std::static_pointer_cast<TestFilterIterator>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
