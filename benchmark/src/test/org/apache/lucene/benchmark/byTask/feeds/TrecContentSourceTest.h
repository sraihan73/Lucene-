#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/TrecContentSource.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}

namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
}
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TrecContentSourceTest : public LuceneTestCase
{
  GET_CLASS_NAME(TrecContentSourceTest)

  /** A TrecDocMaker which works on a std::wstring and not files. */
private:
  class StringableTrecSource : public TrecContentSource
  {
    GET_CLASS_NAME(StringableTrecSource)

  private:
    std::wstring docs = L"";

  public:
    StringableTrecSource(const std::wstring &docs, bool forever);

    void openNextFile()  override;

    void setConfig(std::shared_ptr<Config> config) override;

  protected:
    std::shared_ptr<StringableTrecSource> shared_from_this()
    {
      return std::static_pointer_cast<StringableTrecSource>(
          TrecContentSource::shared_from_this());
    }
  };

private:
  void assertDocData(std::shared_ptr<DocData> dd, const std::wstring &expName,
                     const std::wstring &expTitle, const std::wstring &expBody,
                     Date expDate) ;

  void assertNoMoreDataException(
      std::shared_ptr<StringableTrecSource> stdm) ;

public:
  virtual void testOneDocument() ;

  virtual void testTwoDocuments() ;

  // If a Date: attribute is missing, make sure the document is not skipped, but
  // rather that null Data is assigned.
  virtual void testMissingDate() ;

  // When a 'bad date' is input (unparsable date), make sure the DocData date is
  // assigned null.
  virtual void testBadDate() ;

  virtual void testForever() ;

  /**
   * Open a trec content source over a directory with files of all trec path
   * types and all supported formats - bzip, gzip, txt.
   */
  virtual void testTrecFeedDirAllTypes() ;

protected:
  std::shared_ptr<TrecContentSourceTest> shared_from_this()
  {
    return std::static_pointer_cast<TrecContentSourceTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
