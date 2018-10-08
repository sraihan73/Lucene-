using namespace std;

#include "IntAssociationFacetField.h"

namespace org::apache::lucene::facet::taxonomy
{
using Document = org::apache::lucene::document::Document;
using BytesRef = org::apache::lucene::util::BytesRef;

IntAssociationFacetField::IntAssociationFacetField(int assoc,
                                                   const wstring &dim,
                                                   deque<wstring> &path)
    : AssociationFacetField(intToBytesRef(assoc), dim, path)
{
}

shared_ptr<BytesRef> IntAssociationFacetField::intToBytesRef(int v)
{
  std::deque<char> bytes(4);
  // big-endian:
  bytes[0] = static_cast<char>(v >> 24);
  bytes[1] = static_cast<char>(v >> 16);
  bytes[2] = static_cast<char>(v >> 8);
  bytes[3] = static_cast<char>(v);
  return make_shared<BytesRef>(bytes);
}

int IntAssociationFacetField::bytesRefToInt(shared_ptr<BytesRef> b)
{
  return ((b->bytes[b->offset] & 0xFF) << 24) |
         ((b->bytes[b->offset + 1] & 0xFF) << 16) |
         ((b->bytes[b->offset + 2] & 0xFF) << 8) |
         (b->bytes[b->offset + 3] & 0xFF);
}

wstring IntAssociationFacetField::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"IntAssociationFacetField(dim=" + dim + L" path=" +
         Arrays->toString(path) + L" value=" +
         to_wstring(bytesRefToInt(assoc)) + L")";
}
} // namespace org::apache::lucene::facet::taxonomy