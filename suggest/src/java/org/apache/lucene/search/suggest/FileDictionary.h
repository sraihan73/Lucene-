#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::search::suggest
{

using Dictionary = org::apache::lucene::search::spell::Dictionary;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Dictionary represented by a text file.
 *
 * <p>Format allowed: 1 entry per line:<br>
 * An entry can be: <br>
 * <ul>
 * <li>suggestion</li>
 * <li>suggestion <code>fieldDelimiter</code> weight</li>
 * <li>suggestion <code>fieldDelimiter</code> weight <code>fieldDelimiter</code>
 * payload</li>
 * </ul>
 * where the default <code>fieldDelimiter</code> is {@value
 * #DEFAULT_FIELD_DELIMITER}<br> <p> <b>NOTE:</b> <ul> <li>In order to have
 * payload enabled, the first entry has to have a payload</li> <li>If the weight
 * for an entry is not specified then a value of 1 is used</li> <li>A payload
 * cannot be specified without having the weight specified for an entry</li>
 * <li>If the payload for an entry is not specified (assuming payload is
 * enabled) then an empty payload is returned</li> <li>An entry cannot have more
 * than two <code>fieldDelimiter</code></li>
 * </ul>
 * <p>
 * <b>Example:</b><br>
 * word1 word2 TAB 100 TAB payload1<br>
 * word3 TAB 101<br>
 * word4 word3 TAB 102<br>
 */
class FileDictionary : public std::enable_shared_from_this<FileDictionary>,
                       public Dictionary
{
  GET_CLASS_NAME(FileDictionary)

  /**
   * Tab-delimited fields are most common thus the default, but one can override
   * this via the constructor
   */
public:
  static const std::wstring DEFAULT_FIELD_DELIMITER;

private:
  std::shared_ptr<BufferedReader> in_;
  std::wstring line;
  bool done = false;
  const std::wstring fieldDelimiter;

  /**
   * Creates a dictionary based on an inputstream.
   * Using {@link #DEFAULT_FIELD_DELIMITER} as the
   * field separator in a line.
   * <p>
   * NOTE: content is treated as UTF-8
   */
public:
  FileDictionary(std::shared_ptr<InputStream> dictFile);

  /**
   * Creates a dictionary based on a reader.
   * Using {@link #DEFAULT_FIELD_DELIMITER} as the
   * field separator in a line.
   */
  FileDictionary(std::shared_ptr<Reader> reader);

  /**
   * Creates a dictionary based on a reader.
   * Using <code>fieldDelimiter</code> to separate out the
   * fields in a line.
   */
  FileDictionary(std::shared_ptr<Reader> reader,
                 const std::wstring &fieldDelimiter);

  /**
   * Creates a dictionary based on an inputstream.
   * Using <code>fieldDelimiter</code> to separate out the
   * fields in a line.
   * <p>
   * NOTE: content is treated as UTF-8
   */
  FileDictionary(std::shared_ptr<InputStream> dictFile,
                 const std::wstring &fieldDelimiter);

  std::shared_ptr<InputIterator> getEntryIterator() override;

public:
  class FileIterator final : public std::enable_shared_from_this<FileIterator>,
                             public InputIterator
  {
    GET_CLASS_NAME(FileIterator)
  private:
    std::shared_ptr<FileDictionary> outerInstance;

    int64_t curWeight = 0;
    const std::shared_ptr<BytesRefBuilder> spare =
        std::make_shared<BytesRefBuilder>();
    std::shared_ptr<BytesRefBuilder> curPayload =
        std::make_shared<BytesRefBuilder>();
    bool isFirstLine = true;
    // C++ NOTE: Fields cannot have the same name as methods:
    bool hasPayloads_ = false;

    FileIterator(std::shared_ptr<FileDictionary> outerInstance) throw(
        IOException);

  public:
    int64_t weight() override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

  private:
    void readWeight(const std::wstring &weight);

  public:
    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };
};

} // namespace org::apache::lucene::search::suggest
