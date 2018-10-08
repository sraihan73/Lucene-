#pragma once
#include "../../../../../../java/org/apache/lucene/classification/Classifier.h"
#include "../ClassificationTestBase.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::classification
{
template <typename T>
class ClassificationResult;
}
namespace org::apache::lucene::classification::utils
{
class ConfusionMatrixGenerator;
}
namespace org::apache::lucene::classification::utils
{
class ConfusionMatrix;
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
namespace org::apache::lucene::classification::utils
{

using ClassificationTestBase =
    org::apache::lucene::classification::ClassificationTestBase;

/**
 * Tests for {@link ConfusionMatrixGenerator}
 */
class ConfusionMatrixGeneratorTest : public ClassificationTestBase<std::any>
{
  GET_CLASS_NAME(ConfusionMatrixGeneratorTest)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrix() throws Exception
  virtual void testGetConfusionMatrix() ;

private:
  class ClassifierAnonymousInnerClass
      : public std::enable_shared_from_this<ClassifierAnonymousInnerClass>,
        public Classifier<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ClassifierAnonymousInnerClass)
  private:
    std::shared_ptr<ConfusionMatrixGeneratorTest> outerInstance;

  public:
    ClassifierAnonymousInnerClass(
        std::shared_ptr<ConfusionMatrixGeneratorTest> outerInstance);

    std::shared_ptr<ClassificationResult<std::shared_ptr<BytesRef>>>
    assignClass(const std::wstring &text) ;

    std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
    getClasses(const std::wstring &text)  override;

    std::deque<ClassificationResult<std::shared_ptr<BytesRef>>>
    getClasses(const std::wstring &text, int max)  override;
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithSNB() throws
  // Exception
  virtual void testGetConfusionMatrixWithSNB() ;

private:
  void checkCM(std::shared_ptr<ConfusionMatrixGenerator::ConfusionMatrix>
                   confusionMatrix);

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithBM25NB() throws
  // Exception
  virtual void testGetConfusionMatrixWithBM25NB() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithCNB() throws
  // Exception
  virtual void testGetConfusionMatrixWithCNB() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithKNN() throws
  // Exception
  virtual void testGetConfusionMatrixWithKNN() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithFLTKNN() throws
  // Exception
  virtual void testGetConfusionMatrixWithFLTKNN() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testGetConfusionMatrixWithBP() throws
  // Exception
  virtual void testGetConfusionMatrixWithBP() ;

protected:
  std::shared_ptr<ConfusionMatrixGeneratorTest> shared_from_this()
  {
    return std::static_pointer_cast<ConfusionMatrixGeneratorTest>(
        org.apache.lucene.classification
            .ClassificationTestBase<Object>::shared_from_this());
  }
};
} // namespace org::apache::lucene::classification::utils
