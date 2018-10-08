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
namespace org::apache::lucene::analysis::hi
{

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Normalizer for Hindi.
 * <p>
 * Normalizes text to remove some differences in spelling variations.
 * <p>
 * Implements the Hindi-language specific algorithm specified in:
 * <i>Word normalization in Indian languages</i>
 * Prasad Pingali and Vasudeva Varma.
 * http://web2py.iiit.ac.in/publications/default/download/inproceedings.pdf.3fe5b38c-02ee-41ce-9a8f-3e745670be32.pdf
 * <p>
 * with the following additions from <i>Hindi CLIR in Thirty Days</i>
 * Leah S. Larkey, Margaret E. Connell, and Nasreen AbdulJaleel.
 * http://maroo.cs.umass.edu/pub/web/getpdf.php?id=454:
 * <ul>
 *  <li>Internal Zero-width joiner and Zero-width non-joiners are removed
 *  <li>In addition to chandrabindu, NA+halant is normalized to anusvara
 * </ul>
 *
 */
class HindiNormalizer : public std::enable_shared_from_this<HindiNormalizer>
{
  GET_CLASS_NAME(HindiNormalizer)
  /**
   * Normalize an input buffer of Hindi text
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
public:
  virtual int normalize(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::hi
