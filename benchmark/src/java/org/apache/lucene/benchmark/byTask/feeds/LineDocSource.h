#pragma once
#include "ContentSource.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
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

/**
 * A {@link ContentSource} reading one line at a time as a
 * {@link org.apache.lucene.document.Document} from a single file. This saves IO
 * cost (over DirContentSource) of recursing through a directory and opening a
 * new file for every document.<br>
 * The expected format of each line is (arguments are separated by &lt;TAB&gt;):
 * <i>title, date, body</i>. If a line is read in a different format, a
 * {@link RuntimeException} will be thrown. In general, you should use this
 * content source for files that were created with {@link WriteLineDocTask}.<br>
 * <br>
 * Config properties:
 * <ul>
 * <li>docs.file=&lt;path to the file&gt;
 * <li>content.source.encoding - default to UTF-8.
 * <li>line.parser - default to {@link HeaderLineParser} if a header line exists
 * which differs from {@link WriteLineDocTask#DEFAULT_FIELDS} and to {@link
 * SimpleLineParser} otherwise.
 * </ul>
 */
class LineDocSource : public ContentSource
{
  GET_CLASS_NAME(LineDocSource)

  /** Reader of a single input line into {@link DocData}. */
public:
  class LineParser : public std::enable_shared_from_this<LineParser>
  {
    GET_CLASS_NAME(LineParser)
  protected:
    std::deque<std::wstring> const header;
    /** Construct with the header
     * @param header header line found in the input file, or null if none
     */
  public:
    LineParser(std::deque<std::wstring> &header);
    /** parse an input line and fill doc data appropriately */
    virtual void parseLine(std::shared_ptr<DocData> docData,
                           const std::wstring &line) = 0;
  };

  /**
   * {@link LineParser} which ignores the header passed to its constructor
   * and assumes simply that field names and their order are the same
   * as in {@link WriteLineDocTask#DEFAULT_FIELDS}
   */
public:
  class SimpleLineParser : public LineParser
  {
    GET_CLASS_NAME(SimpleLineParser)
  public:
    SimpleLineParser(std::deque<std::wstring> &header);
    void parseLine(std::shared_ptr<DocData> docData,
                   const std::wstring &line) override;

  protected:
    std::shared_ptr<SimpleLineParser> shared_from_this()
    {
      return std::static_pointer_cast<SimpleLineParser>(
          LineParser::shared_from_this());
    }
  };

  /**
   * {@link LineParser} which sets field names and order by
   * the header - any header - of the lines file.
   * It is less efficient than {@link SimpleLineParser} but more powerful.
   */
public:
  class HeaderLineParser : public LineParser
  {
    GET_CLASS_NAME(HeaderLineParser)
  private:
    enum class FieldName {
      GET_CLASS_NAME(FieldName) NAME,
      TITLE,
      DATE,
      BODY,
      PROP
    };

  private:
    std::deque<FieldName> const posToF;

  public:
    HeaderLineParser(std::deque<std::wstring> &header);

    void parseLine(std::shared_ptr<DocData> docData,
                   const std::wstring &line) override;

  private:
    void setDocDataField(std::shared_ptr<DocData> docData, int position,
                         const std::wstring &text);

  protected:
    std::shared_ptr<HeaderLineParser> shared_from_this()
    {
      return std::static_pointer_cast<HeaderLineParser>(
          LineParser::shared_from_this());
    }
  };

private:
  std::shared_ptr<Path> file;
  std::shared_ptr<BufferedReader> reader;
  int readCount = 0;

  std::shared_ptr<LineParser> docDataLineReader = nullptr;
  bool skipHeaderLine = false;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void openFile();

public:
  virtual ~LineDocSource();

  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

private:
  std::shared_ptr<LineParser> createDocDataLineReader(const std::wstring &line);

public:
  void resetInputs()  override;

  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<LineDocSource> shared_from_this()
  {
    return std::static_pointer_cast<LineDocSource>(
        ContentSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
