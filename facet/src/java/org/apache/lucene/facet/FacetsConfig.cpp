using namespace std;

#include "FacetsConfig.h"

namespace org::apache::lucene::facet
{
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using SortedSetDocValuesField =
    org::apache::lucene::document::SortedSetDocValuesField;
using StringField = org::apache::lucene::document::StringField;
using SortedSetDocValuesFacetField =
    org::apache::lucene::facet::sortedset::SortedSetDocValuesFacetField;
using AssociationFacetField =
    org::apache::lucene::facet::taxonomy::AssociationFacetField;
using FacetLabel = org::apache::lucene::facet::taxonomy::FacetLabel;
using FloatAssociationFacetField =
    org::apache::lucene::facet::taxonomy::FloatAssociationFacetField;
using IntAssociationFacetField =
    org::apache::lucene::facet::taxonomy::IntAssociationFacetField;
using TaxonomyWriter = org::apache::lucene::facet::taxonomy::TaxonomyWriter;
using IndexableField = org::apache::lucene::index::IndexableField;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
const wstring FacetsConfig::DEFAULT_INDEX_FIELD_NAME = L"$facets";

FacetsConfig::DimConfig::DimConfig() {}

const shared_ptr<DimConfig> FacetsConfig::DEFAULT_DIM_CONFIG =
    make_shared<DimConfig>();

FacetsConfig::FacetsConfig() {}

shared_ptr<DimConfig> FacetsConfig::getDefaultDimConfig()
{
  return DEFAULT_DIM_CONFIG;
}

shared_ptr<DimConfig> FacetsConfig::getDimConfig(const wstring &dimName)
{
  shared_ptr<DimConfig> ft = fieldTypes[dimName];
  if (ft == nullptr) {
    ft = getDefaultDimConfig();
  }
  return ft;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FacetsConfig::setHierarchical(const wstring &dimName, bool v)
{
  shared_ptr<DimConfig> ft = fieldTypes[dimName];
  if (ft == nullptr) {
    ft = make_shared<DimConfig>();
    fieldTypes.emplace(dimName, ft);
  }
  ft->hierarchical = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FacetsConfig::setMultiValued(const wstring &dimName, bool v)
{
  shared_ptr<DimConfig> ft = fieldTypes[dimName];
  if (ft == nullptr) {
    ft = make_shared<DimConfig>();
    fieldTypes.emplace(dimName, ft);
  }
  ft->multiValued = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FacetsConfig::setRequireDimCount(const wstring &dimName, bool v)
{
  shared_ptr<DimConfig> ft = fieldTypes[dimName];
  if (ft == nullptr) {
    ft = make_shared<DimConfig>();
    fieldTypes.emplace(dimName, ft);
  }
  ft->requireDimCount = v;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FacetsConfig::setIndexFieldName(const wstring &dimName,
                                     const wstring &indexFieldName)
{
  shared_ptr<DimConfig> ft = fieldTypes[dimName];
  if (ft == nullptr) {
    ft = make_shared<DimConfig>();
    fieldTypes.emplace(dimName, ft);
  }
  ft->indexFieldName = indexFieldName;
}

unordered_map<wstring, std::shared_ptr<DimConfig>> FacetsConfig::getDimConfigs()
{
  return fieldTypes;
}

void FacetsConfig::checkSeen(shared_ptr<Set<wstring>> seenDims,
                             const wstring &dim)
{
  if (seenDims->contains(dim)) {
    throw invalid_argument(L"dimension \"" + dim +
                           L"\" is not multiValued, but it appears more than "
                           L"once in this document");
  }
  seenDims->add(dim);
}

shared_ptr<Document>
FacetsConfig::build(shared_ptr<Document> doc) 
{
  return build(nullptr, doc);
}

shared_ptr<Document>
FacetsConfig::build(shared_ptr<TaxonomyWriter> taxoWriter,
                    shared_ptr<Document> doc) 
{
  // Find all FacetFields, collated by the actual field:
  unordered_map<wstring, deque<std::shared_ptr<FacetField>>> byField =
      unordered_map<wstring, deque<std::shared_ptr<FacetField>>>();

  // ... and also all SortedSetDocValuesFacetFields:
  unordered_map<wstring, deque<std::shared_ptr<SortedSetDocValuesFacetField>>>
      dvByField = unordered_map<
          wstring, deque<std::shared_ptr<SortedSetDocValuesFacetField>>>();

  // ... and also all AssociationFacetFields
  unordered_map<wstring, deque<std::shared_ptr<AssociationFacetField>>>
      assocByField =
          unordered_map<wstring,
                        deque<std::shared_ptr<AssociationFacetField>>>();

  shared_ptr<Set<wstring>> seenDims = unordered_set<wstring>();

  for (auto field : doc) {
    if (field->fieldType() == FacetField::TYPE) {
      shared_ptr<FacetField> facetField =
          std::static_pointer_cast<FacetField>(field);
      shared_ptr<FacetsConfig::DimConfig> dimConfig =
          getDimConfig(facetField->dim);
      if (dimConfig->multiValued == false) {
        checkSeen(seenDims, facetField->dim);
      }
      wstring indexFieldName = dimConfig->indexFieldName;
      deque<std::shared_ptr<FacetField>> fields = byField[indexFieldName];
      if (fields.empty()) {
        fields = deque<>();
        byField.emplace(indexFieldName, fields);
      }
      fields.push_back(facetField);
    }

    if (field->fieldType() == SortedSetDocValuesFacetField::TYPE) {
      shared_ptr<SortedSetDocValuesFacetField> facetField =
          std::static_pointer_cast<SortedSetDocValuesFacetField>(field);
      shared_ptr<FacetsConfig::DimConfig> dimConfig =
          getDimConfig(facetField->dim);
      if (dimConfig->multiValued == false) {
        checkSeen(seenDims, facetField->dim);
      }
      wstring indexFieldName = dimConfig->indexFieldName;
      deque<std::shared_ptr<SortedSetDocValuesFacetField>> fields =
          dvByField[indexFieldName];
      if (fields.empty()) {
        fields = deque<>();
        dvByField.emplace(indexFieldName, fields);
      }
      fields.push_back(facetField);
    }

    if (field->fieldType() == AssociationFacetField::TYPE) {
      shared_ptr<AssociationFacetField> facetField =
          std::static_pointer_cast<AssociationFacetField>(field);
      shared_ptr<FacetsConfig::DimConfig> dimConfig =
          getDimConfig(facetField->dim);
      if (dimConfig->multiValued == false) {
        checkSeen(seenDims, facetField->dim);
      }
      if (dimConfig->hierarchical) {
        throw invalid_argument(
            L"AssociationFacetField cannot be hierarchical (dim=\"" +
            facetField->dim + L"\")");
      }
      if (dimConfig->requireDimCount) {
        throw invalid_argument(
            L"AssociationFacetField cannot requireDimCount (dim=\"" +
            facetField->dim + L"\")");
      }

      wstring indexFieldName = dimConfig->indexFieldName;
      deque<std::shared_ptr<AssociationFacetField>> fields =
          assocByField[indexFieldName];
      if (fields.empty()) {
        fields = deque<>();
        assocByField.emplace(indexFieldName, fields);
      }
      fields.push_back(facetField);

      // Best effort: detect mis-matched types in same
      // indexed field:
      wstring type;
      if (std::dynamic_pointer_cast<IntAssociationFacetField>(facetField) !=
          nullptr) {
        type = L"int";
      } else if (std::dynamic_pointer_cast<FloatAssociationFacetField>(
                     facetField) != nullptr) {
        type = L"float";
      } else {
        type = L"bytes";
      }
      // NOTE: not thread safe, but this is just best effort:
      wstring curType = assocDimTypes[indexFieldName];
      if (curType == L"") {
        assocDimTypes.emplace(indexFieldName, type);
      } else if (curType != type) {
        throw invalid_argument(
            L"mixing incompatible types of AssocationFacetField (" + curType +
            L" and " + type + L") in indexed field \"" + indexFieldName +
            L"\"; use FacetsConfig to change the indexFieldName for each "
            L"dimension");
      }
    }
  }

  shared_ptr<Document> result = make_shared<Document>();

  processFacetFields(taxoWriter, byField, result);
  processSSDVFacetFields(dvByField, result);
  processAssocFacetFields(taxoWriter, assocByField, result);

  // System.out.println("add stored: " + addedStoredFields);

  for (auto field : doc->getFields()) {
    shared_ptr<IndexableFieldType> ft = field->fieldType();
    if (ft != FacetField::TYPE && ft != SortedSetDocValuesFacetField::TYPE &&
        ft != AssociationFacetField::TYPE) {
      result->push_back(field);
    }
  }

  // System.out.println("all indexed: " + allIndexedFields);
  // System.out.println("all stored: " + allStoredFields);

  return result;
}

void FacetsConfig::processFacetFields(
    shared_ptr<TaxonomyWriter> taxoWriter,
    unordered_map<wstring, deque<std::shared_ptr<FacetField>>> &byField,
    shared_ptr<Document> doc) 
{

  for (auto ent : byField) {

    wstring indexFieldName = ent.first;
    // System.out.println("  indexFieldName=" + indexFieldName + " fields=" +
    // ent.getValue());

    shared_ptr<IntsRefBuilder> ordinals = make_shared<IntsRefBuilder>();
    for (shared_ptr<FacetField> facetField : ent.second) {

      shared_ptr<FacetsConfig::DimConfig> ft = getDimConfig(facetField->dim);
      if (facetField->path.size() > 1 && ft->hierarchical == false) {
        throw invalid_argument(L"dimension \"" + facetField->dim +
                               L"\" is not hierarchical yet has " +
                               facetField->path.size() + L" components");
      }

      shared_ptr<FacetLabel> cp =
          make_shared<FacetLabel>(facetField->dim, facetField->path);

      checkTaxoWriter(taxoWriter);
      int ordinal = taxoWriter->addCategory(cp);
      ordinals->append(ordinal);
      // System.out.println("ords[" + (ordinals.length-1) + "]=" + ordinal);
      // System.out.println("  add cp=" + cp);

      if (ft->multiValued && (ft->hierarchical || ft->requireDimCount)) {
        // System.out.println("  add parents");
        // Add all parents too:
        int parent = taxoWriter->getParent(ordinal);
        while (parent > 0) {
          ordinals->append(parent);
          parent = taxoWriter->getParent(parent);
        }

        if (ft->requireDimCount == false) {
          // Remove last (dimension) ord:
          ordinals->setLength(ordinals->length() - 1);
        }
      }

      // Drill down:
      for (int i = 1; i <= cp->length; i++) {
        doc->push_back(make_shared<StringField>(
            indexFieldName, pathToString(cp->components, i), Field::Store::NO));
      }
    }

    // Facet counts:
    // DocValues are considered stored fields:
    doc->push_back(make_shared<BinaryDocValuesField>(
        indexFieldName, dedupAndEncode(ordinals->get())));
  }
}

void FacetsConfig::processSSDVFacetFields(
    unordered_map<wstring,
                  deque<std::shared_ptr<SortedSetDocValuesFacetField>>>
        &byField,
    shared_ptr<Document> doc) 
{
  // System.out.println("process SSDV: " + byField);
  for (auto ent : byField) {

    wstring indexFieldName = ent.first;
    // System.out.println("  field=" + indexFieldName);

    for (shared_ptr<SortedSetDocValuesFacetField> facetField : ent.second) {
      shared_ptr<FacetLabel> cp =
          make_shared<FacetLabel>(facetField->dim, facetField->label);
      wstring fullPath = pathToString(cp->components, cp->length);
      // System.out.println("add " + fullPath);

      // For facet counts:
      doc->push_back(make_shared<SortedSetDocValuesField>(
          indexFieldName, make_shared<BytesRef>(fullPath)));

      // For drill-down:
      doc->push_back(
          make_shared<StringField>(indexFieldName, fullPath, Field::Store::NO));
      doc->push_back(make_shared<StringField>(indexFieldName, facetField->dim,
                                              Field::Store::NO));
    }
  }
}

void FacetsConfig::processAssocFacetFields(
    shared_ptr<TaxonomyWriter> taxoWriter,
    unordered_map<wstring, deque<std::shared_ptr<AssociationFacetField>>>
        &byField,
    shared_ptr<Document> doc) 
{
  for (auto ent : byField) {
    std::deque<char> bytes(16);
    int upto = 0;
    wstring indexFieldName = ent.first;
    for (shared_ptr<AssociationFacetField> field : ent.second) {
      // NOTE: we don't add parents for associations
      checkTaxoWriter(taxoWriter);
      shared_ptr<FacetLabel> label =
          make_shared<FacetLabel>(field->dim, field->path);
      int ordinal = taxoWriter->addCategory(label);
      if (upto + 4 > bytes.size()) {
        bytes = ArrayUtil::grow(bytes, upto + 4);
      }
      // big-endian:
      bytes[upto++] = static_cast<char>(ordinal >> 24);
      bytes[upto++] = static_cast<char>(ordinal >> 16);
      bytes[upto++] = static_cast<char>(ordinal >> 8);
      bytes[upto++] = static_cast<char>(ordinal);
      if (upto + field->assoc->length > bytes.size()) {
        bytes = ArrayUtil::grow(bytes, upto + field->assoc->length);
      }
      System::arraycopy(field->assoc->bytes, field->assoc->offset, bytes, upto,
                        field->assoc->length);
      upto += field->assoc->length;

      // Drill down:
      for (int i = 1; i <= label->length; i++) {
        doc->push_back(make_shared<StringField>(
            indexFieldName, pathToString(label->components, i),
            Field::Store::NO));
      }
    }
    doc->push_back(make_shared<BinaryDocValuesField>(
        indexFieldName, make_shared<BytesRef>(bytes, 0, upto)));
  }
}

shared_ptr<BytesRef> FacetsConfig::dedupAndEncode(shared_ptr<IntsRef> ordinals)
{
  Arrays::sort(ordinals->ints, ordinals->offset, ordinals->length);
  std::deque<char> bytes(5 * ordinals->length);
  int lastOrd = -1;
  int upto = 0;
  for (int i = 0; i < ordinals->length; i++) {
    int ord = ordinals->ints[ordinals->offset + i];
    // ord could be == lastOrd, so we must dedup:
    if (ord > lastOrd) {
      int delta;
      if (lastOrd == -1) {
        delta = ord;
      } else {
        delta = ord - lastOrd;
      }
      if ((delta & ~0x7F) == 0) {
        bytes[upto] = static_cast<char>(delta);
        upto++;
      } else if ((delta & ~0x3FFF) == 0) {
        bytes[upto] = static_cast<char>(0x80 | ((delta & 0x3F80) >> 7));
        bytes[upto + 1] = static_cast<char>(delta & 0x7F);
        upto += 2;
      } else if ((delta & ~0x1FFFFF) == 0) {
        bytes[upto] = static_cast<char>(0x80 | ((delta & 0x1FC000) >> 14));
        bytes[upto + 1] = static_cast<char>(0x80 | ((delta & 0x3F80) >> 7));
        bytes[upto + 2] = static_cast<char>(delta & 0x7F);
        upto += 3;
      } else if ((delta & ~0xFFFFFFF) == 0) {
        bytes[upto] = static_cast<char>(0x80 | ((delta & 0xFE00000) >> 21));
        bytes[upto + 1] = static_cast<char>(0x80 | ((delta & 0x1FC000) >> 14));
        bytes[upto + 2] = static_cast<char>(0x80 | ((delta & 0x3F80) >> 7));
        bytes[upto + 3] = static_cast<char>(delta & 0x7F);
        upto += 4;
      } else {
        bytes[upto] = static_cast<char>(0x80 | ((delta & 0xF0000000) >> 28));
        bytes[upto + 1] = static_cast<char>(0x80 | ((delta & 0xFE00000) >> 21));
        bytes[upto + 2] = static_cast<char>(0x80 | ((delta & 0x1FC000) >> 14));
        bytes[upto + 3] = static_cast<char>(0x80 | ((delta & 0x3F80) >> 7));
        bytes[upto + 4] = static_cast<char>(delta & 0x7F);
        upto += 5;
      }
      lastOrd = ord;
    }
  }
  return make_shared<BytesRef>(bytes, 0, upto);
}

void FacetsConfig::checkTaxoWriter(shared_ptr<TaxonomyWriter> taxoWriter)
{
  if (taxoWriter == nullptr) {
    throw make_shared<IllegalStateException>(
        L"a non-null TaxonomyWriter must be provided when indexing FacetField "
        L"or AssociationFacetField");
  }
}

wstring FacetsConfig::pathToString(const wstring &dim,
                                   std::deque<wstring> &path)
{
  std::deque<wstring> fullPath(1 + path.size());
  fullPath[0] = dim;
  System::arraycopy(path, 0, fullPath, 1, path.size());
  return pathToString(fullPath, fullPath.size());
}

wstring FacetsConfig::pathToString(std::deque<wstring> &path)
{
  return pathToString(path, path.size());
}

wstring FacetsConfig::pathToString(std::deque<wstring> &path, int length)
{
  if (length == 0) {
    return L"";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < length; i++) {
    wstring s = path[i];
    if (s.length() == 0) {
      throw invalid_argument(
          L"each path component must have length > 0 (got: \"\")");
    }
    int numChars = s.length();
    for (int j = 0; j < numChars; j++) {
      wchar_t ch = s[j];
      if (ch == DELIM_CHAR || ch == ESCAPE_CHAR) {
        sb->append(ESCAPE_CHAR);
      }
      sb->append(ch);
    }
    sb->append(DELIM_CHAR);
  }

  // Trim off last DELIM_CHAR:
  sb->setLength(sb->length() - 1);
  return sb->toString();
}

std::deque<wstring> FacetsConfig::stringToPath(const wstring &s)
{
  deque<wstring> parts = deque<wstring>();
  int length = s.length();
  if (length == 0) {
    return std::deque<wstring>(0);
  }
  std::deque<wchar_t> buffer(length);

  int upto = 0;
  bool lastEscape = false;
  for (int i = 0; i < length; i++) {
    wchar_t ch = s[i];
    if (lastEscape) {
      buffer[upto++] = ch;
      lastEscape = false;
    } else if (ch == ESCAPE_CHAR) {
      lastEscape = true;
    } else if (ch == DELIM_CHAR) {
      parts.push_back(wstring(buffer, 0, upto));
      upto = 0;
    } else {
      buffer[upto++] = ch;
    }
  }
  parts.push_back(wstring(buffer, 0, upto));
  assert(!lastEscape);
  return parts.toArray(std::deque<wstring>(parts.size()));
}
} // namespace org::apache::lucene::facet