#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
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
namespace org::apache::lucene::search
{

/**
 * A {@link Query} that matches documents that have a value for a given field
 * as reported by field norms.  This will not work for fields that omit norms,
 * e.g. {@link StringField}.
 */
class NormsFieldExistsQuery final : public Query
{
  GET_CLASS_NAME(NormsFieldExistsQuery)

private:
  const std::wstring field;

  /** Create a query that will match that have a value for the given
   *  {@code field}. */
public:
  NormsFieldExistsQuery(const std::wstring &field);

  std::wstring getField();

  bool equals(std::any other) override;

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<NormsFieldExistsQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<NormsFieldExistsQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<NormsFieldExistsQuery> shared_from_this()
  {
    return std::static_pointer_cast<NormsFieldExistsQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
