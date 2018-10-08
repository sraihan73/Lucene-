#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
 * This class represents a hyphen. A 'full' hyphen is made of 3 parts: the
 * pre-break text, post-break text and no-break. If no line-break is generated
 * at this position, the no-break text is used, otherwise, pre-break and
 * post-break are used. Typically, pre-break is equal to the hyphen character
 * and the others are empty. However, this general scheme allows support for
 * cases in some languages where words change spelling if they're split across
 * lines, like german's 'backen' which hyphenates 'bak-ken'. BTW, this comes
 * from TeX.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */

class Hyphen : public std::enable_shared_from_this<Hyphen>
{
  GET_CLASS_NAME(Hyphen)
public:
  std::wstring preBreak;

  std::wstring noBreak;

  std::wstring postBreak;

  Hyphen(const std::wstring &pre, const std::wstring &no,
         const std::wstring &post);

  Hyphen(const std::wstring &pre);

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
