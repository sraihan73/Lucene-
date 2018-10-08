using namespace std;

#include "SortedSetDocValuesFacetField.h"

namespace org::apache::lucene::facet::sortedset
{
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using FacetField = org::apache::lucene::facet::FacetField;
using IndexOptions = org::apache::lucene::index::IndexOptions;
const shared_ptr<org::apache::lucene::document::FieldType>
    SortedSetDocValuesFacetField::TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

SortedSetDocValuesFacetField::StaticConstructor::StaticConstructor()
{
  // NOTE: we don't actually use these index options, because this field is
  // "processed" by FacetsConfig.build()
  TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE->freeze();
}

SortedSetDocValuesFacetField::StaticConstructor
    SortedSetDocValuesFacetField::staticConstructor;

SortedSetDocValuesFacetField::SortedSetDocValuesFacetField(const wstring &dim,
                                                           const wstring &label)
    : org::apache::lucene::document::Field(L"dummy", TYPE), dim(dim),
      label(label)
{
  FacetField::verifyLabel(label);
  FacetField::verifyLabel(dim);
}

wstring SortedSetDocValuesFacetField::toString()
{
  return L"SortedSetDocValuesFacetField(dim=" + dim + L" label=" + label + L")";
}
} // namespace org::apache::lucene::facet::sortedset