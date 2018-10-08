#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
class LongBitSet;
}
namespace org::apache::lucene::util::bkd
{
class PointWriter;
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
namespace org::apache::lucene::util::bkd
{

using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;
using LongBitSet = org::apache::lucene::util::LongBitSet;

/** Reads points from disk in a fixed-with format, previously written with
 * {@link OfflinePointWriter}.
 *
 * @lucene.internal */
class OfflinePointReader final : public PointReader
{
  GET_CLASS_NAME(OfflinePointReader)
public:
  int64_t countLeft = 0;
  const std::shared_ptr<IndexInput> in_;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  std::deque<char> const packedValue_;

public:
  const bool singleValuePerDoc;
  const int bytesPerDoc;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t ord_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docID_ = 0;
  // true if ords are written as long (8 bytes), else 4 bytes
  bool longOrds = false;
  bool checked = false;

  // File name we are reading
public:
  const std::wstring name;

  OfflinePointReader(std::shared_ptr<Directory> tempDir,
                     const std::wstring &tempFileName, int packedBytesLength,
                     int64_t start, int64_t length, bool longOrds,
                     bool singleValuePerDoc) ;

  bool next()  override;

  std::deque<char> packedValue() override;

  int64_t ord() override;

  int docID() override;

  virtual ~OfflinePointReader();

  void
  markOrds(int64_t count,
           std::shared_ptr<LongBitSet> ordBitSet)  override;

  int64_t split(int64_t count, std::shared_ptr<LongBitSet> rightTree,
                  std::shared_ptr<PointWriter> left,
                  std::shared_ptr<PointWriter> right,
                  bool doClearBits)  override;

  // Poached from ByteArrayDataInput:
private:
  static int64_t readLong(std::deque<char> &bytes, int pos);

  // Poached from ByteArrayDataInput:
  static int readInt(std::deque<char> &bytes, int pos);

protected:
  std::shared_ptr<OfflinePointReader> shared_from_this()
  {
    return std::static_pointer_cast<OfflinePointReader>(
        PointReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::util::bkd
