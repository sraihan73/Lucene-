#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/uhighlight/Passage.h"

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
namespace org::apache::lucene::search::uhighlight
{

/**
 * Creates a formatted snippet from the top passages.
 * <p>
 * The default implementation marks the query terms as bold, and places
 * ellipses between unconnected passages.
 */
class DefaultPassageFormatter : public PassageFormatter
{
  GET_CLASS_NAME(DefaultPassageFormatter)
  /** text that will appear before highlighted terms */
protected:
  const std::wstring preTag;
  /** text that will appear after highlighted terms */
  const std::wstring postTag;
  /** text that will appear between two unconnected passages */
  const std::wstring ellipsis;
  /** true if we should escape for html */
  const bool escape;

  /**
   * Creates a new DefaultPassageFormatter with the default tags.
   */
public:
  DefaultPassageFormatter();

  /**
   * Creates a new DefaultPassageFormatter with custom tags.
   *
   * @param preTag   text which should appear before a highlighted term.
   * @param postTag  text which should appear after a highlighted term.
   * @param ellipsis text which should be used to connect two unconnected
   * passages.
   * @param escape   true if text should be html-escaped
   */
  DefaultPassageFormatter(const std::wstring &preTag,
                          const std::wstring &postTag,
                          const std::wstring &ellipsis, bool escape);

  std::wstring format(std::deque<std::shared_ptr<Passage>> &passages,
                      const std::wstring &content) override;

  /**
   * Appends original text to the response.
   *
   * @param dest    resulting text, possibly transformed or encoded
   * @param content original text content
   * @param start   index of the first character in content
   * @param end     index of the character following the last character in
   * content
   */
protected:
  virtual void append(std::shared_ptr<StringBuilder> dest,
                      const std::wstring &content, int start, int end);

protected:
  std::shared_ptr<DefaultPassageFormatter> shared_from_this()
  {
    return std::static_pointer_cast<DefaultPassageFormatter>(
        PassageFormatter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
