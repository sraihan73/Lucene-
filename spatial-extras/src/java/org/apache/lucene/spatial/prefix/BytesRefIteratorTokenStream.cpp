using namespace std;

#include "BytesRefIteratorTokenStream.h"

namespace org::apache::lucene::spatial::prefix
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

shared_ptr<BytesRefIterator> BytesRefIteratorTokenStream::getBytesRefIterator()
{
  return bytesIter;
}

shared_ptr<BytesRefIteratorTokenStream>
BytesRefIteratorTokenStream::setBytesRefIterator(
    shared_ptr<BytesRefIterator> iter)
{
  this->bytesIter = iter;
  return shared_from_this();
}

void BytesRefIteratorTokenStream::reset() 
{
  if (bytesIter == nullptr) {
    throw make_shared<IllegalStateException>(
        L"call setBytesRefIterator() before usage");
  }
}

bool BytesRefIteratorTokenStream::incrementToken() 
{
  if (bytesIter == nullptr) {
    throw make_shared<IllegalStateException>(
        L"call setBytesRefIterator() before usage");
  }

  // get next
  shared_ptr<BytesRef> bytes = bytesIter->next();
  if (bytes == nullptr) {
    return false;
  } else {
    clearAttributes();
    bytesAtt->setBytesRef(bytes);
    // note: we don't bother setting posInc or type attributes.  There's no
    // point to it.
    return true;
  }
}
} // namespace org::apache::lucene::spatial::prefix