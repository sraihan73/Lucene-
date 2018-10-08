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
namespace org::apache::lucene::analysis::ckb
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.endsWith;

/**
 * Light stemmer for Sorani
 */
class SoraniStemmer : public std::enable_shared_from_this<SoraniStemmer>
{
  GET_CLASS_NAME(SoraniStemmer)

  /**
   * Stem an input buffer of Sorani text.
   *
   * @param s input buffer
   * @param len length of input buffer
   * @return length of input buffer after normalization
   */
public:
  virtual int stem(std::deque<wchar_t> &s, int len);
};

} // #include  "core/src/java/org/apache/lucene/analysis/ckb/
