#pragma once
#include "ContentSource.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/Config.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/NoMoreDataException.h"

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
 * A {@link ContentSource} reading from the Reuters collection.
 * <p>
 * Config properties:
 * <ul>
 * <li><b>work.dir</b> - path to the root of docs and indexes dirs (default
 * <b>work</b>).
 * <li><b>docs.dir</b> - path to the docs dir (default <b>reuters-out</b>).
 * </ul>
 */
class ReutersContentSource : public ContentSource
{
  GET_CLASS_NAME(ReutersContentSource)

private:
  class DateFormatInfo final
      : public std::enable_shared_from_this<DateFormatInfo>
  {
    GET_CLASS_NAME(DateFormatInfo)
  public:
    std::shared_ptr<DateFormat> df;
    std::shared_ptr<ParsePosition> pos;
  };

private:
  std::shared_ptr<ThreadLocal<std::shared_ptr<DateFormatInfo>>> dateFormat =
      std::make_shared<ThreadLocal<std::shared_ptr<DateFormatInfo>>>();
  std::shared_ptr<Path> dataDir = nullptr;
  std::deque<std::shared_ptr<Path>> inputFiles =
      std::deque<std::shared_ptr<Path>>();
  int nextFile = 0;
  int iteration = 0;

public:
  void setConfig(std::shared_ptr<Config> config) override;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DateFormatInfo> getDateFormatInfo();

  Date parseDate(const std::wstring &dateStr);

public:
  virtual ~ReutersContentSource();

  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void resetInputs()  override;

protected:
  std::shared_ptr<ReutersContentSource> shared_from_this()
  {
    return std::static_pointer_cast<ReutersContentSource>(
        ContentSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
