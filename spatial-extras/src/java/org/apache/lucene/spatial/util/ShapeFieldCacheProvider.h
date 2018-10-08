#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

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
namespace org::apache::lucene::spatial::util
{

using org::locationtech::spatial4j::shape::Shape;
using namespace org::apache::lucene::index;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Provides access to a {@link ShapeFieldCache} for a given {@link
 * org.apache.lucene.index.LeafReader}.
 *
 * If a Cache does not exist for the Reader, then it is built by iterating over
 * the all terms for a given field, reconstructing the Shape from them, and
 * adding them to the Cache.
 *
 * @lucene.internal
 */
template <typename T>
class ShapeFieldCacheProvider
    : public std::enable_shared_from_this<ShapeFieldCacheProvider>
{
  GET_CLASS_NAME(ShapeFieldCacheProvider)
  static_assert(
      std::is_base_of<org.locationtech.spatial4j.shape.Shape, T>::value,
      L"T must inherit from org.locationtech.spatial4j.shape.Shape");

private:
  std::shared_ptr<Logger> log = Logger::getLogger(getClassName());

  // it may be a List<T> or T
public:
  std::shared_ptr<WeakHashMap<std::shared_ptr<IndexReader>, ShapeFieldCache<T>>>
      sidx = std::make_shared<
          WeakHashMap<std::shared_ptr<IndexReader>, ShapeFieldCache<T>>>();

protected:
  const int defaultSize;
  const std::wstring shapeField;

public:
  ShapeFieldCacheProvider(const std::wstring &shapeField, int defaultSize)
      : defaultSize(defaultSize), shapeField(shapeField)
  {
  }

protected:
  virtual T readShape(std::shared_ptr<BytesRef> term) = 0;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::shared_ptr<ShapeFieldCache<T>>
  getCache(std::shared_ptr<LeafReader> reader) 
  {
    std::shared_ptr<ShapeFieldCache<T>> idx = sidx->get(reader);
    if (idx != nullptr) {
      return idx;
    }
    int64_t startTime = System::currentTimeMillis();

    log->fine(L"Building Cache [" + std::to_wstring(reader->maxDoc()) + L"]");
    idx = std::make_shared<ShapeFieldCache<T>>(reader->maxDoc(), defaultSize);
    int count = 0;
    std::shared_ptr<PostingsEnum> docs = nullptr;
    std::shared_ptr<Terms> terms = reader->terms(shapeField);
    if (terms != nullptr) {
      std::shared_ptr<TermsEnum> te = terms->begin();
      std::shared_ptr<BytesRef> term = te->next();
      while (term != nullptr) {
        T shape = readShape(term);
        if (shape != nullptr) {
          docs = te->postings(docs, PostingsEnum::NONE);
          std::optional<int> docid = docs->nextDoc();
          while (docid != DocIdSetIterator::NO_MORE_DOCS) {
            idx->add(docid, shape);
            docid = docs->nextDoc();
            count++;
          }
        }
        term = te->next();
      }
    }
    sidx->put(reader, idx);
    int64_t elapsed = System::currentTimeMillis() - startTime;
    log->fine(L"Cached: [" + std::to_wstring(count) + L" in " +
              std::to_wstring(elapsed) + L"ms] " + idx);
    return idx;
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/util/
