#pragma once
#include "ContentSource.h"
#include "TrecDocParser.h"
#include "TrecGov2Parser.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/HTMLParser.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"

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

using ParsePathType =
    org::apache::lucene::benchmark::byTask::feeds::TrecDocParser::ParsePathType;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

/**
 * Implements a {@link ContentSource} over the TREC collection.
 * <p>
 * Supports the following configuration parameters (on top of
 * {@link ContentSource}):
 * <ul>
 * <li><b>work.dir</b> - specifies the working directory. Required if "docs.dir"
 * denotes a relative path (<b>default=work</b>).
 * <li><b>docs.dir</b> - specifies the directory where the TREC files reside.
 * Can be set to a relative path if "work.dir" is also specified
 * (<b>default=trec</b>).
 * <li><b>trec.doc.parser</b> - specifies the {@link TrecDocParser} class to use
 * for parsing the TREC documents content (<b>default=TrecGov2Parser</b>).
 * <li><b>html.parser</b> - specifies the {@link HTMLParser} class to use for
 * parsing the HTML parts of the TREC documents content
 * (<b>default=DemoHTMLParser</b>). <li><b>content.source.encoding</b> - if not
 * specified, ISO-8859-1 is used. <li><b>content.source.excludeIteration</b> -
 * if true, do not append iteration number to docname
 * </ul>
 */
class TrecContentSource : public ContentSource
{
  GET_CLASS_NAME(TrecContentSource)

public:
  class DateFormatInfo final
      : public std::enable_shared_from_this<DateFormatInfo>
  {
    GET_CLASS_NAME(DateFormatInfo)
  public:
    std::deque<std::shared_ptr<DateFormat>> dfs;
    std::shared_ptr<ParsePosition> pos;
  };

public:
  static const std::wstring DOCNO;
  static const std::wstring TERMINATING_DOCNO;
  static const std::wstring DOC;
  static const std::wstring TERMINATING_DOC;

  /** separator between lines in the byffer */
  static const std::wstring NEW_LINE;

private:
  static std::deque<std::wstring> const DATE_FORMATS;

  std::shared_ptr<ThreadLocal<std::shared_ptr<DateFormatInfo>>> dateFormats =
      std::make_shared<ThreadLocal<std::shared_ptr<DateFormatInfo>>>();
  std::shared_ptr<ThreadLocal<std::shared_ptr<StringBuilder>>> trecDocBuffer =
      std::make_shared<ThreadLocal<std::shared_ptr<StringBuilder>>>();
  std::shared_ptr<Path> dataDir = nullptr;
  std::deque<std::shared_ptr<Path>> inputFiles =
      std::deque<std::shared_ptr<Path>>();
  int nextFile = 0;
  // Use to synchronize threads on reading from the TREC documents.
  std::mutex lock;

  // Required for test
public:
  std::shared_ptr<BufferedReader> reader;
  int iteration = 0;
  std::shared_ptr<HTMLParser> htmlParser;

private:
  bool excludeDocnameIteration = false;
  std::shared_ptr<TrecDocParser> trecDocParser =
      std::make_shared<TrecGov2Parser>(); // default
public:
  ParsePathType currPathType =
      static_cast<ParsePathType>(0); // not private for tests

private:
  std::shared_ptr<DateFormatInfo> getDateFormatInfo();

  std::shared_ptr<StringBuilder> getDocBuffer();

public:
  virtual std::shared_ptr<HTMLParser> getHtmlParser();

  /**
   * Read until a line starting with the specified <code>lineStart</code>.
   * @param buf buffer for collecting the data if so specified/
   * @param lineStart line start to look for, must not be null.
   * @param collectMatchLine whether to collect the matching line into
   * <code>buffer</code>.
   * @param collectAll whether to collect all lines into <code>buffer</code>.
   * @throws IOException If there is a low-level I/O error.
   * @throws NoMoreDataException If the source is exhausted.
   */
private:
  void read(std::shared_ptr<StringBuilder> buf, const std::wstring &lineStart,
            bool collectMatchLine,
            bool collectAll) ;

public:
  virtual void openNextFile() ;

  virtual Date parseDate(const std::wstring &dateStr);

  virtual ~TrecContentSource();

  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

  void resetInputs()  override;

  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<TrecContentSource> shared_from_this()
  {
    return std::static_pointer_cast<TrecContentSource>(
        ContentSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
