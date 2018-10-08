#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
namespace org::apache::lucene::search::spans
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

class SpanContainQuery : public SpanQuery, public Cloneable
{
  GET_CLASS_NAME(SpanContainQuery)

public:
  std::shared_ptr<SpanQuery> big;
  std::shared_ptr<SpanQuery> little;

  SpanContainQuery(std::shared_ptr<SpanQuery> big,
                   std::shared_ptr<SpanQuery> little);

  std::wstring getField() override;

  virtual std::shared_ptr<SpanQuery> getBig();

  virtual std::shared_ptr<SpanQuery> getLittle();

public:
  class SpanContainWeight : public SpanWeight
  {
    GET_CLASS_NAME(SpanContainWeight)
  private:
    std::shared_ptr<SpanContainQuery> outerInstance;

  public:
    const std::shared_ptr<SpanWeight> bigWeight;
    const std::shared_ptr<SpanWeight> littleWeight;

    SpanContainWeight(std::shared_ptr<SpanContainQuery> outerInstance,
                      std::shared_ptr<IndexSearcher> searcher,
                      std::unordered_map<std::shared_ptr<Term>,
                                         std::shared_ptr<TermContext>> &terms,
                      std::shared_ptr<SpanWeight> bigWeight,
                      std::shared_ptr<SpanWeight> littleWeight,
                      float boost) ;

    /**
     * Extract terms from both <code>big</code> and <code>little</code>.
     */
    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    virtual std::deque<std::shared_ptr<Spans>>
    prepareConjunction(std::shared_ptr<LeafReaderContext> context,
                       Postings postings) ;

    void extractTermContexts(
        std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
            &contexts) override;

  protected:
    std::shared_ptr<SpanContainWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanContainWeight>(
          SpanWeight::shared_from_this());
    }
  };

public:
  virtual std::wstring toString(const std::wstring &field,
                                const std::wstring &name);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<SpanContainQuery> other);

public:
  virtual int hashCode();

protected:
  std::shared_ptr<SpanContainQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanContainQuery>(
        SpanQuery::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/search/spans/
