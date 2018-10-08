#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanCollector.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"

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

using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Holds all implementations of classes in the o.a.l.s.spans package as a
 * back-compatibility test. It does not run any tests per-se, however if
 * someone adds a method to an interface or abstract method to an abstract
 * class, one of the implementations here will fail to compile and so we know
 * back-compat policy was violated.
 */
class JustCompileSearchSpans final
    : public std::enable_shared_from_this<JustCompileSearchSpans>
{
  GET_CLASS_NAME(JustCompileSearchSpans)

private:
  static const std::wstring UNSUPPORTED_MSG;

public:
  class JustCompileSpans final : public Spans
  {
    GET_CLASS_NAME(JustCompileSpans)

  public:
    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int startPosition() override;

    int endPosition() override;

    int width() override;

    void collect(std::shared_ptr<SpanCollector> collector) throw(
        IOException) override;

    int nextStartPosition()  override;

    int64_t cost() override;

    float positionsCost() override;

  protected:
    std::shared_ptr<JustCompileSpans> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileSpans>(
          Spans::shared_from_this());
    }
  };

public:
  class JustCompileSpanQuery final : public SpanQuery
  {
    GET_CLASS_NAME(JustCompileSpanQuery)

  public:
    std::wstring getField() override;

    std::shared_ptr<SpanWeight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

    std::wstring toString(const std::wstring &field) override;

    virtual bool equals(std::any o);

    virtual int hashCode();

  protected:
    std::shared_ptr<JustCompileSpanQuery> shared_from_this()
    {
      return std::static_pointer_cast<JustCompileSpanQuery>(
          SpanQuery::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
