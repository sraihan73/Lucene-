#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/payloads/PayloadFunction.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

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
namespace org::apache::lucene::queries::payloads
{

using BaseExplanationTestCase =
    org::apache::lucene::search::BaseExplanationTestCase;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * TestExplanations subclass focusing on payload queries
 */
class TestPayloadExplanations : public BaseExplanationTestCase
{
  GET_CLASS_NAME(TestPayloadExplanations)

private:
  static std::deque<std::shared_ptr<PayloadFunction>> functions;

public:
  void setUp()  override;

private:
  class ClassicSimilarityAnonymousInnerClass : public ClassicSimilarity
  {
    GET_CLASS_NAME(ClassicSimilarityAnonymousInnerClass)
  private:
    std::shared_ptr<TestPayloadExplanations> outerInstance;

  public:
    ClassicSimilarityAnonymousInnerClass(
        std::shared_ptr<TestPayloadExplanations> outerInstance);

    float scorePayload(int doc, int start, int end,
                       std::shared_ptr<BytesRef> payload) override;

  protected:
    std::shared_ptr<ClassicSimilarityAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ClassicSimilarityAnonymousInnerClass>(
          org.apache.lucene.search.similarities
              .ClassicSimilarity::shared_from_this());
    }
  };

  /** macro for payloadscorequery */
private:
  std::shared_ptr<SpanQuery> pt(const std::wstring &s,
                                std::shared_ptr<PayloadFunction> fn);

  /* simple PayloadTermQueries */

public:
  virtual void testPT1() ;

  virtual void testPT2() ;

  virtual void testPT4() ;

  virtual void testPT5() ;

  /*
    protected static final std::wstring[] docFields = {
    "w1 w2 w3 w4 w5",
    "w1 w3 w2 w3 zz",
    "w1 xx w2 yy w3",
    "w1 w3 xx w2 yy w3 zz"
  };
   */

  virtual void
  testAllFunctions(std::shared_ptr<SpanQuery> query,
                   std::deque<int> &expected) ;

  virtual void testSimpleTerm() ;

  virtual void testOrTerm() ;

  virtual void testOrderedNearQuery() ;

  virtual void testUnorderedNearQuery() ;

protected:
  std::shared_ptr<TestPayloadExplanations> shared_from_this()
  {
    return std::static_pointer_cast<TestPayloadExplanations>(
        org.apache.lucene.search.BaseExplanationTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/payloads/
