#pragma once
#include "LineDocSource.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
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

/**
 * A line parser for Geonames.org data.
 * See <a href="http://download.geonames.org/export/dump/readme.txt">'geoname'
 * table</a>. Requires {@link SpatialDocMaker}.
 */
class GeonamesLineParser : public LineDocSource::LineParser
{
  GET_CLASS_NAME(GeonamesLineParser)

  /** This header will be ignored; the geonames format is fixed and doesn't have
   * a header line. */
public:
  GeonamesLineParser(std::deque<std::wstring> &header);

  void parseLine(std::shared_ptr<DocData> docData,
                 const std::wstring &line) override;

protected:
  std::shared_ptr<GeonamesLineParser> shared_from_this()
  {
    return std::static_pointer_cast<GeonamesLineParser>(
        LineDocSource.LineParser::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
