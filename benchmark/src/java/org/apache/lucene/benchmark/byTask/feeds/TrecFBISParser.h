#pragma once
#include "TrecDocParser.h"
#include "stringbuilder.h"
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
 * Parser for the FBIS docs in trec disks 4+5 collection format
 */
class TrecFBISParser : public TrecDocParser
{
  GET_CLASS_NAME(TrecFBISParser)

private:
  static const std::wstring HEADER;
  static const std::wstring HEADER_END;
  static const int HEADER_END_LENGTH = HEADER_END.length();

  static const std::wstring DATE1;
  static const std::wstring DATE1_END;

  static const std::wstring TI;
  static const std::wstring TI_END;

public:
  std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name,
        std::shared_ptr<TrecContentSource> trecSrc,
        std::shared_ptr<StringBuilder> docBuf,
        ParsePathType pathType)  override;

protected:
  std::shared_ptr<TrecFBISParser> shared_from_this()
  {
    return std::static_pointer_cast<TrecFBISParser>(
        TrecDocParser::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
