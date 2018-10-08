#pragma once
#include "stringhelper.h"
#include <memory>

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
namespace org::apache::lucene::codecs
{

/**
 * Holder for per-term statistics.
 *
 * @see TermsEnum#docFreq
 * @see TermsEnum#totalTermFreq
 */
class TermStats : public std::enable_shared_from_this<TermStats>
{
  GET_CLASS_NAME(TermStats)
  /** How many documents have at least one occurrence of
   *  this term. */
public:
  const int docFreq;

  /** Total number of times this term occurs across all
   *  documents in the field. */
  const int64_t totalTermFreq;

  /** Sole constructor. */
  TermStats(int docFreq, int64_t totalTermFreq);
};

} // namespace org::apache::lucene::codecs
