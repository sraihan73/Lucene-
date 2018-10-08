#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class MergePolicy;
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterMergePolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterMergePolicy)

  // Test the normal case
public:
  virtual void testNormalCase() ;

  // Test to see if there is over merge
  virtual void testNoOverMerge() ;

  // Test the case where flush is forced after every addDoc
  virtual void testForceFlush() ;

  // Test the case where mergeFactor changes
  virtual void testMergeFactorChange() ;

  // Test the case where both mergeFactor and maxBufferedDocs change
  virtual void testMaxBufferedDocsChange() ;

  // Test the case where a merge results in no doc at all
  virtual void testMergeDocCount0() ;

private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

  void checkInvariants(std::shared_ptr<IndexWriter> writer) ;

  static constexpr double EPSILON = 1E-14;

public:
  virtual void testSetters();

private:
  void assertSetters(std::shared_ptr<MergePolicy> lmp);

protected:
  std::shared_ptr<TestIndexWriterMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterMergePolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
