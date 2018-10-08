#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/compressing/CompressionMode.h"

#include  "core/src/java/org/apache/lucene/codecs/compressing/Compressor.h"
#include  "core/src/java/org/apache/lucene/codecs/compressing/Decompressor.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"

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
namespace org::apache::lucene::codecs::compressing::dummy
{

using CompressingCodec =
    org::apache::lucene::codecs::compressing::CompressingCodec;

/** CompressionCodec that does not compress data, useful for testing. */
// In its own package to make sure the oal.codecs.compressing classes are
// visible enough to let people write their own CompressionMode
class DummyCompressingCodec : public CompressingCodec
{
  GET_CLASS_NAME(DummyCompressingCodec)

public:
  static const std::shared_ptr<CompressionMode> DUMMY;

private:
  class CompressionModeAnonymousInnerClass : public CompressionMode
  {
    GET_CLASS_NAME(CompressionModeAnonymousInnerClass)
  public:
    CompressionModeAnonymousInnerClass();

    std::shared_ptr<Compressor> newCompressor() override;

    std::shared_ptr<Decompressor> newDecompressor() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<CompressionModeAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CompressionModeAnonymousInnerClass>(
          org.apache.lucene.codecs.compressing
              .CompressionMode::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<Decompressor> DUMMY_DECOMPRESSOR;

private:
  class DecompressorAnonymousInnerClass : public Decompressor
  {
    GET_CLASS_NAME(DecompressorAnonymousInnerClass)
  public:
    DecompressorAnonymousInnerClass();

    void
    decompress(std::shared_ptr<DataInput> in_, int originalLength, int offset,
               int length,
               std::shared_ptr<BytesRef> bytes)  override;

    std::shared_ptr<Decompressor> clone() override;

  protected:
    std::shared_ptr<DecompressorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DecompressorAnonymousInnerClass>(
          org.apache.lucene.codecs.compressing
              .Decompressor::shared_from_this());
    }
  };

private:
  static const std::shared_ptr<Compressor> DUMMY_COMPRESSOR;

private:
  class CompressorAnonymousInnerClass : public Compressor
  {
    GET_CLASS_NAME(CompressorAnonymousInnerClass)
  public:
    CompressorAnonymousInnerClass();

    void compress(std::deque<char> &bytes, int off, int len,
                  std::shared_ptr<DataOutput> out)  override;

    virtual ~CompressorAnonymousInnerClass();

  protected:
    std::shared_ptr<CompressorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CompressorAnonymousInnerClass>(
          org.apache.lucene.codecs.compressing.Compressor::shared_from_this());
    }
  };

  /** Constructor that allows to configure the chunk size. */
public:
  DummyCompressingCodec(int chunkSize, int maxDocsPerChunk,
                        bool withSegmentSuffix, int blockSize);

  /** Default constructor. */
  DummyCompressingCodec();

protected:
  std::shared_ptr<DummyCompressingCodec> shared_from_this()
  {
    return std::static_pointer_cast<DummyCompressingCodec>(
        org.apache.lucene.codecs.compressing
            .CompressingCodec::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/compressing/dummy/
