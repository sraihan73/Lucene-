#pragma once
#include "../pt/RSLPStemmerBase.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::pt
{
class RSLPStemmerBase;
}

namespace org::apache::lucene::analysis::pt
{
class Step;
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
namespace org::apache::lucene::analysis::gl
{

using RSLPStemmerBase = org::apache::lucene::analysis::pt::RSLPStemmerBase;

/**
 * Minimal Stemmer for Galician
 * <p>
 * This follows the "RSLP-S" algorithm, but modified for Galician.
 * Hence this stemmer only applies the plural reduction step of:
 * "Regras do lematizador para o galego"
 * @see RSLPStemmerBase
 */
class GalicianMinimalStemmer : public RSLPStemmerBase
{
  GET_CLASS_NAME(GalicianMinimalStemmer)

private:
  static const std::shared_ptr<RSLPStemmerBase::Step> pluralStep;

public:
  virtual int stem(std::deque<wchar_t> &s, int len);

protected:
  std::shared_ptr<GalicianMinimalStemmer> shared_from_this()
  {
    return std::static_pointer_cast<GalicianMinimalStemmer>(
        org.apache.lucene.analysis.pt.RSLPStemmerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::gl
