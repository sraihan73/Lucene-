#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/SegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedNumericSelector.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedSetSelector.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class SegmentInfo;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::search
{
class SortedSetSelector;
}
namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::search
{
class SortedNumericSelector;
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
namespace org::apache::lucene::codecs::simpletext
{

using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * plain text segments file format.
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextSegmentInfoFormat : public SegmentInfoFormat
{
  GET_CLASS_NAME(SimpleTextSegmentInfoFormat)
public:
  static const std::shared_ptr<BytesRef> SI_VERSION;
  static const std::shared_ptr<BytesRef> SI_MIN_VERSION;
  static const std::shared_ptr<BytesRef> SI_DOCCOUNT;
  static const std::shared_ptr<BytesRef> SI_USECOMPOUND;
  static const std::shared_ptr<BytesRef> SI_NUM_DIAG;
  static const std::shared_ptr<BytesRef> SI_DIAG_KEY;
  static const std::shared_ptr<BytesRef> SI_DIAG_VALUE;
  static const std::shared_ptr<BytesRef> SI_NUM_ATT;
  static const std::shared_ptr<BytesRef> SI_ATT_KEY;
  static const std::shared_ptr<BytesRef> SI_ATT_VALUE;
  static const std::shared_ptr<BytesRef> SI_NUM_FILES;
  static const std::shared_ptr<BytesRef> SI_FILE;
  static const std::shared_ptr<BytesRef> SI_ID;
  static const std::shared_ptr<BytesRef> SI_SORT;
  static const std::shared_ptr<BytesRef> SI_SORT_FIELD;
  static const std::shared_ptr<BytesRef> SI_SORT_TYPE;
  static const std::shared_ptr<BytesRef> SI_SELECTOR_TYPE;
  static const std::shared_ptr<BytesRef> SI_SORT_REVERSE;
  static const std::shared_ptr<BytesRef> SI_SORT_MISSING;

  static const std::wstring SI_EXTENSION;

  std::shared_ptr<SegmentInfo>
  read(std::shared_ptr<Directory> directory, const std::wstring &segmentName,
       std::deque<char> &segmentID,
       std::shared_ptr<IOContext> context)  override;

private:
  std::wstring readString(int offset, std::shared_ptr<BytesRefBuilder> scratch);

  SortedSetSelector::Type
  readSetSelector(std::shared_ptr<IndexInput> input,
                  std::shared_ptr<BytesRefBuilder> scratch) ;

  SortedNumericSelector::Type readNumericSelector(
      std::shared_ptr<IndexInput> input,
      std::shared_ptr<BytesRefBuilder> scratch) ;

public:
  void write(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentInfo> si,
             std::shared_ptr<IOContext> ioContext)  override;

protected:
  std::shared_ptr<SimpleTextSegmentInfoFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextSegmentInfoFormat>(
        org.apache.lucene.codecs.SegmentInfoFormat::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::simpletext
