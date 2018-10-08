#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
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
namespace org::apache::lucene::replicator
{

using IndexInput = org::apache::lucene::store::IndexInput;

/**
 * An {@link InputStream} which wraps an {@link IndexInput}.
 *
 * @lucene.experimental
 */
class IndexInputInputStream final : public InputStream
{
  GET_CLASS_NAME(IndexInputInputStream)

private:
  const std::shared_ptr<IndexInput> in_;

  int64_t remaining = 0;

public:
  IndexInputInputStream(std::shared_ptr<IndexInput> in_);

  int read()  override;

  int available()  override;

  virtual ~IndexInputInputStream();

  int read(std::deque<char> &b)  override;

  int read(std::deque<char> &b, int off, int len)  override;

  int64_t skip(int64_t n)  override;

protected:
  std::shared_ptr<IndexInputInputStream> shared_from_this()
  {
    return std::static_pointer_cast<IndexInputInputStream>(
        java.io.InputStream::shared_from_this());
  }
};
} // namespace org::apache::lucene::replicator
