#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::facet::taxonomy
{
class OrdinalsReader;
}
namespace org::apache::lucene::facet::taxonomy
{
class OrdinalsSegmentReader;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::facet
{
class FacetsConfig;
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
namespace org::apache::lucene::facet::taxonomy
{

using FacetsConfig = org::apache::lucene::facet::FacetsConfig;
using OrdinalsSegmentReader =
    org::apache::lucene::facet::taxonomy::OrdinalsReader::OrdinalsSegmentReader;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FilterBinaryDocValues = org::apache::lucene::index::FilterBinaryDocValues;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

/**
 * A {@link org.apache.lucene.index.FilterLeafReader} for updating facets
 * ordinal references, based on an ordinal map_obj. You should use this code in
 * conjunction with merging taxonomies - after you merge taxonomies, you receive
 * an {@link OrdinalMap} which maps the 'old' ordinals to the 'new' ones. You
 * can use that map_obj to re-map_obj the doc values which contain the facets
 * information (ordinals) either before or while merging the indexes. <p> For
 * re-mapping the ordinals during index merge, do the following:
 *
 * <pre class="prettyprint">
 * // merge the old taxonomy with the new one.
 * OrdinalMap map_obj = new MemoryOrdinalMap();
 * DirectoryTaxonomyWriter.addTaxonomy(srcTaxoDir, map_obj);
 * int[] ordmap = map_obj.getMap();
 *
 * // Add the index and re-map_obj ordinals on the go
 * DirectoryReader reader = DirectoryReader.open(oldDir);
 * IndexWriterConfig conf = new IndexWriterConfig(VER, ANALYZER);
 * IndexWriter writer = new IndexWriter(newDir, conf);
 * List&lt;LeafReaderContext&gt; leaves = reader.leaves();
 * LeafReader wrappedLeaves[] = new LeafReader[leaves.size()];
 * for (int i = 0; i &lt; leaves.size(); i++) {
 *   wrappedLeaves[i] = new OrdinalMappingLeafReader(leaves.get(i).reader(),
 * ordmap);
 * }
 * writer.addIndexes(new MultiReader(wrappedLeaves));
 * writer.commit();
 * </pre>
 *
 * @lucene.experimental
 */
class OrdinalMappingLeafReader : public FilterLeafReader
{
  GET_CLASS_NAME(OrdinalMappingLeafReader)

  // silly way, but we need to use dedupAndEncode and it's protected on
  // FacetsConfig.
private:
  class InnerFacetsConfig : public FacetsConfig
  {
    GET_CLASS_NAME(InnerFacetsConfig)

  public:
    InnerFacetsConfig();

    std::shared_ptr<BytesRef>
    dedupAndEncode(std::shared_ptr<IntsRef> ordinals) override;

  protected:
    std::shared_ptr<InnerFacetsConfig> shared_from_this()
    {
      return std::static_pointer_cast<InnerFacetsConfig>(
          org.apache.lucene.facet.FacetsConfig::shared_from_this());
    }
  };

private:
  class OrdinalMappingBinaryDocValues : public FilterBinaryDocValues
  {
    GET_CLASS_NAME(OrdinalMappingBinaryDocValues)
  private:
    std::shared_ptr<OrdinalMappingLeafReader> outerInstance;

    const std::shared_ptr<IntsRef> ordinals = std::make_shared<IntsRef>(32);
    const std::shared_ptr<OrdinalsSegmentReader> ordsReader;

  public:
    OrdinalMappingBinaryDocValues(
        std::shared_ptr<OrdinalMappingLeafReader> outerInstance,
        std::shared_ptr<OrdinalsSegmentReader> ordsReader,
        std::shared_ptr<BinaryDocValues> in_) ;

    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("synthetic-access") @Override public
    // org.apache.lucene.util.BytesRef binaryValue()
    std::shared_ptr<BytesRef> binaryValue() override;

  protected:
    std::shared_ptr<OrdinalMappingBinaryDocValues> shared_from_this()
    {
      return std::static_pointer_cast<OrdinalMappingBinaryDocValues>(
          org.apache.lucene.index.FilterBinaryDocValues::shared_from_this());
    }
  };

private:
  std::deque<int> const ordinalMap;
  const std::shared_ptr<InnerFacetsConfig> facetsConfig;
  const std::shared_ptr<Set<std::wstring>> facetFields;

  /**
   * Wraps an LeafReader, mapping ordinals according to the ordinalMap, using
   * the provided {@link FacetsConfig} which was used to build the wrapped
   * reader.
   */
public:
  OrdinalMappingLeafReader(std::shared_ptr<LeafReader> in_,
                           std::deque<int> &ordinalMap,
                           std::shared_ptr<FacetsConfig> srcConfig);

  /**
   * Expert: encodes category ordinals into a BytesRef. Override in case you use
   * custom encoding, other than the default done by FacetsConfig.
   */
protected:
  virtual std::shared_ptr<BytesRef> encode(std::shared_ptr<IntsRef> ordinals);

  /**
   * Expert: override in case you used custom encoding for the categories under
   * this field.
   */
  virtual std::shared_ptr<OrdinalsReader>
  getOrdinalsReader(const std::wstring &field);

public:
  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<OrdinalMappingLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<OrdinalMappingLeafReader>(
        org.apache.lucene.index.FilterLeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::facet::taxonomy
