#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"

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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

/**
 * A Weight that has a constant score equal to the boost of the wrapped query.
 * This is typically useful when building queries which do not produce
 * meaningful scores and are mostly useful for filtering.
 *
 * @lucene.internal
 */
class ConstantScoreWeight : public Weight
{
  GET_CLASS_NAME(ConstantScoreWeight)

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const float score_;

protected:
  ConstantScoreWeight(std::shared_ptr<Query> query, float score);

public:
  void extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  /** Return the score produced by this {@link Weight}. */
protected:
  float score();

public:
  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

protected:
  std::shared_ptr<ConstantScoreWeight> shared_from_this()
  {
    return std::static_pointer_cast<ConstantScoreWeight>(
        Weight::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
