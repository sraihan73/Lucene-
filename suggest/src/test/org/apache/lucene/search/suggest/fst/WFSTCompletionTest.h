#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <map_obj>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class Lookup;
}

namespace org::apache::lucene::search::suggest
{
class LookupResult;
}
namespace org::apache::lucene::store
{
class Directory;
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
namespace org::apache::lucene::search::suggest::fst
{

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class WFSTCompletionTest : public LuceneTestCase
{
  GET_CLASS_NAME(WFSTCompletionTest)

public:
  virtual void testBasic() ;

  virtual void testExactFirst() ;

  virtual void testNonExactFirst() ;

  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<WFSTCompletionTest> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<WFSTCompletionTest> outerInstance);

    int compare(std::shared_ptr<LookupResult> left,
                std::shared_ptr<LookupResult> right);
  };

public:
  virtual void test0ByteKeys() ;

  virtual void testEmpty() ;

private:
  std::shared_ptr<Directory> getDirectory();

protected:
  std::shared_ptr<WFSTCompletionTest> shared_from_this()
  {
    return std::static_pointer_cast<WFSTCompletionTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::fst
