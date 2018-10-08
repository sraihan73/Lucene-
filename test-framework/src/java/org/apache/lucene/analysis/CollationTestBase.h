#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::search
{
class Sort;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis
{

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Sort = org::apache::lucene::search::Sort;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base test class for testing Unicode collation.
 */
class CollationTestBase : public LuceneTestCase
{
  GET_CLASS_NAME(CollationTestBase)

protected:
  std::wstring firstRangeBeginningOriginal = L"\u062F";
  std::wstring firstRangeEndOriginal = L"\u0698";

  std::wstring secondRangeBeginningOriginal = L"\u0633";
  std::wstring secondRangeEndOriginal = L"\u0638";

public:
  virtual void testFarsiRangeFilterCollating(
      std::shared_ptr<Analyzer> analyzer, std::shared_ptr<BytesRef> firstBeg,
      std::shared_ptr<BytesRef> firstEnd, std::shared_ptr<BytesRef> secondBeg,
      std::shared_ptr<BytesRef> secondEnd) ;

  virtual void testFarsiRangeQueryCollating(
      std::shared_ptr<Analyzer> analyzer, std::shared_ptr<BytesRef> firstBeg,
      std::shared_ptr<BytesRef> firstEnd, std::shared_ptr<BytesRef> secondBeg,
      std::shared_ptr<BytesRef> secondEnd) ;

  virtual void testFarsiTermRangeQuery(
      std::shared_ptr<Analyzer> analyzer, std::shared_ptr<BytesRef> firstBeg,
      std::shared_ptr<BytesRef> firstEnd, std::shared_ptr<BytesRef> secondBeg,
      std::shared_ptr<BytesRef> secondEnd) ;

  // Make sure the documents returned by the search match the expected deque
  // Copied from TestSort.java
private:
  void assertMatches(std::shared_ptr<IndexSearcher> searcher,
                     std::shared_ptr<Query> query, std::shared_ptr<Sort> sort,
                     const std::wstring &expectedResult) ;

public:
  virtual void assertThreadSafe(std::shared_ptr<Analyzer> analyzer) throw(
      std::runtime_error);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<CollationTestBase> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer;
    std::unordered_map<std::wstring, std::shared_ptr<BytesRef>> map_obj;
    std::shared_ptr<org::apache::lucene::analysis::TokenStream> ts;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<CollationTestBase> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::Analyzer> analyzer,
        std::unordered_map<std::wstring, std::shared_ptr<BytesRef>> &map_obj,
        std::shared_ptr<org::apache::lucene::analysis::TokenStream> ts);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CollationTestBase> shared_from_this()
  {
    return std::static_pointer_cast<CollationTestBase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
