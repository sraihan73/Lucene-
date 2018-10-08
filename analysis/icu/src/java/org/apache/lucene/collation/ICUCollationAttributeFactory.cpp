using namespace std;

#include "ICUCollationAttributeFactory.h"
#include "tokenattributes/ICUCollatedTermAttributeImpl.h"

namespace org::apache::lucene::collation
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ICUCollatedTermAttributeImpl = org::apache::lucene::collation::
    tokenattributes::ICUCollatedTermAttributeImpl;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using com::ibm::icu::text::Collator;

ICUCollationAttributeFactory::ICUCollationAttributeFactory(
    shared_ptr<Collator> collator)
    : ICUCollationAttributeFactory(TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                   collator)
{
}

ICUCollationAttributeFactory::ICUCollationAttributeFactory(
    shared_ptr<AttributeFactory> delegate_, shared_ptr<Collator> collator)
    : org::apache::lucene::util::AttributeFactory::
          StaticImplementationAttributeFactory<
              org::apache::lucene::collation::tokenattributes::
                  ICUCollatedTermAttributeImpl>(
              delegate_, ICUCollatedTermAttributeImpl::class),
      collator(collator)
{
}

shared_ptr<ICUCollatedTermAttributeImpl>
ICUCollationAttributeFactory::createInstance()
{
  return make_shared<ICUCollatedTermAttributeImpl>(collator);
}
} // namespace org::apache::lucene::collation