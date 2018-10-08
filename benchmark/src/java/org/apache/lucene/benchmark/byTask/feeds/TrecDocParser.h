#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

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
 * Parser for trec doc content, invoked on doc text excluding &lt;DOC&gt; and
 * &lt;DOCNO&gt; which are handled in TrecContentSource. Required to be
 * stateless and hence thread safe.
 */
class TrecDocParser : public std::enable_shared_from_this<TrecDocParser>
{
  GET_CLASS_NAME(TrecDocParser)

  /** Types of trec parse paths, */
public:
  enum class ParsePathType {
    GET_CLASS_NAME(ParsePathType) GOV2,
    FBIS,
    FT,
    FR94,
    LATIMES
  };

  /** trec parser type used for unknown extensions */
public:
  static constexpr ParsePathType DEFAULT_PATH_TYPE = ParsePathType::GOV2;

  static const std::unordered_map<ParsePathType, std::shared_ptr<TrecDocParser>>
      pathType2parser;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TrecDocParser::StaticConstructor staticConstructor;

public:
  static const std::unordered_map<std::wstring, ParsePathType> pathName2Type;

  /** max length of walk up from file to its ancestors when looking for a known
   * path type */
private:
  static constexpr int MAX_PATH_LENGTH = 10;

  /**
   * Compute the path type of a file by inspecting name of file and its parents
   */
public:
  static ParsePathType pathType(std::shared_ptr<Path> f);

  /**
   * parse the text prepared in docBuf into a result DocData,
   * no synchronization is required.
   * @param docData reusable result
   * @param name name that should be set to the result
   * @param trecSrc calling trec content source
   * @param docBuf text to parse
   * @param pathType type of parsed file, or null if unknown - may be used by
   * parsers to alter their behavior according to the file path type.
   */
  virtual std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name,
        std::shared_ptr<TrecContentSource> trecSrc,
        std::shared_ptr<StringBuilder> docBuf, ParsePathType pathType) = 0;

  /**
   * strip tags from <code>buf</code>: each tag is replaced by a single blank.
   * @return text obtained when stripping all tags from <code>buf</code> (Input
   * StringBuilder is unmodified).
   */
  static std::wstring stripTags(std::shared_ptr<StringBuilder> buf, int start);

  /**
   * strip tags from input.
   * @see #stripTags(StringBuilder, int)
   */
  static std::wstring stripTags(const std::wstring &buf, int start);

  /**
   * Extract from <code>buf</code> the text of interest within specified tags
   * @param buf entire input text
   * @param startTag tag marking start of text of interest
   * @param endTag tag marking end of text of interest
   * @param maxPos if &ge; 0 sets a limit on start of text of interest
   * @return text of interest or null if not found
   */
  static std::wstring extract(std::shared_ptr<StringBuilder> buf,
                              const std::wstring &startTag,
                              const std::wstring &endTag, int maxPos,
                              std::deque<std::wstring> &noisePrefixes);

  // public static void main(std::wstring[] args) {
  //  System.out.println(stripTags("is it true that<space>2<<second
  //  space>><almost last space>1<one more space>?",0));
  //}
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
