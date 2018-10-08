#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class InputIterator;
}

namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::search::spell
{

using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/**
 * Dictionary represented by a text file.
 *
 * <p>Format allowed: 1 word per line:<br>
 * word1<br>
 * word2<br>
 * word3<br>
 */
class PlainTextDictionary
    : public std::enable_shared_from_this<PlainTextDictionary>,
      public Dictionary
{
  GET_CLASS_NAME(PlainTextDictionary)

private:
  std::shared_ptr<BufferedReader> in_;

  /**
   * Creates a dictionary based on a Path.
   * <p>
   * NOTE: content is treated as UTF-8
   */
public:
  PlainTextDictionary(std::shared_ptr<Path> path) ;

  /**
   * Creates a dictionary based on an inputstream.
   * <p>
   * NOTE: content is treated as UTF-8
   */
  PlainTextDictionary(std::shared_ptr<InputStream> dictFile);

  /**
   * Creates a dictionary based on a reader.
   */
  PlainTextDictionary(std::shared_ptr<Reader> reader);

  std::shared_ptr<InputIterator> getEntryIterator()  override;

public:
  class FileIterator final : public std::enable_shared_from_this<FileIterator>,
                             public BytesRefIterator
  {
    GET_CLASS_NAME(FileIterator)
  private:
    std::shared_ptr<PlainTextDictionary> outerInstance;

  public:
    FileIterator(std::shared_ptr<PlainTextDictionary> outerInstance);

  private:
    bool done = false;
    const std::shared_ptr<BytesRefBuilder> spare =
        std::make_shared<BytesRefBuilder>();

  public:
    std::shared_ptr<BytesRef> next()  override;
  };
};

} // namespace org::apache::lucene::search::spell
