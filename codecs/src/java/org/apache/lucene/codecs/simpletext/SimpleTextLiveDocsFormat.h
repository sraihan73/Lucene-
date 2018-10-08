#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/LiveDocsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Bits.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

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

using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

/**
 * reads/writes plaintext live docs
 * <p>
 * <b>FOR RECREATIONAL USE ONLY</b>
 * @lucene.experimental
 */
class SimpleTextLiveDocsFormat : public LiveDocsFormat
{
  GET_CLASS_NAME(SimpleTextLiveDocsFormat)

public:
  static const std::wstring LIVEDOCS_EXTENSION;

  static const std::shared_ptr<BytesRef> SIZE;
  static const std::shared_ptr<BytesRef> DOC;
  static const std::shared_ptr<BytesRef> END;

  std::shared_ptr<Bits>
  readLiveDocs(std::shared_ptr<Directory> dir,
               std::shared_ptr<SegmentCommitInfo> info,
               std::shared_ptr<IOContext> context)  override;

private:
  int parseIntAt(std::shared_ptr<BytesRef> bytes, int offset,
                 std::shared_ptr<CharsRefBuilder> scratch);

public:
  void
  writeLiveDocs(std::shared_ptr<Bits> bits, std::shared_ptr<Directory> dir,
                std::shared_ptr<SegmentCommitInfo> info, int newDelCount,
                std::shared_ptr<IOContext> context)  override;

  void files(std::shared_ptr<SegmentCommitInfo> info,
             std::shared_ptr<std::deque<std::wstring>> files) 
      override;

  // read-only
public:
  class SimpleTextBits : public std::enable_shared_from_this<SimpleTextBits>,
                         public Bits
  {
    GET_CLASS_NAME(SimpleTextBits)
  public:
    const std::shared_ptr<BitSet> bits;
    const int size;

    SimpleTextBits(std::shared_ptr<BitSet> bits, int size);

    bool get(int index) override;

    int length() override;
  };

protected:
  std::shared_ptr<SimpleTextLiveDocsFormat> shared_from_this()
  {
    return std::static_pointer_cast<SimpleTextLiveDocsFormat>(
        org.apache.lucene.codecs.LiveDocsFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/simpletext/
