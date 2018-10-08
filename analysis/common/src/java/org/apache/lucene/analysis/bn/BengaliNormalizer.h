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
namespace org::apache::lucene::analysis::bn
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.delete;

/**
 * Normalizer for Bengali.
 * <p>
 * Implements the Bengali-language specific algorithm specified in:
 * <i>A Double Metaphone encoding for Bangla and its application in spelling
 * checker</i> Naushad UzZaman and Mumit Khan.
 * http://www.panl10n.net/english/final%20reports/pdf%20files/Bangladesh/BAN16.pdf
 * </p>
 */
class BengaliNormalizer : public std::enable_shared_from_this<BengaliNormalizer>
{
  GET_CLASS_NAME(BengaliNormalizer)
  /**
   * Normalize an input buffer of Bengali text
   *
   * @param s   input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
public:
  virtual int normalize(std::deque<wchar_t> &s, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/bn/
