#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::facet
{
class DimConfig;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::facet::taxonomy
{
class TaxonomyWriter;
}
namespace org::apache::lucene::facet
{
class FacetField;
}
namespace org::apache::lucene::facet::sortedset
{
class SortedSetDocValuesFacetField;
}
namespace org::apache::lucene::facet::taxonomy
{
class AssociationFacetField;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class IntsRef;
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
namespace org::apache::lucene::facet
{

using Document = org::apache::lucene::document::Document;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using AssociationFacetField =
    org::apache::lucene::facet::taxonomy::AssociationFacetField;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

/** Records per-dimension configuration.  By default a
 *  dimension is flat, single valued and does
 *  not require count for the dimension; use
 *  the setters in this class to change these settings for
 *  each dim.
 *
 *  <p><b>NOTE</b>: this configuration is not saved into the
 *  index, but it's vital, and up to the application to
 *  ensure, that at search time the provided {@code
 *  FacetsConfig} matches what was used during indexing.
 *
 *  @lucene.experimental */
class FacetsConfig : public std::enable_shared_from_this<FacetsConfig>
{
  GET_CLASS_NAME(FacetsConfig)

  /** Which Lucene field holds the drill-downs and ords (as
   *  doc values). */
public:
  static const std::wstring DEFAULT_INDEX_FIELD_NAME;

private:
  const std::unordered_map<std::wstring, std::shared_ptr<DimConfig>>
      fieldTypes = std::make_shared<
          ConcurrentHashMap<std::wstring, std::shared_ptr<DimConfig>>>();

  // Used only for best-effort detection of app mixing
  // int/float/bytes in a single indexed field:
  const std::unordered_map<std::wstring, std::wstring> assocDimTypes =
      std::make_shared<ConcurrentHashMap<std::wstring, std::wstring>>();

  /** Holds the configuration for one dimension
   *
   * @lucene.experimental */
public:
  class DimConfig final : public std::enable_shared_from_this<DimConfig>
  {
    GET_CLASS_NAME(DimConfig)
    /** True if this dimension is hierarchical. */
  public:
    bool hierarchical = false;

    /** True if this dimension is multi-valued. */
    bool multiValued = false;

    /** True if the count/aggregate for the entire dimension
     *  is required, which is unusual (default is false). */
    bool requireDimCount = false;

    /** Actual field where this dimension's facet labels
     *  should be indexed */
    std::wstring indexFieldName = DEFAULT_INDEX_FIELD_NAME;

    /** Default constructor. */
    DimConfig();
  };

  /** Default per-dimension configuration. */
public:
  static const std::shared_ptr<DimConfig> DEFAULT_DIM_CONFIG;

  /** Default constructor. */
  FacetsConfig();

  /** Get the default configuration for new dimensions.  Useful when
   *  the dimension is not known beforehand and may need different
   *  global default settings, like {@code multivalue =
   *  true}.
   *
   *  @return The default configuration to be used for dimensions that
   *  are not yet set in the {@link FacetsConfig} */
protected:
  virtual std::shared_ptr<DimConfig> getDefaultDimConfig();

  /** Get the current configuration for a dimension. */
public:
  virtual std::shared_ptr<DimConfig> getDimConfig(const std::wstring &dimName);

  /** Pass {@code true} if this dimension is hierarchical
   *  (has depth &gt; 1 paths). */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setHierarchical(const std::wstring &dimName, bool v);

  /** Pass {@code true} if this dimension may have more than
   *  one value per document. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setMultiValued(const std::wstring &dimName, bool v);

  /** Pass {@code true} if at search time you require
   *  accurate counts of the dimension, i.e. how many
   *  hits have this dimension. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setRequireDimCount(const std::wstring &dimName, bool v);

  /** Specify which index field name should hold the
   *  ordinals for this dimension; this is only used by the
   *  taxonomy based facet methods. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void setIndexFieldName(const std::wstring &dimName,
                                 const std::wstring &indexFieldName);

  /** Returns map_obj of field name to {@link DimConfig}. */
  virtual std::unordered_map<std::wstring, std::shared_ptr<DimConfig>>
  getDimConfigs();

private:
  static void checkSeen(std::shared_ptr<Set<std::wstring>> seenDims,
                        const std::wstring &dim);

  /**
   * Translates any added {@link FacetField}s into normal fields for indexing;
   * only use this version if you did not add any taxonomy-based fields (
   * {@link FacetField} or {@link AssociationFacetField}).
   *
   * <p>
   * <b>NOTE:</b> you should add the returned document to IndexWriter, not the
   * input one!
   */
public:
  virtual std::shared_ptr<Document>
  build(std::shared_ptr<Document> doc) ;

  /**
   * Translates any added {@link FacetField}s into normal fields for indexing.
   *
   * <p>
   * <b>NOTE:</b> you should add the returned document to IndexWriter, not the
   * input one!
   */
  virtual std::shared_ptr<Document>
  build(std::shared_ptr<TaxonomyWriter> taxoWriter,
        std::shared_ptr<Document> doc) ;

private:
  void processFacetFields(
      std::shared_ptr<TaxonomyWriter> taxoWriter,
      std::unordered_map<std::wstring, std::deque<std::shared_ptr<FacetField>>>
          &byField,
      std::shared_ptr<Document> doc) ;

  void processSSDVFacetFields(
      std::unordered_map<
          std::wstring,
          std::deque<std::shared_ptr<SortedSetDocValuesFacetField>>> &byField,
      std::shared_ptr<Document> doc) ;

  void processAssocFacetFields(
      std::shared_ptr<TaxonomyWriter> taxoWriter,
      std::unordered_map<std::wstring,
                         std::deque<std::shared_ptr<AssociationFacetField>>>
          &byField,
      std::shared_ptr<Document> doc) ;

  /** Encodes ordinals into a BytesRef; expert: subclass can
   *  override this to change encoding. */
protected:
  virtual std::shared_ptr<BytesRef>
  dedupAndEncode(std::shared_ptr<IntsRef> ordinals);

private:
  void checkTaxoWriter(std::shared_ptr<TaxonomyWriter> taxoWriter);

  // Joins the path components together:
  static constexpr wchar_t DELIM_CHAR = L'\u001F';

  // Escapes any occurrence of the path component inside the label:
  static constexpr wchar_t ESCAPE_CHAR = L'\u001E';

  /** Turns a dim + path into an encoded string. */
public:
  static std::wstring pathToString(const std::wstring &dim,
                                   std::deque<std::wstring> &path);

  /** Turns a dim + path into an encoded string. */
  static std::wstring pathToString(std::deque<std::wstring> &path);

  /** Turns the first {@code length} elements of {@code
   * path} into an encoded string. */
  static std::wstring pathToString(std::deque<std::wstring> &path, int length);

  /** Turns an encoded string (from a previous call to {@link
   *  #pathToString}) back into the original {@code
   *  std::wstring[]}. */
  static std::deque<std::wstring> stringToPath(const std::wstring &s);
};

} // namespace org::apache::lucene::facet
