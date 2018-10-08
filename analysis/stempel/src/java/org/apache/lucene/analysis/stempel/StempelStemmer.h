#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::egothor::stemmer
{
class Trie;
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
namespace org::apache::lucene::analysis::stempel
{

using Trie = org::egothor::stemmer::Trie;

/**
 * <p>
 * Stemmer class is a convenient facade for other stemmer-related classes. The
 * core stemming algorithm and its implementation is taken verbatim from the
 * Egothor project ( <a href="http://www.egothor.org">www.egothor.org </a>).
 * </p>
 * <p>
 * Even though the stemmer tables supplied in the distribution package are built
 * for Polish language, there is nothing language-specific here.
 * </p>
 */
class StempelStemmer : public std::enable_shared_from_this<StempelStemmer>
{
  GET_CLASS_NAME(StempelStemmer)
private:
  std::shared_ptr<Trie> stemmer = nullptr;
  std::shared_ptr<StringBuilder> buffer = std::make_shared<StringBuilder>();

  /**
   * Create a Stemmer using selected stemmer table
   *
   * @param stemmerTable stemmer table.
   */
public:
  StempelStemmer(std::shared_ptr<InputStream> stemmerTable) ;

  /**
   * Create a Stemmer using pre-loaded stemmer table
   *
   * @param stemmer pre-loaded stemmer table
   */
  StempelStemmer(std::shared_ptr<Trie> stemmer);

  /**
   * Load a stemmer table from an inputstream.
   */
  static std::shared_ptr<Trie>
  load(std::shared_ptr<InputStream> stemmerTable) ;

  /**
   * Stem a word.
   *
   * @param word input word to be stemmed.
   * @return stemmed word, or null if the stem could not be generated.
   */
  virtual std::shared_ptr<StringBuilder>
  stem(std::shared_ptr<std::wstring> word);
};

} // namespace org::apache::lucene::analysis::stempel
