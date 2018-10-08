#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/CachedOrdinalsReader.h"

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

class TestCachedOrdinalsReader : public FacetTestCase
{
  GET_CLASS_NAME(TestCachedOrdinalsReader)

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testWithThreads() throws Exception
  virtual void testWithThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestCachedOrdinalsReader> outerInstance;

    std::shared_ptr<DirectoryReader> reader;
    std::shared_ptr<org::apache::lucene::facet::taxonomy::CachedOrdinalsReader>
        ordsReader;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestCachedOrdinalsReader> outerInstance,
        std::wstring L"CachedOrdsThread-" + i,
        std::shared_ptr<DirectoryReader> reader,
        std::shared_ptr<
            org::apache::lucene::facet::taxonomy::CachedOrdinalsReader>
            ordsReader);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestCachedOrdinalsReader> shared_from_this()
  {
    return std::static_pointer_cast<TestCachedOrdinalsReader>(
        org.apache.lucene.facet.FacetTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
