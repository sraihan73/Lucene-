#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <iostream>
#include <map_obj>
#include <memory>
#include <set>
#include <stdexcept>
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
namespace org::apache::lucene::analysis::standard
{

/**
 * Generates a file containing JFlex macros to accept valid ASCII TLDs
 * (top level domains), for inclusion in JFlex grammars that can accept
 * domain names.
 * <p>
 * The IANA Root Zone Database is queried via HTTP from URL cmdline arg #0, the
 * response is parsed, and the results are written out to a file containing
 * a JFlex macro that will accept all valid ASCII-only TLDs, including punycode
 * forms of internationalized TLDs (output file cmdline arg #1).
 */
class GenerateJflexTLDMacros
    : public std::enable_shared_from_this<GenerateJflexTLDMacros>
{
  GET_CLASS_NAME(GenerateJflexTLDMacros)

  static void main(std::deque<std::wstring> &args) ;

private:
  static const std::wstring NL;

  static const std::wstring APACHE_LICENSE;

  static const std::shared_ptr<Pattern> TLD_PATTERN_1;
  static const std::shared_ptr<Pattern> TLD_PATTERN_2;
  const std::shared_ptr<URL> tldFileURL;
  int64_t tldFileLastModified = -1LL;
  const std::shared_ptr<File> outputFile;
  const std::shared_ptr<SortedMap<std::wstring, bool>>
      processedTLDsLongestFirst = std::map_obj<std::wstring, bool>(
          Comparator::comparing(std::wstring::length)
              .reversed()
              .thenComparing(std::wstring::compareTo));
  const std::deque<SortedSet<std::wstring>> TLDsBySuffixLength =
      std::deque<SortedSet<std::wstring>>(); // deque position indicates suffix
                                              // length

public:
  GenerateJflexTLDMacros(
      const std::wstring &tldFileURL,
      const std::wstring &outputFile) ;

  /**
   * Downloads the IANA Root Zone Database, extracts the ASCII TLDs, then
   * writes a set of JFlex macros accepting any of them case-insensitively
   * out to the specified output file.
   *
   * @throws IOException if there is a problem either downloading the database
   *  or writing out the output file.
   */
  virtual void execute() ;

  /**
   * Downloads the IANA Root Zone Database.
   * @throws java.io.IOException if there is a problem downloading the database
   */
private:
  void getIANARootZoneDatabase() ;

  /**
   * Partition TLDs by whether they are prefixes of other TLDs and then by
   * suffix length. We only care about TLDs that are prefixes and are exactly
   * one character shorter than another TLD. See LUCENE-8278 and LUCENE-5391.
   */
  void partitionTLDprefixesBySuffixLength();

  /**
   * Writes a file containing a JFlex macro that will accept any of the given
   * TLDs case-insensitively.
   */
  void writeOutput() ;

  std::wstring getMacroName(int suffixLength);

  void writeTLDmacro(
      std::shared_ptr<Writer> writer, const std::wstring &macroName,
      std::shared_ptr<SortedSet<std::wstring>> TLDs) ;

  /**
   * Returns a regex that will accept the given ASCII TLD case-insensitively.
   *
   * @param ASCIITLD The ASCII TLD to generate a regex for
   * @return a regex that will accept the given ASCII TLD case-insensitively
   */
  std::wstring getCaseInsensitiveRegex(const std::wstring &ASCIITLD);
};

} // #include  "core/src/java/org/apache/lucene/analysis/standard/
