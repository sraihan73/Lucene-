#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Directory = org::apache::lucene::store::Directory;

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestThreadedForceMerge : public LuceneTestCase
{
  GET_CLASS_NAME(TestThreadedForceMerge)

private:
  static std::shared_ptr<Analyzer> ANALYZER;

  static constexpr int NUM_THREADS = 3;
  // private final static int NUM_THREADS = 5;

  static constexpr int NUM_ITER = 1;

  static constexpr int NUM_ITER2 = 1;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool failed;
  bool failed = false;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @BeforeClass public static void setup()
  static void setup();

private:
  void setFailed();

public:
  virtual void
  runTest(std::shared_ptr<Random> random,
          std::shared_ptr<Directory> directory) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestThreadedForceMerge> outerInstance;

    int iterFinal = 0;
    std::shared_ptr<FieldType> customType;
    int iFinal = 0;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> writerFinal;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestThreadedForceMerge> outerInstance, int iterFinal,
        std::shared_ptr<FieldType> customType, int iFinal,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writerFinal);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /*
    Run above stress test against RAMDirectory and then
    FSDirectory.
  */
public:
  virtual void testThreadedForceMerge() ;

protected:
  std::shared_ptr<TestThreadedForceMerge> shared_from_this()
  {
    return std::static_pointer_cast<TestThreadedForceMerge>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
