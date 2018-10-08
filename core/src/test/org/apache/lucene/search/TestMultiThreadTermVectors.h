#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
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
class IndexReader;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
namespace org::apache::lucene::search
{

using namespace org::apache::lucene::document;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMultiThreadTermVectors : public LuceneTestCase
{
  GET_CLASS_NAME(TestMultiThreadTermVectors)
private:
  std::shared_ptr<Directory> directory;

public:
  int numDocs = 100;
  int numThreads = 3;

  void setUp()  override;

  void tearDown()  override;

  virtual void test() ;

  virtual void
  testTermPositionVectors(std::shared_ptr<IndexReader> reader,
                          int threadCount) ;

protected:
  std::shared_ptr<TestMultiThreadTermVectors> shared_from_this()
  {
    return std::static_pointer_cast<TestMultiThreadTermVectors>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

class MultiThreadTermVectorsReader
    : public std::enable_shared_from_this<MultiThreadTermVectorsReader>,
      public Runnable
{
  GET_CLASS_NAME(MultiThreadTermVectorsReader)

private:
  std::shared_ptr<IndexReader> reader = nullptr;
  std::shared_ptr<Thread> t = nullptr;

  const int runsToDo = 100;

public:
  int64_t timeElapsed = 0;

  virtual void init(std::shared_ptr<IndexReader> reader);

  virtual bool isAlive();

  void run() override;

private:
  void testTermVectors() ;

  void verifyVectors(std::shared_ptr<Fields> vectors,
                     int num) ;

  void verifyVector(std::shared_ptr<TermsEnum> deque,
                    int num) ;
};

} // namespace org::apache::lucene::search
