using namespace std;

#include "AssociationFacetField.h"

namespace org::apache::lucene::facet::taxonomy
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using FacetField = org::apache::lucene::facet::FacetField;
using Facets = org::apache::lucene::facet::Facets;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<org::apache::lucene::document::FieldType>
    AssociationFacetField::TYPE =
        make_shared<org::apache::lucene::document::FieldType>();

AssociationFacetField::StaticConstructor::StaticConstructor()
{
  TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE->freeze();
}

AssociationFacetField::StaticConstructor
    AssociationFacetField::staticConstructor;

AssociationFacetField::AssociationFacetField(shared_ptr<BytesRef> assoc,
                                             const wstring &dim,
                                             deque<wstring> &path)
    : org::apache::lucene::document::Field(L"dummy", TYPE), dim(dim),
      path(path), assoc(assoc)
{
  FacetField::verifyLabel(dim);
  for (auto label : path) {
    FacetField::verifyLabel(label);
  }
  if (path.empty()) {
    throw invalid_argument(L"path must have at least one element");
  }
}

wstring AssociationFacetField::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"AssociationFacetField(dim=" + dim + L" path=" +
         Arrays->toString(path) + L" bytes=" + assoc + L")";
}
} // namespace org::apache::lucene::facet::taxonomy