#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis
{

/** Utility class for doing vocabulary-based stemming tests */
class VocabularyAssert : public std::enable_shared_from_this<VocabularyAssert>
{
  /** Run a vocabulary test against two data files. */
public:
  static void
  assertVocabulary(std::shared_ptr<Analyzer> a,
                   std::shared_ptr<InputStream> voc,
                   std::shared_ptr<InputStream> out) ;

  /** Run a vocabulary test against one file: tab separated. */
  static void
  assertVocabulary(std::shared_ptr<Analyzer> a,
                   std::shared_ptr<InputStream> vocOut) ;

  /** Run a vocabulary test against two data files inside a zip file */
  static void assertVocabulary(std::shared_ptr<Analyzer> a,
                               std::shared_ptr<Path> zipFile,
                               const std::wstring &voc,
                               const std::wstring &out) ;

  /** Run a vocabulary test against a tab-separated data file inside a zip file
   */
  static void assertVocabulary(std::shared_ptr<Analyzer> a,
                               std::shared_ptr<Path> zipFile,
                               const std::wstring &vocOut) ;
};

} // namespace org::apache::lucene::analysis
