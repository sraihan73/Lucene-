#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

using LeafReader = org::apache::lucene::index::LeafReader;

/**
 * A {@link Query} that matches documents that have a value for a given field
 * as reported by doc values iterators.
 */
class DocValuesFieldExistsQuery final : public Query
{
  GET_CLASS_NAME(DocValuesFieldExistsQuery)

private:
  const std::wstring field;

  /** Create a query that will match that have a value for the given
   *  {@code field}. */
public:
  DocValuesFieldExistsQuery(const std::wstring &field);

  std::wstring getField();

  bool equals(std::any other) override;

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores, float boost) override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesFieldExistsQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<DocValuesFieldExistsQuery> outerInstance, float boost);

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

  /**
   * Returns a {@link DocIdSetIterator} from the given field or null if the
   * field doesn't exist in the reader or if the reader has no doc values for
   * the field.
   */
public:
  static std::shared_ptr<DocIdSetIterator> getDocValuesDocIdSetIterator(
      const std::wstring &field,
      std::shared_ptr<LeafReader> reader) ;

protected:
  std::shared_ptr<DocValuesFieldExistsQuery> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesFieldExistsQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
