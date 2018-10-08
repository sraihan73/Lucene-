#pragma once
#include "DocumentClassificationTestBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::classification::document
{

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Tests for {@link org.apache.lucene.classification.SimpleNaiveBayesClassifier}
 */
class SimpleNaiveBayesDocumentClassifierTest
    : public DocumentClassificationTestBase<std::shared_ptr<BytesRef>>
{
  GET_CLASS_NAME(SimpleNaiveBayesDocumentClassifierTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicDocumentClassification() throws
  // Exception
  virtual void testBasicDocumentClassification() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBasicDocumentClassificationScore()
  // throws Exception
  virtual void testBasicDocumentClassificationScore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testBoostedDocumentClassification() throws
  // Exception
  virtual void testBoostedDocumentClassification() ;

protected:
  std::shared_ptr<SimpleNaiveBayesDocumentClassifierTest> shared_from_this()
  {
    return std::static_pointer_cast<SimpleNaiveBayesDocumentClassifierTest>(
        DocumentClassificationTestBase<
            org.apache.lucene.util.BytesRef>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/classification/document/
