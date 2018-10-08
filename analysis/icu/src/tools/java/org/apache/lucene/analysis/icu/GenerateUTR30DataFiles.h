#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <regex>
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
namespace org::apache::lucene::analysis::icu
{

/**
 * Downloads/generates lucene/analysis/icu/src/data/utr30/*.txt
 *
 * ASSUMPTION: This class will be run with current directory set to
 * lucene/analysis/icu/src/data/utr30/
 *
 * <ol>
 *   <li>
 *     Downloads nfc.txt, nfkc.txt and nfkc_cf.txt from icu-project.org,
 *     overwriting the versions in lucene/analysis/icu/src/data/utr30/.
 *   </li>
 *   <li>
 *     Converts round-trip mappings in nfc.txt (containing '=')
 *     that map_obj to at least one [:Diacritic:] character
 *     into one-way mappings ('&gt;' instead of '=').
 *   </li>
 * </ol>
 */
class GenerateUTR30DataFiles
    : public std::enable_shared_from_this<GenerateUTR30DataFiles>
{
  GET_CLASS_NAME(GenerateUTR30DataFiles)
private:
  static const std::wstring ICU_SVN_TAG_URL;
  static const std::wstring ICU_RELEASE_TAG;
  static const std::wstring ICU_DATA_NORM2_PATH;
  static const std::wstring NFC_TXT;
  static const std::wstring NFKC_TXT;
  static const std::wstring NFKC_CF_TXT;
  static std::deque<char> DOWNLOAD_BUFFER;
  static const std::shared_ptr<Pattern> ROUND_TRIP_MAPPING_LINE_PATTERN;
  static const std::shared_ptr<Pattern> VERBATIM_RULE_LINE_PATTERN;
  static const std::shared_ptr<Pattern> RULE_LINE_PATTERN;
  static const std::shared_ptr<Pattern> BLANK_OR_COMMENT_LINE_PATTERN;
  static const std::shared_ptr<Pattern> NUMERIC_VALUE_PATTERN;

  static void main(std::deque<std::wstring> &args);

  static void expandRulesInUTR30DataFiles() ;

private:
  class FileFilterAnonymousInnerClass : public FileFilter
  {
    GET_CLASS_NAME(FileFilterAnonymousInnerClass)
  public:
    FileFilterAnonymousInnerClass();

    bool accept(std::shared_ptr<File> pathname) override;

  protected:
    std::shared_ptr<FileFilterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FileFilterAnonymousInnerClass>(
          java.io.FileFilter::shared_from_this());
    }
  };

private:
  static void
  expandDataFileRules(std::shared_ptr<File> file) ;

  static void getNFKCDataFilesFromIcuProject() ;

  static void download(std::shared_ptr<URL> url,
                       const std::wstring &outputFile) ;

  static std::shared_ptr<URLConnection>
  openConnection(std::shared_ptr<URL> url) ;

  static void expandSingleRule(
      std::shared_ptr<StringBuilder> builder, const std::wstring &leftHandSide,
      const std::wstring &rightHandSide) ;
};

} // namespace org::apache::lucene::analysis::icu
