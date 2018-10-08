#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;

/** Assertion-enabled query. */
class AssertingQuery final : public Query
{
  GET_CLASS_NAME(AssertingQuery)

private:
  const std::shared_ptr<Random> random;
  const std::shared_ptr<Query> in_;

  /** Sole constructor. */
public:
  AssertingQuery(std::shared_ptr<Random> random, std::shared_ptr<Query> in_);

  /** Wrap a query if necessary. */
  static std::shared_ptr<Query> wrap(std::shared_ptr<Random> random,
                                     std::shared_ptr<Query> query);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any other) override;

  virtual int hashCode();

  std::shared_ptr<Random> getRandom();

  std::shared_ptr<Query> getIn();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

protected:
  std::shared_ptr<AssertingQuery> shared_from_this()
  {
    return std::static_pointer_cast<AssertingQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
