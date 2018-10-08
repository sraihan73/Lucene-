#pragma once
#include "PerfTask.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/PerfRunData.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using Document = org::apache::lucene::document::Document;

/**
 * A task which writes documents, one line per document. Each line is in the
 * following format: title &lt;TAB&gt; date &lt;TAB&gt; body. The output of this
 * task can be consumed by
 * {@link org.apache.lucene.benchmark.byTask.feeds.LineDocSource} and is
 * intended to save the IO overhead of opening a file per document to be
 * indexed. <p> The format of the output is set according to the output file
 * extension. Compression is recommended when the output file is expected to be
 * large. See info on file extensions in
 * {@link org.apache.lucene.benchmark.byTask.utils.StreamUtils.Type}
 * <p>
 * Supports the following parameters:
 * <ul>
 * <li><b>line.file.out</b> - the name of the file to write the output to. That
 * parameter is mandatory. <b>NOTE:</b> the file is re-created.
 * <li><b>line.fields</b> - which fields should be written in each line.
 * (optional, default: {@link #DEFAULT_FIELDS}).
 * <li><b>sufficient.fields</b> - deque of field names, separated by comma,
 * which, if all of them are missing, the document will be skipped. For example,
 * to require that at least one of f1,f2 is not empty, specify: "f1,f2" in this
 * field. To specify that no field is required, i.e. that even empty docs should
 * be emitted, specify <b>","</b>. (optional, default: {@link
 * #DEFAULT_SUFFICIENT_FIELDS}).
 * </ul>
 * <b>NOTE:</b> this class is not thread-safe and if used by multiple threads
 * the output is unspecified (as all will write to the same output file in a
 * non-synchronized way).
 */
class WriteLineDocTask : public PerfTask
{
  GET_CLASS_NAME(WriteLineDocTask)

public:
  static const std::wstring FIELDS_HEADER_INDICATOR;

  static constexpr wchar_t SEP = L'\t';

  /**
   * Fields to be written by default
   */
  static std::deque<std::wstring> const DEFAULT_FIELDS;

  /**
   * Default fields which at least one of them is required to not skip the doc.
   */
  static const std::wstring DEFAULT_SUFFICIENT_FIELDS;

private:
  int docSize = 0;

protected:
  const std::wstring fname;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<PrintWriter> lineFileOut_;
  const std::shared_ptr<DocMaker> docMaker;
  const std::shared_ptr<ThreadLocal<std::shared_ptr<StringBuilder>>>
      threadBuffer =
          std::make_shared<ThreadLocal<std::shared_ptr<StringBuilder>>>();
  const std::shared_ptr<ThreadLocal<std::shared_ptr<Matcher>>>
      threadNormalizer =
          std::make_shared<ThreadLocal<std::shared_ptr<Matcher>>>();
  std::deque<std::wstring> const fieldsToWrite;
  std::deque<bool> const sufficientFields;
  const bool checkSufficientFields;

public:
  WriteLineDocTask(std::shared_ptr<PerfRunData> runData) throw(
      std::runtime_error);

  /**
   * Write header to the lines file - indicating how to read the file later.
   */
protected:
  virtual void writeHeader(std::shared_ptr<PrintWriter> out);

  std::wstring getLogMessage(int recsCount) override;

public:
  int doLogic()  override;

  /**
   * Selects output line file by written doc.
   * Default: original output line file.
   */
protected:
  virtual std::shared_ptr<PrintWriter>
  lineFileOut(std::shared_ptr<Document> doc);

public:
  virtual ~WriteLineDocTask();

  /**
   * Set the params (docSize only)
   * @param params docSize, or 0 for no limit.
   */
  void setParams(const std::wstring &params) override;

  bool supportsParams() override;

protected:
  std::shared_ptr<WriteLineDocTask> shared_from_this()
  {
    return std::static_pointer_cast<WriteLineDocTask>(
        PerfTask::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/tasks/
