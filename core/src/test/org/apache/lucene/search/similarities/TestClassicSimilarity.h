#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Explanation;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestClassicSimilarity : public LuceneTestCase
{
  GET_CLASS_NAME(TestClassicSimilarity)
private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> indexReader;
  std::shared_ptr<IndexSearcher> indexSearcher;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testHit() ;

  virtual void testMiss() ;

  virtual void testEmpty() ;

  virtual void testBQHit() ;

  virtual void testBQHitOrMiss() ;

  virtual void testBQHitOrEmpty() ;

  virtual void testDMQHit() ;

  virtual void testDMQHitOrMiss() ;

  virtual void testDMQHitOrEmpty() ;

  virtual void testSaneNormValues() ;

  virtual void testNormEncodingBackwardCompatibility() ;

private:
  static std::shared_ptr<Explanation>
  findExplanation(std::shared_ptr<Explanation> expl, const std::wstring &text);

public:
  virtual void testSameNormsAsBM25();

protected:
  std::shared_ptr<TestClassicSimilarity> shared_from_this()
  {
    return std::static_pointer_cast<TestClassicSimilarity>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
