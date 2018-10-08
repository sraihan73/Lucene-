#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::expressions
{

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Scorer = org::apache::lucene::search::Scorer;

class FakeScorer : public Scorer
{
  GET_CLASS_NAME(FakeScorer)

public:
  // C++ NOTE: Fields cannot have the same name as methods:
  float score_ = 0;
  int doc = -1;

  FakeScorer();

  int docID() override;

  std::shared_ptr<DocIdSetIterator> iterator() override;

  float score()  override;

protected:
  std::shared_ptr<FakeScorer> shared_from_this()
  {
    return std::static_pointer_cast<FakeScorer>(
        org.apache.lucene.search.Scorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::expressions
