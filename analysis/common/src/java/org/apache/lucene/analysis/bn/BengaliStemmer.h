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
//    import static org.apache.lucene.analysis.util.StemmerUtil.endsWith;

/**
 * Stemmer for Bengali.
 * <p>
 * The algorithm is based on the report in:
 * <i>Natural Language Processing in an Indian Language (Bengali)-I: Verb Phrase
 * Analysis</i> P Sengupta and B B Chaudhuri
 * </p>
 *
 * <p>
 *   Few Stemmer criteria are taken from:
 *   <i>http://members.unine.ch/jacques.savoy/clef/BengaliStemmerLight.java.txt</i>
 * </p>
 */
class BengaliStemmer : public std::enable_shared_from_this<BengaliStemmer>
{
  GET_CLASS_NAME(BengaliStemmer)
public:
  virtual int stem(std::deque<wchar_t> &buffer, int len);
};

} // namespace org::apache::lucene::analysis::bn
