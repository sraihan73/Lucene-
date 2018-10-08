#pragma once
#include "../pt/RSLPStemmerBase.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/pt/RSLPStemmerBase.h"

#include  "core/src/java/org/apache/lucene/analysis/pt/Step.h"

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
namespace org::apache::lucene::analysis::gl
{

using RSLPStemmerBase = org::apache::lucene::analysis::pt::RSLPStemmerBase;

/**
 * Galician stemmer implementing "Regras do lematizador para o galego".
 *
 * @see RSLPStemmerBase
 * @see <a href="http://bvg.udc.es/recursos_lingua/stemming.jsp">Description of
 * rules</a>
 */
class GalicianStemmer : public RSLPStemmerBase
{
  GET_CLASS_NAME(GalicianStemmer)
private:
  static const std::shared_ptr<RSLPStemmerBase::Step> plural, unification,
      adverb, augmentative, noun, verb, vowel;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static GalicianStemmer::StaticConstructor staticConstructor;

  /**
   * @param s buffer, oversized to at least <code>len+1</code>
   * @param len initial valid length of buffer
   * @return new valid length, stemmed
   */
public:
  virtual int stem(std::deque<wchar_t> &s, int len);

protected:
  std::shared_ptr<GalicianStemmer> shared_from_this()
  {
    return std::static_pointer_cast<GalicianStemmer>(
        org.apache.lucene.analysis.pt.RSLPStemmerBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/gl/
