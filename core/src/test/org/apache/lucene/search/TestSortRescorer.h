#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
namespace org::apache::lucene::search
{

using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSortRescorer : public LuceneTestCase
{
  GET_CLASS_NAME(TestSortRescorer)
public:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<DirectoryReader> reader;
  std::shared_ptr<Directory> dir;

  void setUp()  override;

  void tearDown()  override;

  virtual void testBasic() ;

  virtual void testDoubleValuesSourceSort() ;

  virtual void testRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<int>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestSortRescorer> outerInstance;

    std::deque<int> idToNum;
    std::shared_ptr<IndexReader> r;
    int reverseInt = 0;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestSortRescorer> outerInstance,
        std::deque<int> &idToNum, std::shared_ptr<IndexReader> r,
        int reverseInt);

    int compare(std::optional<int> &a, std::optional<int> &b);
  };

protected:
  std::shared_ptr<TestSortRescorer> shared_from_this()
  {
    return std::static_pointer_cast<TestSortRescorer>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
