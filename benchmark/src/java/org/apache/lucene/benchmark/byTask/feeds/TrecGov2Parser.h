#pragma once
#include "TrecDocParser.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/TrecContentSource.h"

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
 * Parser for the GOV2 collection format
 */
class TrecGov2Parser : public TrecDocParser
{
  GET_CLASS_NAME(TrecGov2Parser)

private:
  static const std::wstring DATE;
  static const std::wstring DATE_END;

  static const std::wstring DOCHDR;
  static const std::wstring TERMINATING_DOCHDR;

public:
  std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name,
        std::shared_ptr<TrecContentSource> trecSrc,
        std::shared_ptr<StringBuilder> docBuf,
        ParsePathType pathType)  override;

protected:
  std::shared_ptr<TrecGov2Parser> shared_from_this()
  {
    return std::static_pointer_cast<TrecGov2Parser>(
        TrecDocParser::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
