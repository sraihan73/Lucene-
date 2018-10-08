#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util
{
class FixedLengthBytesRefArray;
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

class TestFixedLengthBytesRefArray : public LuceneTestCase
{
  GET_CLASS_NAME(TestFixedLengthBytesRefArray)

public:
  virtual void testBasic() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestFixedLengthBytesRefArray> outerInstance;

    std::shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestFixedLengthBytesRefArray> outerInstance,
        std::shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a);

    int compare(std::shared_ptr<BytesRef> a, std::shared_ptr<BytesRef> b);
  };

public:
  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFixedLengthBytesRefArray> outerInstance;

    std::shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a;

  public:
    ComparatorAnonymousInnerClass2(
        std::shared_ptr<TestFixedLengthBytesRefArray> outerInstance,
        std::shared_ptr<org::apache::lucene::util::FixedLengthBytesRefArray> a);

    int compare(std::shared_ptr<BytesRef> a, std::shared_ptr<BytesRef> b);
  };

protected:
  std::shared_ptr<TestFixedLengthBytesRefArray> shared_from_this()
  {
    return std::static_pointer_cast<TestFixedLengthBytesRefArray>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
