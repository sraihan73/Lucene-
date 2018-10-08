#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::no
{

/*
 * This algorithm is updated based on code located at:
 * http://members.unine.ch/jacques.savoy/clef/
 *
 * Full copyright for that code follows:
 */

/*
 * Copyright (c) 2005, Jacques Savoy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * deque of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this deque of conditions and
 * the following disclaimer in the documentation and/or other materials
 * provided with the distribution. Neither the name of the author nor the names
 * of its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.BOKMAAL;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.no.NorwegianLightStemmer.NYNORSK;

/**
 * Minimal Stemmer for Norwegian Bokmål (no-nb) and Nynorsk (no-nn)
 * <p>
 * Stems known plural forms for Norwegian nouns only, together with genitiv -s
 */
class NorwegianMinimalStemmer
    : public std::enable_shared_from_this<NorwegianMinimalStemmer>
{
  GET_CLASS_NAME(NorwegianMinimalStemmer)
public:
  const bool useBokmaal;
  const bool useNynorsk;

  /**
   * Creates a new NorwegianMinimalStemmer
   * @param flags set to {@link NorwegianLightStemmer#BOKMAAL},
   *                     {@link NorwegianLightStemmer#NYNORSK}, or both.
   */
  NorwegianMinimalStemmer(int flags);

  virtual int stem(std::deque<wchar_t> &s, int len);
};

} // namespace org::apache::lucene::analysis::no
