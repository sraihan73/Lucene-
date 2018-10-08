#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/NearestNeighbor.h"

#include  "core/src/java/org/apache/lucene/document/NearestHit.h"
#include  "core/src/java/org/apache/lucene/index/IndexWriterConfig.h"

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
namespace org::apache::lucene::document
{

using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestNearest : public LuceneTestCase
{
  GET_CLASS_NAME(TestNearest)

public:
  virtual void testNearestNeighborWithDeletedDocs() ;

  virtual void
  testNearestNeighborWithAllDeletedDocs() ;

  virtual void testTieBreakByDocID() ;

  virtual void testNearestNeighborWithNoDocs() ;

private:
  double quantizeLat(double latRaw);

  double quantizeLon(double lonRaw);

public:
  virtual void testNearestNeighborRandom() ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<NearestHit>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestNearest> outerInstance;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<TestNearest> outerInstance);

    int compare(std::shared_ptr<NearestHit> a, std::shared_ptr<NearestHit> b);
  };

private:
  std::shared_ptr<IndexWriterConfig> getIndexWriterConfig();

protected:
  std::shared_ptr<TestNearest> shared_from_this()
  {
    return std::static_pointer_cast<TestNearest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
