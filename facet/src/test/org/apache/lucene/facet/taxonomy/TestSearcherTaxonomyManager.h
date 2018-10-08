#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/TaxonomyWriter.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/SearcherTaxonomyManager.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/SearcherAndTaxonomy.h"
namespace org::apache::lucene::search
{
template <typename G>
class ReferenceManager;
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
namespace org::apache::lucene::facet::taxonomy
{

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using SearcherAndTaxonomy = org::apache::lucene::facet::taxonomy::
    SearcherTaxonomyManager::SearcherAndTaxonomy;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;

class TestSearcherTaxonomyManager : public FacetTestCase
{
  GET_CLASS_NAME(TestSearcherTaxonomyManager)

private:
  class IndexerThread : public Thread
  {
    GET_CLASS_NAME(IndexerThread)

  private:
    std::shared_ptr<IndexWriter> w;
    std::shared_ptr<FacetsConfig> config;
    std::shared_ptr<TaxonomyWriter> tw;
    std::shared_ptr<ReferenceManager<std::shared_ptr<SearcherAndTaxonomy>>> mgr;
    int ordLimit = 0;
    std::shared_ptr<AtomicBoolean> stop;

  public:
    IndexerThread(
        std::shared_ptr<IndexWriter> w, std::shared_ptr<FacetsConfig> config,
        std::shared_ptr<TaxonomyWriter> tw,
        std::shared_ptr<ReferenceManager<std::shared_ptr<SearcherAndTaxonomy>>>
            mgr,
        int ordLimit, std::shared_ptr<AtomicBoolean> stop);

    void run() override;

  protected:
    std::shared_ptr<IndexerThread> shared_from_this()
    {
      return std::static_pointer_cast<IndexerThread>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testNRT() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestSearcherTaxonomyManager> outerInstance;

    std::shared_ptr<AtomicBoolean> stop;
    std::shared_ptr<
        org::apache::lucene::facet::taxonomy::SearcherTaxonomyManager>
        mgr;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestSearcherTaxonomyManager> outerInstance,
        std::shared_ptr<AtomicBoolean> stop,
        std::shared_ptr<
            org::apache::lucene::facet::taxonomy::SearcherTaxonomyManager>
            mgr);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testDirectory() ;

  virtual void testReplaceTaxonomyNRT() ;

  virtual void testReplaceTaxonomyDirectory() ;

  virtual void testExceptionDuringRefresh() ;

protected:
  std::shared_ptr<TestSearcherTaxonomyManager> shared_from_this()
  {
    return std::static_pointer_cast<TestSearcherTaxonomyManager>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
