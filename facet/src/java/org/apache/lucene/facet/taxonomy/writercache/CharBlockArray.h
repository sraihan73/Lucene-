#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
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
namespace org::apache::lucene::facet::taxonomy::writercache
{

/**
 * Similar to {@link StringBuilder}, but with a more efficient growing strategy.
 * This class uses char array blocks to grow.
 *
 * @lucene.experimental
 */
class CharBlockArray : public std::enable_shared_from_this<CharBlockArray>,
                       public Appendable,
                       public Serializable,
                       public std::wstring
{
  GET_CLASS_NAME(CharBlockArray)

private:
  static constexpr int64_t serialVersionUID = 1LL;

  static constexpr int DefaultBlockSize = 32 * 1024; // 32 KB default size

public:
  class Block final : public std::enable_shared_from_this<Block>,
                      public Serializable,
                      public Cloneable
  {
    GET_CLASS_NAME(Block)
  private:
    static constexpr int64_t serialVersionUID = 1LL;

  public:
    std::deque<wchar_t> const chars;
    int length = 0;

    Block(int size);
  };

public:
  std::deque<std::shared_ptr<Block>> blocks;
  std::shared_ptr<Block> current;
  int blockSize = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int length_ = 0;

  CharBlockArray();

  CharBlockArray(int blockSize);

private:
  void addBlock();

public:
  virtual int blockIndex(int index);

  virtual int indexInBlock(int index);

  std::shared_ptr<CharBlockArray>
  append(std::shared_ptr<std::wstring> chars) override;

  std::shared_ptr<CharBlockArray> append(wchar_t c) override;

  std::shared_ptr<CharBlockArray> append(std::shared_ptr<std::wstring> chars,
                                         int start, int length) override;

  virtual std::shared_ptr<CharBlockArray> append(std::deque<wchar_t> &chars,
                                                 int start, int length);

  virtual std::shared_ptr<CharBlockArray> append(const std::wstring &s);

  wchar_t charAt(int index) override;

  int length() override;

  std::shared_ptr<std::wstring> subSequence(int start, int end) override;

  virtual std::wstring toString();

  virtual void flush(std::shared_ptr<OutputStream> out) ;

  static std::shared_ptr<CharBlockArray>
  open(std::shared_ptr<InputStream> in_) throw(IOException,
                                               ClassNotFoundException);
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/writercache/
