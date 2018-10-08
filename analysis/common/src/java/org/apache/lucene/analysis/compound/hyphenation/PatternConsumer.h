#pragma once
#include "stringhelper.h"
#include <any>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::compound::hyphenation
{

/**
 * This interface is used to connect the XML pattern file parser to the
 * hyphenation tree.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class PatternConsumer
{
  GET_CLASS_NAME(PatternConsumer)

  /**
   * Add a character class. A character class defines characters that are
   * considered equivalent for the purpose of hyphenation (e.g. "aA"). It
   * usually means to ignore case.
   *
   * @param chargroup character group
   */
public:
  virtual void addClass(const std::wstring &chargroup) = 0;

  /**
   * Add a hyphenation exception. An exception replaces the result obtained by
   * the algorithm for cases for which this fails or the user wants to provide
   * his own hyphenation. A hyphenatedword is a deque of alternating std::wstring's
   * and {@link Hyphen Hyphen} instances
   */
  virtual void addException(const std::wstring &word,
                            std::deque<std::any> &hyphenatedword) = 0;

  /**
   * Add hyphenation patterns.
   *
   * @param pattern the pattern
   * @param values interletter values expressed as a string of digit characters.
   */
  virtual void addPattern(const std::wstring &pattern,
                          const std::wstring &values) = 0;
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
