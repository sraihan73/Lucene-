#pragma once
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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

using Explanation = org::apache::lucene::search::Explanation;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBM25Similarity : public LuceneTestCase
{
  GET_CLASS_NAME(TestBM25Similarity)

public:
  virtual void testIllegalK1();

  virtual void testIllegalB();

  virtual void testLengthEncodingBackwardCompatibility() ;

private:
  static std::shared_ptr<Explanation>
  findExplanation(std::shared_ptr<Explanation> expl, const std::wstring &text);

protected:
  std::shared_ptr<TestBM25Similarity> shared_from_this()
  {
    return std::static_pointer_cast<TestBM25Similarity>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::similarities
