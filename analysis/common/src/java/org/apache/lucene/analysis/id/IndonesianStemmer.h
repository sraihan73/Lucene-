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
namespace org::apache::lucene::analysis::id
{

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Stemmer for Indonesian.
 * <p>
 * Stems Indonesian words with the algorithm presented in:
 * <i>A Study of Stemming Effects on Information Retrieval in
 * Bahasa Indonesia</i>, Fadillah Z Tala.
 * http://www.illc.uva.nl/Publications/ResearchReports/MoL-2003-02.text.pdf
 */
class IndonesianStemmer : public std::enable_shared_from_this<IndonesianStemmer>
{
  GET_CLASS_NAME(IndonesianStemmer)
private:
  int numSyllables = 0;
  int flags = 0;
  static constexpr int REMOVED_KE = 1;
  static constexpr int REMOVED_PENG = 2;
  static constexpr int REMOVED_DI = 4;
  static constexpr int REMOVED_MENG = 8;
  static constexpr int REMOVED_TER = 16;
  static constexpr int REMOVED_BER = 32;
  static constexpr int REMOVED_PE = 64;

  /**
   * Stem a term (returning its new length).
   * <p>
   * Use <code>stemDerivational</code> to control whether full stemming
   * or only light inflectional stemming is done.
   */
public:
  virtual int stem(std::deque<wchar_t> &text, int length,
                   bool stemDerivational);

private:
  int stemDerivational(std::deque<wchar_t> &text, int length);

  bool isVowel(wchar_t ch);

  int removeParticle(std::deque<wchar_t> &text, int length);

  int removePossessivePronoun(std::deque<wchar_t> &text, int length);

  int removeFirstOrderPrefix(std::deque<wchar_t> &text, int length);

  int removeSecondOrderPrefix(std::deque<wchar_t> &text, int length);

  int removeSuffix(std::deque<wchar_t> &text, int length);
};

} // #include  "core/src/java/org/apache/lucene/analysis/id/
