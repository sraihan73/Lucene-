#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::index
{

/**
 * Signals that no index was found in the Directory. Possibly because the
 * directory is empty, however can also indicate an index corruption.
 */
class IndexNotFoundException final : public FileNotFoundException
{
  GET_CLASS_NAME(IndexNotFoundException)

  /** Creates IndexFileNotFoundException with the
   *  description message. */
public:
  IndexNotFoundException(const std::wstring &msg);

protected:
  std::shared_ptr<IndexNotFoundException> shared_from_this()
  {
    return std::static_pointer_cast<IndexNotFoundException>(
        java.io.FileNotFoundException::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
