#pragma once
#include "PerfTask.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"

#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/ReusableStringReader.h"

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

using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Document = org::apache::lucene::document::Document;

/**
 * Simple task to test performance of tokenizers.  It just
 * creates a token stream for each field of the document and
 * read all tokens out of that stream.
 */
class ReadTokensTask : public PerfTask
{
  GET_CLASS_NAME(ReadTokensTask)

public:
  ReadTokensTask(std::shared_ptr<PerfRunData> runData);

private:
  int totalTokenCount = 0;

  // volatile data passed between setup(), doLogic(), tearDown().
  std::shared_ptr<Document> doc;

public:
  void setup()  override;

protected:
  std::wstring getLogMessage(int recsCount) override;

public:
  void tearDown()  override;

  int doLogic()  override;

  /* Simple StringReader that can be reset to a new string;
   * we use this when tokenizing the string value from a
   * Field. */
  std::shared_ptr<ReusableStringReader> stringReader =
      std::make_shared<ReusableStringReader>();

private:
  class ReusableStringReader final : public Reader
  {
    GET_CLASS_NAME(ReusableStringReader)
  public:
    int upto = 0;
    int left = 0;
    std::wstring s;
    void init(const std::wstring &s);
    int read(std::deque<wchar_t> &c) override;
    int read(std::deque<wchar_t> &c, int off, int len) override;
    virtual ~ReusableStringReader();

  protected:
    std::shared_ptr<ReusableStringReader> shared_from_this()
    {
      return std::static_pointer_cast<ReusableStringReader>(
          java.io.Reader::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ReadTokensTask> shared_from_this()
  {
    return std::static_pointer_cast<ReadTokensTask>(
        PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
