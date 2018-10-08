#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/util/bkd/OfflinePointReader.h"
#include  "core/src/java/org/apache/lucene/util/bkd/PointReader.h"

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
namespace org::apache::lucene::util::bkd
{

using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/** Writes points to disk in a fixed-with format.
 *
 * @lucene.internal */
class OfflinePointWriter final
    : public std::enable_shared_from_this<OfflinePointWriter>,
      public PointWriter
{
  GET_CLASS_NAME(OfflinePointWriter)

public:
  const std::shared_ptr<Directory> tempDir;
  const std::shared_ptr<IndexOutput> out;
  const std::wstring name;
  const int packedBytesLength;
  const bool singleValuePerDoc;
  int64_t count = 0;

private:
  bool closed = false;
  // true if ords are written as long (8 bytes), else 4 bytes
  bool longOrds = false;
  std::shared_ptr<OfflinePointReader> sharedReader;
  int64_t nextSharedRead = 0;

public:
  const int64_t expectedCount;

  /** Create a new writer with an unknown number of incoming points */
  OfflinePointWriter(std::shared_ptr<Directory> tempDir,
                     const std::wstring &tempFileNamePrefix,
                     int packedBytesLength, bool longOrds,
                     const std::wstring &desc, int64_t expectedCount,
                     bool singleValuePerDoc) ;

  /** Initializes on an already written/closed file, just so consumers can use
   * {@link #getReader} to read the file. */
  OfflinePointWriter(std::shared_ptr<Directory> tempDir,
                     const std::wstring &name, int packedBytesLength,
                     int64_t count, bool longOrds, bool singleValuePerDoc);

  void append(std::deque<char> &packedValue, int64_t ord,
              int docID)  override;

  std::shared_ptr<PointReader>
  getReader(int64_t start, int64_t length)  override;

  std::shared_ptr<PointReader>
  getSharedReader(int64_t start, int64_t length,
                  std::deque<std::shared_ptr<Closeable>>
                      &toCloseHeroically)  override;

  virtual ~OfflinePointWriter();

  void destroy()  override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
