#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/document/Field.h"

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
namespace org::apache::lucene::util
{

using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;

/** Minimal port of benchmark's LneDocSource +
 * DocMaker, so tests can enum docs from a line file created
 * by benchmark's WriteLineDoc task */
class LineFileDocs : public std::enable_shared_from_this<LineFileDocs>
{
  GET_CLASS_NAME(LineFileDocs)

private:
  std::shared_ptr<BufferedReader> reader;
  static const int BUFFER_SIZE = 1 << 16; // 64K
  const std::shared_ptr<AtomicInteger> id = std::make_shared<AtomicInteger>();
  const std::wstring path;
  const std::shared_ptr<Random> random;

  /** If forever is true, we rewind the file at EOF (repeat
   * the docs over and over) */
public:
  LineFileDocs(std::shared_ptr<Random> random,
               const std::wstring &path) ;

  LineFileDocs(std::shared_ptr<Random> random) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~LineFileDocs();

private:
  int64_t randomSeekPos(std::shared_ptr<Random> random, int64_t size);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void open(std::shared_ptr<Random> random) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void reset(std::shared_ptr<Random> random) ;

private:
  static constexpr wchar_t SEP = L'\t';

private:
  class DocState final : public std::enable_shared_from_this<DocState>
  {
    GET_CLASS_NAME(DocState)
  public:
    const std::shared_ptr<Document> doc;
    const std::shared_ptr<Field> titleTokenized;
    const std::shared_ptr<Field> title;
    const std::shared_ptr<Field> titleDV;
    const std::shared_ptr<Field> body;
    const std::shared_ptr<Field> id;
    const std::shared_ptr<Field> idNum;
    const std::shared_ptr<Field> idNumDV;
    const std::shared_ptr<Field> date;

    DocState();
  };

private:
  const std::shared_ptr<CloseableThreadLocal<std::shared_ptr<DocState>>>
      threadDocs =
          std::make_shared<CloseableThreadLocal<std::shared_ptr<DocState>>>();

  /** Note: Document instance is re-used per-thread */
public:
  virtual std::shared_ptr<Document> nextDoc() ;
};

} // #include  "core/src/java/org/apache/lucene/util/
