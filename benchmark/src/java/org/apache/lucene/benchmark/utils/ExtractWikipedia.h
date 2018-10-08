#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocMaker.h"

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

using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;

/**
 * Extract the downloaded Wikipedia dump into separate files for indexing.
 */
class ExtractWikipedia : public std::enable_shared_from_this<ExtractWikipedia>
{
  GET_CLASS_NAME(ExtractWikipedia)

private:
  std::shared_ptr<Path> outputDir;

public:
  static int count;

  static constexpr int BASE = 10;

protected:
  std::shared_ptr<DocMaker> docMaker;

public:
  ExtractWikipedia(std::shared_ptr<DocMaker> docMaker,
                   std::shared_ptr<Path> outputDir) ;

  virtual std::shared_ptr<Path> directory(int count,
                                          std::shared_ptr<Path> directory);

  virtual void create(const std::wstring &id, const std::wstring &title,
                      const std::wstring &time,
                      const std::wstring &body) ;

  virtual void extract() ;

  static void main(std::deque<std::wstring> &args) ;

private:
  static void printUsage();
};
} // #include  "core/src/java/org/apache/lucene/benchmark/utils/
