using namespace std;

#include "FloatAssociationFacetField.h"

namespace org::apache::lucene::facet::taxonomy
{
using Document = org::apache::lucene::document::Document;
using BytesRef = org::apache::lucene::util::BytesRef;

FloatAssociationFacetField::FloatAssociationFacetField(float assoc,
                                                       const wstring &dim,
                                                       deque<wstring> &path)
    : AssociationFacetField(floatToBytesRef(assoc), dim, path)
{
}

shared_ptr<BytesRef> FloatAssociationFacetField::floatToBytesRef(float v)
{
  return IntAssociationFacetField::intToBytesRef(Float::floatToIntBits(v));
}

float FloatAssociationFacetField::bytesRefToFloat(shared_ptr<BytesRef> b)
{
  return Float::intBitsToFloat(IntAssociationFacetField::bytesRefToInt(b));
}

wstring FloatAssociationFacetField::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"FloatAssociationFacetField(dim=" + dim + L" path=" +
         Arrays->toString(path) + L" value=" +
         to_wstring(bytesRefToFloat(assoc)) + L")";
}
} // namespace org::apache::lucene::facet::taxonomy