#pragma once
#include "Compressor.h"
#include "Decompressor.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs::compressing
{
class LZ4;
}

namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::codecs::compressing
{
class Decompressor;
}
namespace org::apache::lucene::codecs::compressing
{
class HashTable;
}
namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::codecs::compressing
{
class HCHashTable;
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

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A compression mode. Tells how much effort should be spent on compression and
 * decompression of stored fields.
 * @lucene.experimental
 */
class CompressionMode : public std::enable_shared_from_this<CompressionMode>
{
  GET_CLASS_NAME(CompressionMode)

  /**
   * A compression mode that trades compression ratio for speed. Although the
   * compression ratio might remain high, compression and decompression are
   * very fast. Use this mode with indices that have a high update rate but
   * should be able to load documents from disk quickly.
   */
public:
  static const std::shared_ptr<CompressionMode> FAST;

private:
  class CompressionModeAnonymousInnerClass;

  /**
   * A compression mode that trades speed for compression ratio. Although
   * compression and decompression might be slow, this compression mode should
   * provide a good compression ratio. This mode might be interesting if/when
   * your index size is much bigger than your OS cache.
   */
public:
  static const std::shared_ptr<CompressionMode> HIGH_COMPRESSION;

private:
  class CompressionModeAnonymousInnerClass2;

  /**
   * This compression mode is similar to {@link #FAST} but it spends more time
   * compressing in order to improve the compression ratio. This compression
   * mode is best used with indices that have a low update rate but should be
   * able to load documents from disk quickly.
   */
public:
  static const std::shared_ptr<CompressionMode> FAST_DECOMPRESSION;

private:
  class CompressionModeAnonymousInnerClass3;

  /** Sole constructor. */
protected:
  CompressionMode();

  /**
   * Create a new {@link Compressor} instance.
   */
public:
  virtual std::shared_ptr<Compressor> newCompressor() = 0;

  /**
   * Create a new {@link Decompressor} instance.
   */
  virtual std::shared_ptr<Decompressor> newDecompressor() = 0;

private:
  static const std::shared_ptr<Decompressor> LZ4_DECOMPRESSOR;

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
          Decompressor::shared_from_this());
    }
  };

private:
  class LZ4FastCompressor final : public Compressor
  {
    GET_CLASS_NAME(LZ4FastCompressor)

  private:
    const std::shared_ptr<LZ4::HashTable> ht;

  public:
    LZ4FastCompressor();

    void compress(std::deque<char> &bytes, int off, int len,
                  std::shared_ptr<DataOutput> out)  override;

    virtual ~LZ4FastCompressor();

  protected:
    std::shared_ptr<LZ4FastCompressor> shared_from_this()
    {
      return std::static_pointer_cast<LZ4FastCompressor>(
          Compressor::shared_from_this());
    }
  };

private:
  class LZ4HighCompressor final : public Compressor
  {
    GET_CLASS_NAME(LZ4HighCompressor)

  private:
    const std::shared_ptr<LZ4::HCHashTable> ht;

  public:
    LZ4HighCompressor();

    void compress(std::deque<char> &bytes, int off, int len,
                  std::shared_ptr<DataOutput> out)  override;

    virtual ~LZ4HighCompressor();

  protected:
    std::shared_ptr<LZ4HighCompressor> shared_from_this()
    {
      return std::static_pointer_cast<LZ4HighCompressor>(
          Compressor::shared_from_this());
    }
  };

private:
  class DeflateDecompressor final : public Decompressor
  {
    GET_CLASS_NAME(DeflateDecompressor)

  public:
    std::deque<char> compressed;

    DeflateDecompressor();

    void
    decompress(std::shared_ptr<DataInput> in_, int originalLength, int offset,
               int length,
               std::shared_ptr<BytesRef> bytes)  override;

    std::shared_ptr<Decompressor> clone() override;

  protected:
    std::shared_ptr<DeflateDecompressor> shared_from_this()
    {
      return std::static_pointer_cast<DeflateDecompressor>(
          Decompressor::shared_from_this());
    }
  };

private:
  class DeflateCompressor : public Compressor
  {
    GET_CLASS_NAME(DeflateCompressor)

  public:
    const std::shared_ptr<Deflater> compressor;
    std::deque<char> compressed;
    bool closed = false;

    DeflateCompressor(int level);

    void compress(std::deque<char> &bytes, int off, int len,
                  std::shared_ptr<DataOutput> out)  override;

    virtual ~DeflateCompressor();

  protected:
    std::shared_ptr<DeflateCompressor> shared_from_this()
    {
      return std::static_pointer_cast<DeflateCompressor>(
          Compressor::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::codecs::compressing
class CompressionMode::CompressionModeAnonymousInnerClass
    : public CompressionMode
{
  GET_CLASS_NAME(CompressionMode::CompressionModeAnonymousInnerClass)
public:
  CompressionModeAnonymousInnerClass();

  std::shared_ptr<Compressor> newCompressor() override;

  std::shared_ptr<Decompressor> newDecompressor() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressionModeAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<CompressionModeAnonymousInnerClass>(
        CompressionMode::shared_from_this());
  }
};
class CompressionMode::CompressionModeAnonymousInnerClass2
    : public CompressionMode
{
  GET_CLASS_NAME(CompressionMode::CompressionModeAnonymousInnerClass2)
public:
  CompressionModeAnonymousInnerClass2();

  std::shared_ptr<Compressor> newCompressor() override;

  std::shared_ptr<Decompressor> newDecompressor() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressionModeAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<CompressionModeAnonymousInnerClass2>(
        CompressionMode::shared_from_this());
  }
};
class CompressionMode::CompressionModeAnonymousInnerClass3
    : public CompressionMode
{
  GET_CLASS_NAME(CompressionMode::CompressionModeAnonymousInnerClass3)
public:
  CompressionModeAnonymousInnerClass3();

  std::shared_ptr<Compressor> newCompressor() override;

  std::shared_ptr<Decompressor> newDecompressor() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<CompressionModeAnonymousInnerClass3> shared_from_this()
  {
    return std::static_pointer_cast<CompressionModeAnonymousInnerClass3>(
        CompressionMode::shared_from_this());
  }
};
