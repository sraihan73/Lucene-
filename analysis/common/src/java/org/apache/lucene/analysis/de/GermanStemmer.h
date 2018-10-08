#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <string>
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
namespace org::apache::lucene::analysis::de
{

// This file is encoded in UTF-8

/**
 * A stemmer for German words.
 * <p>
 * The algorithm is based on the report
 * "A Fast and Simple Stemming Algorithm for German Words" by J&ouml;rg
 * Caumanns (joerg.caumanns at isst.fhg.de).
 * </p>
 */
class GermanStemmer : public std::enable_shared_from_this<GermanStemmer>
{
  GET_CLASS_NAME(GermanStemmer)
  /**
   * Buffer for the terms while stemming them.
   */
private:
  std::shared_ptr<StringBuilder> sb = std::make_shared<StringBuilder>();

  /**
   * Amount of characters that are removed with <tt>substitute()</tt> while
   * stemming.
   */
  int substCount = 0;

  static const std::shared_ptr<Locale> locale;

  /**
   * Stemms the given term to an unique <tt>discriminator</tt>.
   *
   * @param term  The term that should be stemmed.
   * @return      Discriminator for <tt>term</tt>
   */
protected:
  virtual std::wstring stem(const std::wstring &term);

  /**
   * Checks if a term could be stemmed.
   *
   * @return  true if, and only if, the given term consists in letters.
   */
private:
  bool isStemmable(const std::wstring &term);

  /**
   * suffix stripping (stemming) on the current term. The stripping is reduced
   * to the seven "base" suffixes "e", "s", "n", "t", "em", "er" and * "nd",
   * from which all regular suffixes are build of. The simplification causes
   * some overstemming, and way more irregular stems, but still provides unique.
   * discriminators in the most of those cases.
   * The algorithm is context free, except of the length restrictions.
   */
  void strip(std::shared_ptr<StringBuilder> buffer);

  /**
   * Does some optimizations on the term. This optimisations are
   * contextual.
   */
  void optimize(std::shared_ptr<StringBuilder> buffer);

  /**
   * Removes a particle denotion ("ge") from a term.
   */
  void removeParticleDenotion(std::shared_ptr<StringBuilder> buffer);

  /**
   * Do some substitutions for the term to reduce overstemming:
   *
   * - Substitute Umlauts with their corresponding vowel:{@code äöü -> aou},
   *   "ß" is substituted by "ss"
   * - Substitute a second char of a pair of equal characters with
   *   an asterisk: {@code ?? -> ?*}
   * - Substitute some common character combinations with a token:
   *   {@code sch/ch/ei/ie/ig/st -> $/§/%/&/#/!}
   */
  void substitute(std::shared_ptr<StringBuilder> buffer);

  /**
   * Undoes the changes made by substitute(). That are character pairs and
   * character combinations. Umlauts will remain as their corresponding vowel,
   * as "ß" remains as "ss".
   */
  void resubstitute(std::shared_ptr<StringBuilder> buffer);
};

} // #include  "core/src/java/org/apache/lucene/analysis/de/
