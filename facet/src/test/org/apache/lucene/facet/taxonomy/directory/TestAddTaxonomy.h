#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/DirectoryTaxonomyWriter.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/OrdinalMap.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

using FacetTestCase = org::apache::lucene::facet::FacetTestCase;
using OrdinalMap = org::apache::lucene::facet::taxonomy::directory::
    DirectoryTaxonomyWriter::OrdinalMap;
using Directory = org::apache::lucene::store::Directory;

class TestAddTaxonomy : public FacetTestCase
{
  GET_CLASS_NAME(TestAddTaxonomy)

private:
  void dotest(int ncats, int const range) ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestAddTaxonomy> outerInstance;

    int range = 0;
    std::shared_ptr<AtomicInteger> numCats;
    std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                        DirectoryTaxonomyWriter>
        tw;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestAddTaxonomy> outerInstance, int range,
        std::shared_ptr<AtomicInteger> numCats,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                            DirectoryTaxonomyWriter>
            tw);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  std::shared_ptr<OrdinalMap> randomOrdinalMap() ;

  void validate(std::shared_ptr<Directory> dest, std::shared_ptr<Directory> src,
                std::shared_ptr<OrdinalMap> ordMap) ;

public:
  virtual void testAddEmpty() ;

  virtual void testAddToEmpty() ;

  // A more comprehensive and big random test.
  virtual void testBig() ;

  // a reasonable random test
  virtual void testMedium() ;

  virtual void testSimple() ;

  virtual void testConcurrency() ;

private:
  class ThreadAnonymousInnerClass2 : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass2)
  private:
    std::shared_ptr<TestAddTaxonomy> outerInstance;

    int numCategories = 0;
    std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                        DirectoryTaxonomyWriter>
        destTW;

  public:
    ThreadAnonymousInnerClass2(
        std::shared_ptr<TestAddTaxonomy> outerInstance, int numCategories,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::directory::
                            DirectoryTaxonomyWriter>
            destTW);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestAddTaxonomy> shared_from_this()
  {
    return std::static_pointer_cast<TestAddTaxonomy>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/directory/
