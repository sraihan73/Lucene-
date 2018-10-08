#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::queries::function
{
class ValueFiller;
}
namespace org::apache::lucene::util::@ mutable { class MutableValueFloat; }
namespace org::apache::lucene::util::@ mutable { class MutableValue; }

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
namespace org::apache::lucene::queries::function::valuesource
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using FloatDocValues =
    org::apache::lucene::queries::function::docvalues::FloatDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

/**
 * <code>QueryValueSource</code> returns the relevance score of the query
 */
class QueryValueSource : public ValueSource
{
  GET_CLASS_NAME(QueryValueSource)
public:
  const std::shared_ptr<Query> q;
  const float defVal;

  QueryValueSource(std::shared_ptr<Query> q, float defVal);

  virtual std::shared_ptr<Query> getQuery();
  virtual float getDefaultValue();

  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map fcontext,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

  virtual int hashCode();

  virtual bool equals(std::any o);

  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

protected:
  std::shared_ptr<QueryValueSource> shared_from_this()
  {
    return std::static_pointer_cast<QueryValueSource>(
        org.apache.lucene.queries.function.ValueSource::shared_from_this());
  }
};

class QueryDocValues : public FloatDocValues
{
  GET_CLASS_NAME(QueryDocValues)
public:
  const std::shared_ptr<LeafReaderContext> readerContext;
  const std::shared_ptr<Weight> weight;
  const float defVal;
  const std::unordered_map fcontext;
  const std::shared_ptr<Query> q;

  std::shared_ptr<Scorer> scorer;
  std::shared_ptr<DocIdSetIterator> it;
  int scorerDoc = 0; // the document the scorer is on
  bool noMatches = false;

  // the last document requested... start off with high value
  // to trigger a scorer reset on first access.
  int lastDocRequested = std::numeric_limits<int>::max();

  QueryDocValues(std::shared_ptr<QueryValueSource> vs,
                 std::shared_ptr<LeafReaderContext> readerContext,
                 std::unordered_map fcontext) ;

  float floatVal(int doc) override;

  bool exists(int doc) override;

  std::any objectVal(int doc) override;

  std::shared_ptr<FunctionValues::ValueFiller> getValueFiller() override;

private:
  class ValueFillerAnonymousInnerClass : public ValueFiller
  {
    GET_CLASS_NAME(ValueFillerAnonymousInnerClass)
  private:
    std::shared_ptr<QueryDocValues> outerInstance;

  public:
    ValueFillerAnonymousInnerClass(
        std::shared_ptr<QueryDocValues> outerInstance);

  private:
    const std::shared_ptr<MutableValueFloat> mval;

  public:
    std::shared_ptr<MutableValue> getValue() override;

    void fillValue(int doc) override;

  protected:
    std::shared_ptr<ValueFillerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ValueFillerAnonymousInnerClass>(
          ValueFiller::shared_from_this());
    }
  };

public:
  std::wstring toString(int doc) override;

protected:
  std::shared_ptr<QueryDocValues> shared_from_this()
  {
    return std::static_pointer_cast<QueryDocValues>(
        org.apache.lucene.queries.function.docvalues
            .FloatDocValues::shared_from_this());
  }
};
} // namespace org::apache::lucene::queries::function::valuesource
