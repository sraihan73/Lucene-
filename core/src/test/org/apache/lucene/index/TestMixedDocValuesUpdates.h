#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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

using Field = org::apache::lucene::document::Field;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMixedDocValuesUpdates : public LuceneTestCase
{
  GET_CLASS_NAME(TestMixedDocValuesUpdates)

public:
  virtual void testManyReopensAndFields() ;

  virtual void testStressMultiThreading() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestMixedDocValuesUpdates> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    int numFields = 0;
    int numDocs = 0;
    std::shared_ptr<CountDownLatch> done;
    std::shared_ptr<AtomicInteger> numUpdates;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestMixedDocValuesUpdates> outerInstance,
        std::wstring L"UpdateThread-" + i,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        int numFields, int numDocs, std::shared_ptr<CountDownLatch> done,
        std::shared_ptr<AtomicInteger> numUpdates);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void
  testUpdateDifferentDocsInDifferentGens() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testTonsOfUpdates() throws Exception
  virtual void testTonsOfUpdates() ;

  virtual void testTryUpdateDocValues() ;

  virtual void testTryUpdateMultiThreaded() throw(IOException,
                                                  BrokenBarrierException,
                                                  InterruptedException);

  static void doUpdate(std::shared_ptr<Term> doc,
                       std::shared_ptr<IndexWriter> writer,
                       std::deque<Field> &fields) ;

  virtual void testResetValue() ;

  virtual void testResetValueMultipleDocs() ;

  virtual void testUpdateNotExistingFieldDV() ;

  virtual void testUpdateFieldWithNoPreviousDocValues() ;

protected:
  std::shared_ptr<TestMixedDocValuesUpdates> shared_from_this()
  {
    return std::static_pointer_cast<TestMixedDocValuesUpdates>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
