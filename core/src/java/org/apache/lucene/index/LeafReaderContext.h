#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>
#include "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include "core/src/java/org/apache/lucene/index/LeafReader.h"
#include "core/src/java/org/apache/lucene/index/CompositeReaderContext.h"

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
 * {@link IndexReaderContext} for {@link LeafReader} instances.
 */
class LeafReaderContext final : public IndexReaderContext
{
  GET_CLASS_NAME(LeafReaderContext)
  /** The readers ord in the top-level's leaves array */
public:
  const int ord;
  /** The readers absolute doc base */
  const int docBase;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<LeafReader> reader_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::deque<std::shared_ptr<LeafReaderContext>> leaves_;

  /**
   * Creates a new {@link LeafReaderContext}
   */
public:
  LeafReaderContext(std::shared_ptr<CompositeReaderContext> parent,
                    std::shared_ptr<LeafReader> reader, int ord, int docBase,
                    int leafOrd, int leafDocBase);

  LeafReaderContext(std::shared_ptr<LeafReader> leafReader);

  std::deque<std::shared_ptr<LeafReaderContext>> leaves() override;

  std::deque<std::shared_ptr<IndexReaderContext>> children() override;

  std::shared_ptr<IndexReader> reader() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<LeafReaderContext> shared_from_this()
  {
    return std::static_pointer_cast<LeafReaderContext>(
        IndexReaderContext::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
