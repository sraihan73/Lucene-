#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class TrecContentSource;
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

/**
 * HTML Parsing Interface for test purposes
 */
class HTMLParser
{
  GET_CLASS_NAME(HTMLParser)

  /**
   * Parse the input Reader and return DocData.
   * The provided name,title,date are used for the result, unless when they're
   * null, in which case an attempt is made to set them from the parsed data.
   * @param docData result reused
   * @param name name of the result doc data.
   * @param date date of the result doc data. If null, attempt to set by parsed
   * data.
   * @param reader reader of html text to parse.
   * @param trecSrc the {@link TrecContentSource} used to parse dates.
   * @return Parsed doc data.
   * @throws IOException If there is a low-level I/O error.
   */
public:
  virtual std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name, Date date,
        std::shared_ptr<Reader> reader,
        std::shared_ptr<TrecContentSource> trecSrc) = 0;
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
