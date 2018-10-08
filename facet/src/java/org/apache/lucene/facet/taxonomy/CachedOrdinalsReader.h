#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/facet/taxonomy/OrdinalsReader.h"

#include  "core/src/java/org/apache/lucene/facet/taxonomy/CachedOrds.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::facet::taxonomy
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * A per-segment cache of documents' facet ordinals. Every
 * {@link CachedOrds} holds the ordinals in a raw {@code
 * int[]}, and therefore consumes as much RAM as the total
 * number of ordinals found in the segment, but saves the
 * CPU cost of decoding ordinals during facet counting.
 *
 * <p>
 * <b>NOTE:</b> every {@link CachedOrds} is limited to 2.1B
 * total ordinals. If that is a limitation for you then
 * consider limiting the segment size to fewer documents, or
 * use an alternative cache which pages through the category
 * ordinals.
 *
 * <p>
 * <b>NOTE:</b> when using this cache, it is advised to use
 * a {@link DocValuesFormat} that does not cache the data in
 * memory, at least for the category lists fields, or
 * otherwise you'll be doing double-caching.
 *
 * <p>
 * <b>NOTE:</b> create one instance of this and re-use it
 * for all facet implementations (the cache is per-instance,
 * not static).
 */
class CachedOrdinalsReader : public OrdinalsReader, public Accountable
{
  GET_CLASS_NAME(CachedOrdinalsReader)

private:
  const std::shared_ptr<OrdinalsReader> source;

  const std::unordered_map<std::any, std::shared_ptr<CachedOrds>> ordsCache =
      std::make_shared<WeakHashMap<std::any, std::shared_ptr<CachedOrds>>>();

  /** Sole constructor. */
public:
  CachedOrdinalsReader(std::shared_ptr<OrdinalsReader> source);

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<CachedOrds>
  getCachedOrds(std::shared_ptr<LeafReaderContext> context) ;

public:
  std::wstring getIndexFieldName() override;

  std::shared_ptr<OrdinalsSegmentReader> getReader(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class OrdinalsSegmentReaderAnonymousInnerClass : public OrdinalsSegmentReader
  {
    GET_CLASS_NAME(OrdinalsSegmentReaderAnonymousInnerClass)
  private:
    std::shared_ptr<CachedOrdinalsReader> outerInstance;

    std::shared_ptr<
        org::apache::lucene::facet::taxonomy::CachedOrdinalsReader::CachedOrds>
        cachedOrds;

  public:
    OrdinalsSegmentReaderAnonymousInnerClass(
        std::shared_ptr<CachedOrdinalsReader> outerInstance,
        std::shared_ptr<org::apache::lucene::facet::taxonomy::
                            CachedOrdinalsReader::CachedOrds>
            cachedOrds);

    void get(int docID, std::shared_ptr<IntsRef> ordinals) override;

  protected:
    std::shared_ptr<OrdinalsSegmentReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<OrdinalsSegmentReaderAnonymousInnerClass>(
          OrdinalsSegmentReader::shared_from_this());
    }
  };

  /** Holds the cached ordinals in two parallel {@code int[]} arrays. */
public:
  class CachedOrds final : public std::enable_shared_from_this<CachedOrds>,
                           public Accountable
  {
    GET_CLASS_NAME(CachedOrds)

    /** Index into {@link #ordinals} for each document. */
  public:
    std::deque<int> const offsets;

    /** Holds ords for all docs. */
    std::deque<int> const ordinals;

    /**
     * Creates a new {@link CachedOrds} from the {@link BinaryDocValues}.
     * Assumes that the {@link BinaryDocValues} is not {@code null}.
     */
    CachedOrds(std::shared_ptr<OrdinalsSegmentReader> source,
               int maxDoc) ;

    int64_t ramBytesUsed() override;
  };

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

protected:
  std::shared_ptr<CachedOrdinalsReader> shared_from_this()
  {
    return std::static_pointer_cast<CachedOrdinalsReader>(
        OrdinalsReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
