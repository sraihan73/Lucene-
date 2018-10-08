#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::compressing
{
class CompressingStoredFieldsFormat;
}

namespace org::apache::lucene::codecs::compressing
{
class CompressingTermVectorsFormat;
}
namespace org::apache::lucene::codecs::compressing
{
class CompressionMode;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
}
namespace org::apache::lucene::codecs
{
class TermVectorsFormat;
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
namespace org::apache::lucene::codecs::compressing
{

using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;

/**
 * A codec that uses {@link CompressingStoredFieldsFormat} for its stored
 * fields and delegates to the default codec for everything else.
 */
class CompressingCodec : public FilterCodec
{
  GET_CLASS_NAME(CompressingCodec)

  /**
   * Create a random instance.
   */
public:
  static std::shared_ptr<CompressingCodec>
  randomInstance(std::shared_ptr<Random> random, int chunkSize,
                 int maxDocsPerChunk, bool withSegmentSuffix, int blockSize);

  /**
   * Creates a random {@link CompressingCodec} that is using an empty segment
   * suffix
   */
  static std::shared_ptr<CompressingCodec>
  randomInstance(std::shared_ptr<Random> random);

  /**
   * Creates a random {@link CompressingCodec} with more reasonable parameters
   * for big tests.
   */
  static std::shared_ptr<CompressingCodec>
  reasonableInstance(std::shared_ptr<Random> random);

  /**
   * Creates a random {@link CompressingCodec} that is using a segment suffix
   */
  static std::shared_ptr<CompressingCodec>
  randomInstance(std::shared_ptr<Random> random, bool withSegmentSuffix);

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<CompressingStoredFieldsFormat> storedFieldsFormat_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<CompressingTermVectorsFormat> termVectorsFormat_;

  /**
   * Creates a compressing codec with a given segment suffix
   */
public:
  CompressingCodec(const std::wstring &name, const std::wstring &segmentSuffix,
                   std::shared_ptr<CompressionMode> compressionMode,
                   int chunkSize, int maxDocsPerChunk, int blockSize);

  /**
   * Creates a compressing codec with an empty segment suffix
   */
  CompressingCodec(const std::wstring &name,
                   std::shared_ptr<CompressionMode> compressionMode,
                   int chunkSize, int maxDocsPerChunk, int blockSize);

  std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() override;

  std::shared_ptr<TermVectorsFormat> termVectorsFormat() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressingCodec> shared_from_this()
  {
    return std::static_pointer_cast<CompressingCodec>(
        org.apache.lucene.codecs.FilterCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::compressing
