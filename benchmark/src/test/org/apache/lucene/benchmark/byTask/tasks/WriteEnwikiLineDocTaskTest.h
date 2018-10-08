#pragma once
#include "../../../../../../../java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"
#include "../../BenchmarkTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::benchmark::byTask
{
class PerfRunData;
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
namespace org::apache::lucene::benchmark::byTask::tasks
{

using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Document = org::apache::lucene::document::Document;

/** Tests the functionality of {@link WriteEnwikiLineDocTask}. */
class WriteEnwikiLineDocTaskTest : public BenchmarkTestCase
{
  GET_CLASS_NAME(WriteEnwikiLineDocTaskTest)

  // class has to be public so that Class.forName.newInstance() will work
  /** Interleaves category docs with regular docs */
public:
  class WriteLineCategoryDocMaker final : public DocMaker
  {
    GET_CLASS_NAME(WriteLineCategoryDocMaker)

  public:
    std::shared_ptr<AtomicInteger> flip = std::make_shared<AtomicInteger>(0);

    std::shared_ptr<Document> makeDocument()  override;

  protected:
    std::shared_ptr<WriteLineCategoryDocMaker> shared_from_this()
    {
      return std::static_pointer_cast<WriteLineCategoryDocMaker>(
          org.apache.lucene.benchmark.byTask.feeds
              .DocMaker::shared_from_this());
    }
  };

private:
  std::shared_ptr<PerfRunData>
  createPerfRunData(std::shared_ptr<Path> file,
                    const std::wstring &docMakerName) ;

  void doReadTest(std::shared_ptr<Path> file, const std::wstring &expTitle,
                  const std::wstring &expDate,
                  const std::wstring &expBody) ;

  void doReadTest(int n, std::shared_ptr<Path> file,
                  const std::wstring &expTitle, const std::wstring &expDate,
                  const std::wstring &expBody) ;

public:
  virtual void testCategoryLines() ;

protected:
  std::shared_ptr<WriteEnwikiLineDocTaskTest> shared_from_this()
  {
    return std::static_pointer_cast<WriteEnwikiLineDocTaskTest>(
        org.apache.lucene.benchmark.BenchmarkTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::tasks
