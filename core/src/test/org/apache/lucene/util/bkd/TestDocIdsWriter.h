#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class PointValues;
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
namespace org::apache::lucene::util::bkd
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestDocIdsWriter : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocIdsWriter)

public:
  virtual void testRandom() ;

  virtual void testSorted() ;

private:
  void test(std::shared_ptr<Directory> dir,
            std::deque<int> &ints) ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocIdsWriter> outerInstance;

    std::deque<int> read;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<TestDocIdsWriter> outerInstance,
        std::deque<int> &read);

    int i = 0;
    void visit(int docID)  override;

    void visit(int docID,
               std::deque<char> &packedValue)  override;

    Relation compare(std::deque<char> &minPackedValue,
                     std::deque<char> &maxPackedValue);
  };

protected:
  std::shared_ptr<TestDocIdsWriter> shared_from_this()
  {
    return std::static_pointer_cast<TestDocIdsWriter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::bkd
