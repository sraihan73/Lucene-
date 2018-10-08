using namespace std;

#include "CollationAttributeFactory.h"
#include "tokenattributes/CollatedTermAttributeImpl.h"

namespace org::apache::lucene::collation
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CollatedTermAttributeImpl =
    org::apache::lucene::collation::tokenattributes::CollatedTermAttributeImpl;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

CollationAttributeFactory::CollationAttributeFactory(
    shared_ptr<Collator> collator)
    : CollationAttributeFactory(TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                collator)
{
}

CollationAttributeFactory::CollationAttributeFactory(
    shared_ptr<AttributeFactory> delegate_, shared_ptr<Collator> collator)
    : org::apache::lucene::util::AttributeFactory::
          StaticImplementationAttributeFactory<
              org::apache::lucene::collation::tokenattributes::
                  CollatedTermAttributeImpl>(delegate_,
                                             CollatedTermAttributeImpl::class),
      collator(collator)
{
}

shared_ptr<CollatedTermAttributeImpl>
CollationAttributeFactory::createInstance()
{
  return make_shared<CollatedTermAttributeImpl>(collator);
}
} // namespace org::apache::lucene::collation