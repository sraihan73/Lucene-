#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"

#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/document/Field.h"

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Simple test that adds numeric terms, where each term has the
 * totalTermFreq of its integer value, and checks that the totalTermFreq is
 * correct.
 */
// TODO: somehow factor this with BagOfPostings? it's almost the same
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({"Direct", "Memory"}) public class
// TestBagOfPositions extends org.apache.lucene.util.LuceneTestCase
class TestBagOfPositions : public LuceneTestCase
{
public:
  virtual void test() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestBagOfPositions> outerInstance;

    int numTerms = 0;
    int maxTermsPerDoc = 0;
    std::shared_ptr<ConcurrentLinkedQueue<std::wstring>> postings;
    std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> iw;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<Random> threadRandom;
    std::shared_ptr<Document> document;
    std::shared_ptr<Field> field;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestBagOfPositions> outerInstance, int numTerms,
        int maxTermsPerDoc,
        std::shared_ptr<ConcurrentLinkedQueue<std::wstring>> postings,
        std::shared_ptr<org::apache::lucene::index::RandomIndexWriter> iw,
        std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<Random> threadRandom,
        std::shared_ptr<Document> document, std::shared_ptr<Field> field);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBagOfPositions> shared_from_this()
  {
    return std::static_pointer_cast<TestBagOfPositions>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
