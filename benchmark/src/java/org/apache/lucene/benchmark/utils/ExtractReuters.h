#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
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
namespace org::apache::lucene::benchmark::utils
{

/**
 * Split the Reuters SGML documents into Simple Text files containing: Title,
 * Date, Dateline, Body
 */
class ExtractReuters : public std::enable_shared_from_this<ExtractReuters>
{
  GET_CLASS_NAME(ExtractReuters)
private:
  std::shared_ptr<Path> reutersDir;
  std::shared_ptr<Path> outputDir;

public:
  ExtractReuters(std::shared_ptr<Path> reutersDir,
                 std::shared_ptr<Path> outputDir) ;

  virtual void extract() ;

  std::shared_ptr<Pattern> EXTRACTION_PATTERN = Pattern::compile(
      L"<TITLE>(.*?)</TITLE>|<DATE>(.*?)</DATE>|<BODY>(.*?)</BODY>");

private:
  static std::deque<std::wstring> META_CHARS;

  static std::deque<std::wstring> META_CHARS_SERIALIZATIONS;

  /**
   * Override if you wish to change what is extracted
   */
protected:
  virtual void extractFile(std::shared_ptr<Path> sgmFile);

  static void main(std::deque<std::wstring> &args) ;

private:
  static void usage(const std::wstring &msg);
};

} // namespace org::apache::lucene::benchmark::utils
