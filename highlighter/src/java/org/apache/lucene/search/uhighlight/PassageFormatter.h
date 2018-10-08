#pragma once
#include "stringhelper.h"
#include <any>
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
 *
 * @lucene.experimental
 */
class PassageFormatter : public std::enable_shared_from_this<PassageFormatter>
{
  GET_CLASS_NAME(PassageFormatter)

  /**
   * Formats the top <code>passages</code> from <code>content</code>
   * into a human-readable text snippet.
   *
   * @param passages top-N passages for the field. Note these are sorted in
   *                 the order that they appear in the document for convenience.
   * @param content  content for the field.
   * @return formatted highlight.  Note that for the
   * non-expert APIs in {@link UnifiedHighlighter} that
   * return std::wstring, the toString method on the Object
   * returned by this method is used to compute the string.
   */
public:
  virtual std::any format(std::deque<std::shared_ptr<Passage>> &passages,
                          const std::wstring &content) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
