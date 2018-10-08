#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Collector.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/search/TopDocs.h"

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

using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestTopDocsMerge : public LuceneTestCase
{
  GET_CLASS_NAME(TestTopDocsMerge)

private:
  class ShardSearcher : public IndexSearcher
  {
    GET_CLASS_NAME(ShardSearcher)
  private:
    const std::deque<std::shared_ptr<LeafReaderContext>> ctx;

  public:
    ShardSearcher(std::shared_ptr<LeafReaderContext> ctx,
                  std::shared_ptr<IndexReaderContext> parent);

    virtual void
    search(std::shared_ptr<Weight> weight,
           std::shared_ptr<Collector> collector) ;

    virtual std::shared_ptr<TopDocs> search(std::shared_ptr<Weight> weight,
                                            int topN) ;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<ShardSearcher> shared_from_this()
    {
      return std::static_pointer_cast<ShardSearcher>(
          IndexSearcher::shared_from_this());
    }
  };

public:
  virtual void testSort_1() ;

  virtual void testSort_2() ;

  virtual void testInconsistentTopDocsFail();

  virtual void testPreAssignedShardIndex();

  virtual void testSort(bool useFrom) ;

protected:
  std::shared_ptr<TestTopDocsMerge> shared_from_this()
  {
    return std::static_pointer_cast<TestTopDocsMerge>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
