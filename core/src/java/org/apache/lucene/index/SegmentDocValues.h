#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"

namespace org::apache::lucene::util
{
template <typename T>
class RefCount;
}
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using Directory = org::apache::lucene::store::Directory;
using RefCount = org::apache::lucene::util::RefCount;

/**
 * Manages the {@link DocValuesProducer} held by {@link SegmentReader} and
 * keeps track of their reference counting.
 */
class SegmentDocValues final
    : public std::enable_shared_from_this<SegmentDocValues>
{
  GET_CLASS_NAME(SegmentDocValues)

private:
  const std::unordered_map<int64_t,
                           RefCount<std::shared_ptr<DocValuesProducer>>>
      genDVProducers =
          std::unordered_map<int64_t,
                             RefCount<std::shared_ptr<DocValuesProducer>>>();

  std::shared_ptr<RefCount<std::shared_ptr<DocValuesProducer>>>
  newDocValuesProducer(std::shared_ptr<SegmentCommitInfo> si,
                       std::shared_ptr<Directory> dir,
                       std::optional<int64_t> &gen,
                       std::shared_ptr<FieldInfos> infos) ;

private:
  class RefCountAnonymousInnerClass
      : public RefCount<std::shared_ptr<DocValuesProducer>>
  {
    GET_CLASS_NAME(RefCountAnonymousInnerClass)
  private:
    std::shared_ptr<SegmentDocValues> outerInstance;

    std::optional<int64_t> gen;

  public:
    RefCountAnonymousInnerClass(
        std::shared_ptr<SegmentDocValues> outerInstance,
        std::shared_ptr<DocValuesProducer> fieldsProducer,
        std::optional<int64_t> &gen);

  protected:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override protected
    // void release() throws java.io.IOException
    void release()  override;

  protected:
    std::shared_ptr<RefCountAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RefCountAnonymousInnerClass>(
          org.apache.lucene.util.RefCount<
              org.apache.lucene.codecs.DocValuesProducer>::shared_from_this());
    }
  };

  /** Returns the {@link DocValuesProducer} for the given generation. */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocValuesProducer>
  getDocValuesProducer(int64_t gen, std::shared_ptr<SegmentCommitInfo> si,
                       std::shared_ptr<Directory> dir,
                       std::shared_ptr<FieldInfos> infos) ;

  /**
   * Decrement the reference count of the given {@link DocValuesProducer}
   * generations.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void decRef(std::deque<int64_t> &dvProducersGens) ;
};

} // #include  "core/src/java/org/apache/lucene/index/
