#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class IndexWriterConfig;
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

class TestSizeBoundedForceMerge : public LuceneTestCase
{
  GET_CLASS_NAME(TestSizeBoundedForceMerge)

private:
  void addDocs(std::shared_ptr<IndexWriter> writer,
               int numDocs) ;

  void addDocs(std::shared_ptr<IndexWriter> writer, int numDocs,
               bool withID) ;

  static std::shared_ptr<IndexWriterConfig> newWriterConfig();

public:
  virtual void testByteSizeLimit() ;

  virtual void testNumDocsLimit() ;

  virtual void testLastSegmentTooLarge() ;

  virtual void testFirstSegmentTooLarge() ;

  virtual void testAllSegmentsSmall() ;

  virtual void testAllSegmentsLarge() ;

  virtual void testOneLargeOneSmall() ;

  virtual void testMergeFactor() ;

  virtual void testSingleMergeableSegment() ;

  virtual void testSingleNonMergeableSegment() ;

  virtual void testSingleMergeableTooLargeSegment() ;

protected:
  std::shared_ptr<TestSizeBoundedForceMerge> shared_from_this()
  {
    return std::static_pointer_cast<TestSizeBoundedForceMerge>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
