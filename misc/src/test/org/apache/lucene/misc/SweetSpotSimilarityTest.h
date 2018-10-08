#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"

#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/misc/SweetSpotSimilarity.h"

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
namespace org::apache::lucene::misc
{

using Explanation = org::apache::lucene::search::Explanation;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test of the SweetSpotSimilarity
 */
class SweetSpotSimilarityTest : public LuceneTestCase
{
  GET_CLASS_NAME(SweetSpotSimilarityTest)

private:
  static float computeNorm(std::shared_ptr<Similarity> sim,
                           const std::wstring &field,
                           int length) ;

  static std::shared_ptr<Explanation>
  findExplanation(std::shared_ptr<Explanation> expl, const std::wstring &text);

public:
  virtual void testSweetSpotComputeNorm() ;

private:
  class PerFieldSimilarityWrapperAnonymousInnerClass
      : public PerFieldSimilarityWrapper
  {
    GET_CLASS_NAME(PerFieldSimilarityWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<SweetSpotSimilarityTest> outerInstance;

    std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ss;
    std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssBar;
    std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssYak;
    std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssA;
    std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssB;

  public:
    PerFieldSimilarityWrapperAnonymousInnerClass(
        std::shared_ptr<SweetSpotSimilarityTest> outerInstance,
        std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ss,
        std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssBar,
        std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssYak,
        std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssA,
        std::shared_ptr<org::apache::lucene::misc::SweetSpotSimilarity> ssB);

    std::shared_ptr<Similarity> get(const std::wstring &field) override;

  protected:
    std::shared_ptr<PerFieldSimilarityWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PerFieldSimilarityWrapperAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .PerFieldSimilarityWrapper::shared_from_this());
    }
  };

public:
  virtual void testSweetSpotTf();

  virtual void testHyperbolicSweetSpot();

private:
  class SweetSpotSimilarityAnonymousInnerClass : public SweetSpotSimilarity
  {
    GET_CLASS_NAME(SweetSpotSimilarityAnonymousInnerClass)
  private:
    std::shared_ptr<SweetSpotSimilarityTest> outerInstance;

  public:
    SweetSpotSimilarityAnonymousInnerClass(
        std::shared_ptr<SweetSpotSimilarityTest> outerInstance);

    float tf(float freq) override;

  protected:
    std::shared_ptr<SweetSpotSimilarityAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SweetSpotSimilarityAnonymousInnerClass>(
          SweetSpotSimilarity::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SweetSpotSimilarityTest> shared_from_this()
  {
    return std::static_pointer_cast<SweetSpotSimilarityTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/misc/
