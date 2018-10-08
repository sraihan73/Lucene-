using namespace std;

#include "FacetField.h"

namespace org::apache::lucene::facet
{
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
const shared_ptr<org::apache::lucene::document::FieldType> FacetField::TYPE =
    make_shared<org::apache::lucene::document::FieldType>();

FacetField::StaticConstructor::StaticConstructor()
{
  TYPE->setIndexOptions(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS);
  TYPE->freeze();
}

FacetField::StaticConstructor FacetField::staticConstructor;

FacetField::FacetField(const wstring &dim, deque<wstring> &path)
    : org::apache::lucene::document::Field(L"dummy", TYPE), dim(dim), path(path)
{
  verifyLabel(dim);
  for (auto label : path) {
    verifyLabel(label);
  }
  if (path.empty()) {
    throw invalid_argument(L"path must have at least one element");
  }
}

wstring FacetField::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FacetField(dim=" + dim + L" path=" + Arrays->toString(path) + L")";
}

void FacetField::verifyLabel(const wstring &label)
{
  if (label == L"" || label.isEmpty()) {
    throw invalid_argument(L"empty or null components not allowed; got: " +
                           label);
  }
}
} // namespace org::apache::lucene::facet