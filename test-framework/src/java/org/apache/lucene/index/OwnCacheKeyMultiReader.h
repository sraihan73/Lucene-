#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <deque>

// C++ NOTE: Forward class declarations:
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
 * A {@link MultiReader} that has its own cache key, occasionally useful for
 * testing purposes.
 */
class OwnCacheKeyMultiReader final : public MultiReader
{
  GET_CLASS_NAME(OwnCacheKeyMultiReader)

private:
  const std::shared_ptr<Set<std::shared_ptr<ClosedListener>>>
      readerClosedListeners = std::make_shared<
          CopyOnWriteArraySet<std::shared_ptr<ClosedListener>>>();

  const std::shared_ptr<CacheHelper> cacheHelper =
      std::make_shared<CacheHelperAnonymousInnerClass>();

private:
  class CacheHelperAnonymousInnerClass : public CacheHelper
  {
    GET_CLASS_NAME(CacheHelperAnonymousInnerClass)
  public:
    CacheHelperAnonymousInnerClass();

  private:
    const std::shared_ptr<CacheKey> cacheKey = std::make_shared<CacheKey>();

  public:
    std::shared_ptr<CacheKey> getKey() override;

    void addClosedListener(std::shared_ptr<ClosedListener> listener) override;

  protected:
    std::shared_ptr<CacheHelperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CacheHelperAnonymousInnerClass>(
          CacheHelper::shared_from_this());
    }
  };

  /** Sole constructor. */
public:
  OwnCacheKeyMultiReader(std::deque<IndexReader> &subReaders) throw(
      IOException);

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  void notifyReaderClosedListeners()  override;

protected:
  std::shared_ptr<OwnCacheKeyMultiReader> shared_from_this()
  {
    return std::static_pointer_cast<OwnCacheKeyMultiReader>(
        MultiReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
