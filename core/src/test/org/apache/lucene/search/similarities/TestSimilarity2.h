#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class Similarity;
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
namespace org::apache::lucene::search::similarities
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests against all the similarities we have
 */
class TestSimilarity2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestSimilarity2)
public:
  std::deque<std::shared_ptr<Similarity>> sims;

  void setUp()  override;

  /** because of stupid things like querynorm, it's possible we computeStats on
   * a field that doesnt exist at all test this against a totally empty index,
   * to make sure sims handle it
   */
  virtual void testEmptyIndex() ;

  /** similar to the above, but ORs the query with a real field */
  virtual void testEmptyField() ;

  /** similar to the above, however the field exists, but we query with a term
   * that doesnt exist too */
  virtual void testEmptyTerm() ;

  /** make sure we can retrieve when norms are disabled */
  virtual void testNoNorms() ;

  /** make sure scores are not skewed by docs not containing the field */
  virtual void testNoFieldSkew() ;

  /** make sure all sims work if TF is omitted */
  virtual void testOmitTF() ;

  /** make sure all sims work if TF and norms is omitted */
  virtual void testOmitTFAndNorms() ;

  /** make sure all sims work with spanOR(termX, termY) where termY does not
   * exist */
  virtual void testCrazySpans() ;

protected:
  std::shared_ptr<TestSimilarity2> shared_from_this()
  {
    return std::static_pointer_cast<TestSimilarity2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
