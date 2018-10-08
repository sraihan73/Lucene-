#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::analysis::fa
{

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Normalizer for Persian.
 * <p>
 * Normalization is done in-place for efficiency, operating on a termbuffer.
 * <p>
 * Normalization is defined as:
 * <ul>
 * <li>Normalization of various heh + hamza forms and heh goal to heh.
 * <li>Normalization of farsi yeh and yeh barree to arabic yeh
 * <li>Normalization of persian keheh to arabic kaf
 * </ul>
 *
 */
class PersianNormalizer : public std::enable_shared_from_this<PersianNormalizer>
{
  GET_CLASS_NAME(PersianNormalizer)
public:
  static constexpr wchar_t YEH = L'\u064A';

  static constexpr wchar_t FARSI_YEH = L'\u06CC';

  static constexpr wchar_t YEH_BARREE = L'\u06D2';

  static constexpr wchar_t KEHEH = L'\u06A9';

  static constexpr wchar_t KAF = L'\u0643';

  static constexpr wchar_t HAMZA_ABOVE = L'\u0654';

  static constexpr wchar_t HEH_YEH = L'\u06C0';

  static constexpr wchar_t HEH_GOAL = L'\u06C1';

  static constexpr wchar_t HEH = L'\u0647';

  /**
   * Normalize an input buffer of Persian text
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
  virtual int normalize(std::deque<wchar_t> &s, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/fa/
