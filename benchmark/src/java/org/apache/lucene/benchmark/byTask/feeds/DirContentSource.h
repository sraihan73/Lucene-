#pragma once
#include "ContentSource.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"
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

using Config = org::apache::lucene::benchmark::byTask::utils::Config;

/**
 * A {@link ContentSource} using the Dir collection for its input. Supports
 * the following configuration parameters (on top of {@link ContentSource}):
 * <ul>
 * <li><b>work.dir</b> - specifies the working directory. Required if "docs.dir"
 * denotes a relative path (<b>default=work</b>).
 * <li><b>docs.dir</b> - specifies the directory the Dir collection. Can be set
 * to a relative path if "work.dir" is also specified (<b>default=dir-out</b>).
 * </ul>
 */
class DirContentSource : public ContentSource
{
  GET_CLASS_NAME(DirContentSource)

private:
  class DateFormatInfo final
      : public std::enable_shared_from_this<DateFormatInfo>
  {
    GET_CLASS_NAME(DateFormatInfo)
  public:
    std::shared_ptr<DateFormat> df;
    std::shared_ptr<ParsePosition> pos;
  };

  /**
   * Iterator over the files in the directory
   */
public:
  class Iterator : public std::enable_shared_from_this<Iterator>,
                   public java::util::Iterator<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(Iterator)

  public:
    class Comparator : public std::enable_shared_from_this<Comparator>,
                       public java::util::Comparator<std::shared_ptr<Path>>
    {
      GET_CLASS_NAME(Comparator)
    public:
      int compare(std::shared_ptr<Path> _a, std::shared_ptr<Path> _b) override;
    };

  public:
    int count = 0;

    std::stack<std::shared_ptr<Path>> stack =
        std::stack<std::shared_ptr<Path>>();

    /* this seems silly ... there must be a better way ...
       not that this is good, but can it matter? */

    std::shared_ptr<Comparator> c = std::make_shared<Comparator>();

    Iterator(std::shared_ptr<Path> f) ;

    virtual void find() ;

    virtual void push(std::shared_ptr<Path> f) ;

    virtual void push(std::deque<std::shared_ptr<Path>> &files);

    virtual int getCount();

    bool hasNext() override;

    std::shared_ptr<Path> next() override;

    void remove() override;
  };

private:
  std::shared_ptr<ThreadLocal<std::shared_ptr<DateFormatInfo>>> dateFormat =
      std::make_shared<ThreadLocal<std::shared_ptr<DateFormatInfo>>>();
  std::shared_ptr<Path> dataDir = nullptr;
  int iteration = 0;
  std::shared_ptr<Iterator> inputFiles = nullptr;

  // get/initiate a thread-local simple date format (must do so
  // because SimpleDateFormat is not thread-safe).
  std::shared_ptr<DateFormatInfo> getDateFormatInfo();

  Date parseDate(const std::wstring &dateStr);

public:
  virtual ~DirContentSource();

  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void resetInputs()  override;

  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<DirContentSource> shared_from_this()
  {
    return std::static_pointer_cast<DirContentSource>(
        ContentSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
