using namespace std;

#include "MockUTF16TermAttributeImpl.h"

namespace org::apache::lucene::analysis
{
using CharTermAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::CharTermAttributeImpl;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<org::apache::lucene::util::AttributeFactory>
    MockUTF16TermAttributeImpl::UTF16_TERM_ATTRIBUTE_FACTORY =
        org::apache::lucene::util::AttributeFactory::getStaticImplementation(
            org::apache::lucene::util::AttributeFactory::
                DEFAULT_ATTRIBUTE_FACTORY,
            MockUTF16TermAttributeImpl::typeid);

shared_ptr<BytesRef> MockUTF16TermAttributeImpl::getBytesRef()
{
  shared_ptr<BytesRef> *const ref = this->builder->get();
  ref->bytes = toString().getBytes(StandardCharsets::UTF_16LE);
  ref->offset = 0;
  ref->length = ref->bytes.size();
  return ref;
}
} // namespace org::apache::lucene::analysis