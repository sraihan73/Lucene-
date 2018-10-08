#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/TaxonomyWriterCache.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/FacetLabel.h"
#include  "core/src/java/org/apache/lucene/facet/FacetField.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyWriter.h"
#include  "core/src/java/org/apache/lucene/facet/FacetsConfig.h"

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
namespace org::apache::lucene::facet::taxonomy::directory
{

using FacetField = org::apache::lucene::facet::FacetField;
using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using TaxonomyWriterCache =
    org::apache::lucene::facet::taxonomy::writercache::TaxonomyWriterCache;

/** Tests concurrent indexing with facets. */
class TestConcurrentFacetedIndexing : public FacetTestCase
{
  GET_CLASS_NAME(TestConcurrentFacetedIndexing)

  // A No-Op TaxonomyWriterCache which always discards all given categories, and
  // always returns true in put(), to indicate some cache entries were cleared.
private:
  static std::shared_ptr<TaxonomyWriterCache> NO_OP_CACHE;

private:
  class TaxonomyWriterCacheAnonymousInnerClass
      : public std::enable_shared_from_this<
            TaxonomyWriterCacheAnonymousInnerClass>,
        public TaxonomyWriterCache
  {
    GET_CLASS_NAME(TaxonomyWriterCacheAnonymousInnerClass)
  public:
    TaxonomyWriterCacheAnonymousInnerClass();

    virtual ~TaxonomyWriterCacheAnonymousInnerClass();
    int get(std::shared_ptr<FacetLabel> categoryPath) override;
    bool put(std::shared_ptr<FacetLabel> categoryPath, int ordinal) override;
    bool isFull() override;
    void clear() override;
    int size() override;
  };

public:
  static std::shared_ptr<FacetField> newCategory();

  static std::shared_ptr<TaxonomyWriterCache> newTaxoWriterCache(int ndocs);

  virtual void testConcurrency() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestConcurrentFacetedIndexing> outerInstance;

    std::shared_ptr<AtomicInteger> numDocs;
    std::shared_ptr<ConcurrentHashMap<std::wstring, std::wstring>> values;
    std::shared_ptr<IndexWriter> iw;
    std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                        DirectoryTaxonomyWriter>
        tw;
    std::shared_ptr<FacetsConfig> config;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestConcurrentFacetedIndexing> outerInstance,
        std::shared_ptr<AtomicInteger> numDocs,
        std::shared_ptr<ConcurrentHashMap<std::wstring, std::wstring>> values,
        std::shared_ptr<IndexWriter> iw,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                            DirectoryTaxonomyWriter>
            tw,
        std::shared_ptr<FacetsConfig> config);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestConcurrentFacetedIndexing> shared_from_this()
  {
    return std::static_pointer_cast<TestConcurrentFacetedIndexing>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/
