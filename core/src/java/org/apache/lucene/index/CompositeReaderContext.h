#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/CompositeReader.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

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
 * {@link IndexReaderContext} for {@link CompositeReader} instance.
 */
class CompositeReaderContext final : public IndexReaderContext
{
  GET_CLASS_NAME(CompositeReaderContext)
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::deque<std::shared_ptr<IndexReaderContext>> children_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::deque<std::shared_ptr<LeafReaderContext>> leaves_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<CompositeReader> reader_;

  static std::shared_ptr<CompositeReaderContext>
  create(std::shared_ptr<CompositeReader> reader);

  /**
   * Creates a {@link CompositeReaderContext} for intermediate readers that
   * aren't not top-level readers in the current context
   */
public:
  CompositeReaderContext(
      std::shared_ptr<CompositeReaderContext> parent,
      std::shared_ptr<CompositeReader> reader, int ordInParent,
      int docbaseInParent,
      std::deque<std::shared_ptr<IndexReaderContext>> &children);

  /**
   * Creates a {@link CompositeReaderContext} for top-level readers with parent
   * set to <code>null</code>
   */
  CompositeReaderContext(
      std::shared_ptr<CompositeReader> reader,
      std::deque<std::shared_ptr<IndexReaderContext>> &children,
      std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

private:
  CompositeReaderContext(
      std::shared_ptr<CompositeReaderContext> parent,
      std::shared_ptr<CompositeReader> reader, int ordInParent,
      int docbaseInParent,
      std::deque<std::shared_ptr<IndexReaderContext>> &children,
      std::deque<std::shared_ptr<LeafReaderContext>> &leaves);

public:
  std::deque<std::shared_ptr<LeafReaderContext>>
  leaves()  override;

  std::deque<std::shared_ptr<IndexReaderContext>> children() override;

  std::shared_ptr<IndexReader> reader() override;

private:
  class Builder final : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const std::shared_ptr<CompositeReader> reader;
    const std::deque<std::shared_ptr<LeafReaderContext>> leaves =
        std::deque<std::shared_ptr<LeafReaderContext>>();
    int leafDocBase = 0;

  public:
    Builder(std::shared_ptr<CompositeReader> reader);

    std::shared_ptr<CompositeReaderContext> build();

  private:
    std::shared_ptr<IndexReaderContext>
    build(std::shared_ptr<CompositeReaderContext> parent,
          std::shared_ptr<IndexReader> reader, int ord, int docBase);
  };

protected:
  std::shared_ptr<CompositeReaderContext> shared_from_this()
  {
    return std::static_pointer_cast<CompositeReaderContext>(
        IndexReaderContext::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/index/
