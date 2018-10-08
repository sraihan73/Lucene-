#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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
namespace org::apache::lucene::index
{

using namespace org::apache::lucene::search;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestParallelLeafReader : public LuceneTestCase
{
  GET_CLASS_NAME(TestParallelLeafReader)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<IndexSearcher> parallel_, single_;
  std::shared_ptr<Directory> dir, dir1, dir2;

public:
  virtual void testQueries() ;

  virtual void testFieldNames() ;

  virtual void testRefCounts1() ;

  virtual void testRefCounts2() ;

  virtual void testCloseInnerReader() ;

  virtual void testIncompatibleIndexes() ;

  virtual void testIgnoreStoredFields() ;

private:
  void queryTest(std::shared_ptr<Query> query) ;

  // Fields 1-4 indexed together:
  std::shared_ptr<IndexSearcher>
  single(std::shared_ptr<Random> random) ;

  // Fields 1 & 2 in one index, 3 & 4 in other, with ParallelReader:
  std::shared_ptr<IndexSearcher>
  parallel(std::shared_ptr<Random> random) ;

  std::shared_ptr<Directory>
  getDir1(std::shared_ptr<Random> random) ;

  std::shared_ptr<Directory>
  getDir2(std::shared_ptr<Random> random) ;

  // not ok to have one leaf w/ index sort and another with a different index
  // sort
public:
  virtual void testWithIndexSort1() ;

  // ok to have one leaf w/ index sort and the other with no sort
  virtual void testWithIndexSort2() ;

  virtual void testWithDocValuesUpdates() ;

protected:
  std::shared_ptr<TestParallelLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TestParallelLeafReader>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
