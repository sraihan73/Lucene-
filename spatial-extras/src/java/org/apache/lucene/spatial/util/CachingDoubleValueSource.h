#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DoubleValuesSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/DoubleValues.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
namespace org::apache::lucene::spatial::util
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using DoubleValues = org::apache::lucene::search::DoubleValues;
using DoubleValuesSource = org::apache::lucene::search::DoubleValuesSource;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Caches the doubleVal of another value source in a HashMap
 * so that it is computed only once.
 * @lucene.internal
 */
class CachingDoubleValueSource : public DoubleValuesSource
{
  GET_CLASS_NAME(CachingDoubleValueSource)

public:
  const std::shared_ptr<DoubleValuesSource> source;
  const std::unordered_map<int, double> cache;

  CachingDoubleValueSource(std::shared_ptr<DoubleValuesSource> source);

  virtual std::wstring toString();

  std::shared_ptr<DoubleValues>
  getValues(std::shared_ptr<LeafReaderContext> readerContext,
            std::shared_ptr<DoubleValues> scores)  override;

private:
  class DoubleValuesAnonymousInnerClass : public DoubleValues
  {
    GET_CLASS_NAME(DoubleValuesAnonymousInnerClass)
  private:
    std::shared_ptr<CachingDoubleValueSource> outerInstance;

    int base = 0;
    std::shared_ptr<DoubleValues> vals;

  public:
    DoubleValuesAnonymousInnerClass(
        std::shared_ptr<CachingDoubleValueSource> outerInstance, int base,
        std::shared_ptr<DoubleValues> vals);

    double doubleValue()  override;

    bool advanceExact(int doc)  override;

    int doc = 0;

  protected:
    std::shared_ptr<DoubleValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleValuesAnonymousInnerClass>(
          org.apache.lucene.search.DoubleValues::shared_from_this());
    }
  };

public:
  bool needsScores() override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> ctx, int docId,
          std::shared_ptr<Explanation> scoreExplanation) 
      override;

  std::shared_ptr<DoubleValuesSource>
  rewrite(std::shared_ptr<IndexSearcher> searcher)  override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<CachingDoubleValueSource> shared_from_this()
  {
    return std::static_pointer_cast<CachingDoubleValueSource>(
        org.apache.lucene.search.DoubleValuesSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/util/
