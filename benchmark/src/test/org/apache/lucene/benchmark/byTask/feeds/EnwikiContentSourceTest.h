#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/EnwikiContentSource.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/EnwikiContentSource.h"

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class EnwikiContentSourceTest : public LuceneTestCase
{
  GET_CLASS_NAME(EnwikiContentSourceTest)

  /** An EnwikiContentSource which works on a std::wstring and not files. */
private:
  class StringableEnwikiSource : public EnwikiContentSource
  {
    GET_CLASS_NAME(StringableEnwikiSource)

  private:
    const std::wstring docs;

  public:
    StringableEnwikiSource(const std::wstring &docs);

  protected:
    std::shared_ptr<InputStream> openInputStream()  override;

  protected:
    std::shared_ptr<StringableEnwikiSource> shared_from_this()
    {
      return std::static_pointer_cast<StringableEnwikiSource>(
          EnwikiContentSource::shared_from_this());
    }
  };

private:
  void assertDocData(std::shared_ptr<DocData> dd, const std::wstring &expName,
                     const std::wstring &expTitle, const std::wstring &expBody,
                     const std::wstring &expDate) ;

  void assertNoMoreDataException(
      std::shared_ptr<EnwikiContentSource> stdm) ;

  static const std::wstring PAGE1;

  static const std::wstring PAGE2;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOneDocument() throws Exception
  virtual void testOneDocument() ;

private:
  std::shared_ptr<EnwikiContentSource>
  createContentSource(const std::wstring &docs,
                      bool forever) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testTwoDocuments() throws Exception
  virtual void testTwoDocuments() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testForever() throws Exception
  virtual void testForever() ;

protected:
  std::shared_ptr<EnwikiContentSourceTest> shared_from_this()
  {
    return std::static_pointer_cast<EnwikiContentSourceTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
