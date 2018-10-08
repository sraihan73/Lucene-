#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"

#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/ContextMetaData.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionQuery.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionWeight.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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
namespace org::apache::lucene::search::suggest::document
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Weight = org::apache::lucene::search::Weight;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;

/**
 * A {@link CompletionQuery} that matches documents specified by
 * a wrapped {@link CompletionQuery} supporting boosting and/or filtering
 * by specified contexts.
 * <p>
 * Use this query against {@link ContextSuggestField}
 * <p>
 * Example of using a {@link CompletionQuery} with boosted
 * contexts:
 * <pre class="prettyprint">
 *  CompletionQuery completionQuery = ...;
 *  ContextQuery query = new ContextQuery(completionQuery);
 *  query.addContext("context1", 2);
 *  query.addContext("context2", 1);
 * </pre>
 * <p>
 * NOTE:
 * <ul>
 *   <li>
 *    This query can be constructed with
 *    {@link PrefixCompletionQuery}, {@link RegexCompletionQuery}
 *    or {@link FuzzyCompletionQuery} query.
 *   </li>
 *   <li>
 *     To suggest across all contexts, use {@link #addAllContexts()}.
 *     When no context is added, the default behaviour is to suggest across
 *     all contexts.
 *   </li>
 *   <li>
 *     To apply the same boost to multiple contexts sharing the same prefix,
 *     Use {@link #addContext(std::wstring, float, bool)} with the common
 *     context prefix, boost and set <code>exact</code> to false.
 *   <li>
 *     Using this query against a {@link SuggestField} (not context enabled),
 *     would yield results ignoring any context filtering/boosting
 *   </li>
 * </ul>
 *
 * @lucene.experimental
 */
class ContextQuery : public CompletionQuery
{
  GET_CLASS_NAME(ContextQuery)
private:
  std::shared_ptr<IntsRefBuilder> scratch = std::make_shared<IntsRefBuilder>();
  std::unordered_map<std::shared_ptr<IntsRef>, std::shared_ptr<ContextMetaData>>
      contexts;
  bool matchAllContexts = false;
  /** Inner completion query */
protected:
  std::shared_ptr<CompletionQuery> innerQuery;

  /**
   * Constructs a context completion query that matches
   * documents specified by <code>query</code>.
   * <p>
   * Use {@link #addContext(std::wstring, float, bool)}
   * to add context(s) with boost
   */
public:
  ContextQuery(std::shared_ptr<CompletionQuery> query);

  /**
   * Adds an exact context with default boost of 1
   */
  virtual void addContext(std::shared_ptr<std::wstring> context);

  /**
   * Adds an exact context with boost
   */
  virtual void addContext(std::shared_ptr<std::wstring> context, float boost);

  /**
   * Adds a context with boost, set <code>exact</code> to false
   * if the context is a prefix of any indexed contexts
   */
  virtual void addContext(std::shared_ptr<std::wstring> context, float boost,
                          bool exact);

  /**
   * Add all contexts with a boost of 1f
   */
  virtual void addAllContexts();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  static std::shared_ptr<Automaton> toContextAutomaton(
      std::unordered_map<std::shared_ptr<IntsRef>,
                         std::shared_ptr<ContextMetaData>> &contexts,
      bool const matchAllContexts);

  /**
   * Holder for context value meta data
   */
private:
  class ContextMetaData : public std::enable_shared_from_this<ContextMetaData>
  {
    GET_CLASS_NAME(ContextMetaData)

    /**
     * Boost associated with a
     * context value
     */
  private:
    const float boost;

    /**
     * flag to indicate whether the context
     * value should be treated as an exact
     * value or a context prefix
     */
    const bool exact;

    ContextMetaData(float boost, bool exact);
  };

private:
  class ContextCompletionWeight : public CompletionWeight
  {
    GET_CLASS_NAME(ContextCompletionWeight)

  private:
    const std::unordered_map<std::shared_ptr<IntsRef>, float> contextMap;
    std::deque<int> const contextLengths;
    const std::shared_ptr<CompletionWeight> innerWeight;
    const std::shared_ptr<BytesRefBuilder> scratch =
        std::make_shared<BytesRefBuilder>();

    float currentBoost = 0;
    std::shared_ptr<std::wstring> currentContext;

  public:
    ContextCompletionWeight(
        std::shared_ptr<CompletionQuery> query,
        std::shared_ptr<Automaton> automaton,
        std::shared_ptr<CompletionWeight> innerWeight,
        std::unordered_map<std::shared_ptr<IntsRef>, float> &contextMap,
        std::deque<int> &contextLengths) ;

  protected:
    void setNextMatch(std::shared_ptr<IntsRef> pathPrefix) override;

  private:
    void setInnerWeight(std::shared_ptr<IntsRef> ref, int offset);

  protected:
    std::shared_ptr<std::wstring> context() override;

    float boost() override;

  protected:
    std::shared_ptr<ContextCompletionWeight> shared_from_this()
    {
      return std::static_pointer_cast<ContextCompletionWeight>(
          CompletionWeight::shared_from_this());
    }
  };

public:
  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<ContextQuery> shared_from_this()
  {
    return std::static_pointer_cast<ContextQuery>(
        CompletionQuery::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
