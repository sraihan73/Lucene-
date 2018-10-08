#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::search::suggest
{
class InputIterator;
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

using IndexReader = org::apache::lucene::index::IndexReader;
using InputIterator = org::apache::lucene::search::suggest::InputIterator;

/**
 * Lucene Dictionary: terms taken from the given field
 * of a Lucene index.
 */
class LuceneDictionary : public std::enable_shared_from_this<LuceneDictionary>,
                         public Dictionary
{
  GET_CLASS_NAME(LuceneDictionary)
private:
  std::shared_ptr<IndexReader> reader;
  std::wstring field;

  /**
   * Creates a new Dictionary, pulling source terms from
   * the specified <code>field</code> in the provided <code>reader</code>
   */
public:
  LuceneDictionary(std::shared_ptr<IndexReader> reader,
                   const std::wstring &field);

  std::shared_ptr<InputIterator> getEntryIterator()  override;
};

} // namespace org::apache::lucene::search::spell
