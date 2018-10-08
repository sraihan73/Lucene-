#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/Facets.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsCollector.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyReader.h"
#include  "core/src/java/org/apache/lucene/facet/FacetResult.h"
#include  "core/src/java/org/apache/lucene/facet/LabelAndValue.h"

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
namespace org::apache::lucene::facet
{

using TaxonomyReader = org::apache::lucene::facet::taxonomy::TaxonomyReader;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class FacetTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(FacetTestCase)

public:
  virtual std::shared_ptr<Facets>
  getTaxonomyFacetCounts(std::shared_ptr<TaxonomyReader> taxoReader,
                         std::shared_ptr<FacetsConfig> config,
                         std::shared_ptr<FacetsCollector> c) ;

  virtual std::shared_ptr<Facets>
  getTaxonomyFacetCounts(std::shared_ptr<TaxonomyReader> taxoReader,
                         std::shared_ptr<FacetsConfig> config,
                         std::shared_ptr<FacetsCollector> c,
                         const std::wstring &indexFieldName) ;

protected:
  virtual std::deque<std::wstring> getRandomTokens(int count);

  virtual std::wstring pickToken(std::deque<std::wstring> &tokens);

protected:
  class TestDoc : public std::enable_shared_from_this<TestDoc>
  {
    GET_CLASS_NAME(TestDoc)
  public:
    std::wstring content;
    std::deque<std::wstring> dims;
    float value = 0;
  };

protected:
  virtual std::deque<std::shared_ptr<TestDoc>>
  getRandomDocs(std::deque<std::wstring> &tokens, int count, int numDims);

  virtual void sortTies(std::deque<std::shared_ptr<FacetResult>> &results);

  virtual void
  sortTies(std::deque<std::shared_ptr<LabelAndValue>> &labelValues);

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LabelAndValue>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<FacetTestCase> outerInstance;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<FacetTestCase> outerInstance);

    int compare(std::shared_ptr<LabelAndValue> a,
                std::shared_ptr<LabelAndValue> b);
  };

protected:
  virtual void
  sortLabelValues(std::deque<std::shared_ptr<LabelAndValue>> &labelValues);

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<std::shared_ptr<LabelAndValue>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<FacetTestCase> outerInstance;

  public:
    ComparatorAnonymousInnerClass2(
        std::shared_ptr<FacetTestCase> outerInstance);

    int compare(std::shared_ptr<LabelAndValue> a,
                std::shared_ptr<LabelAndValue> b);
  };

protected:
  virtual void
  sortFacetResults(std::deque<std::shared_ptr<FacetResult>> &results);

private:
  class ComparatorAnonymousInnerClass3
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass3>,
        public Comparator<std::shared_ptr<FacetResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass3)
  private:
    std::shared_ptr<FacetTestCase> outerInstance;

  public:
    ComparatorAnonymousInnerClass3(
        std::shared_ptr<FacetTestCase> outerInstance);

    int compare(std::shared_ptr<FacetResult> a, std::shared_ptr<FacetResult> b);
  };

protected:
  virtual void
  assertFloatValuesEquals(std::deque<std::shared_ptr<FacetResult>> &a,
                          std::deque<std::shared_ptr<FacetResult>> &b);

  virtual void assertFloatValuesEquals(std::shared_ptr<FacetResult> a,
                                       std::shared_ptr<FacetResult> b);

protected:
  std::shared_ptr<FacetTestCase> shared_from_this()
  {
    return std::static_pointer_cast<FacetTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/
