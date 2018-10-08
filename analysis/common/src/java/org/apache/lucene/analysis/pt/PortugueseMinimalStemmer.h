#pragma once
#include "RSLPStemmerBase.h"
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
namespace org::apache::lucene::analysis::pt
{

/**
 * Minimal Stemmer for Portuguese
 * <p>
 * This follows the "RSLP-S" algorithm presented in:
 * <i>A study on the Use of Stemming for Monolingual Ad-Hoc Portuguese
 * Information Retrieval</i> (Orengo, et al)
 * which is just the plural reduction step of the RSLP
 * algorithm from <i>A Stemming Algorithm for the Portuguese Language</i>,
 * Orengo et al.
 * @see RSLPStemmerBase
 */
class PortugueseMinimalStemmer : public RSLPStemmerBase
{
  GET_CLASS_NAME(PortugueseMinimalStemmer)

private:
  static const std::shared_ptr<Step> pluralStep;

public:
  virtual int stem(std::deque<wchar_t> &s, int len);

protected:
  std::shared_ptr<PortugueseMinimalStemmer> shared_from_this()
  {
    return std::static_pointer_cast<PortugueseMinimalStemmer>(
        RSLPStemmerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::pt
