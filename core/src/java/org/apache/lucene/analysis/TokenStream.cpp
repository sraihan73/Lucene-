using namespace std;

#include "TokenStream.h"
#include "../util/AttributeFactory.h"

namespace org::apache::lucene::analysis
{
using PackedTokenAttributeImpl =
    org::apache::lucene::analysis::tokenattributes::PackedTokenAttributeImpl;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeSource = org::apache::lucene::util::AttributeSource;
const shared_ptr<org::apache::lucene::util::AttributeFactory>
    TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY =
        org::apache::lucene::util::AttributeFactory::getStaticImplementation(
            org::apache::lucene::util::AttributeFactory::
                DEFAULT_ATTRIBUTE_FACTORY,
            org::apache::lucene::analysis::tokenattributes::
                PackedTokenAttributeImpl::typeid);

TokenStream::TokenStream()
    : org::apache::lucene::util::AttributeSource(
          DEFAULT_TOKEN_ATTRIBUTE_FACTORY)
{
  assert(assertFinal());
}

TokenStream::TokenStream(shared_ptr<AttributeSource> input)
    : org::apache::lucene::util::AttributeSource(input)
{
  assert(assertFinal());
}

TokenStream::TokenStream(shared_ptr<AttributeFactory> factory)
    : org::apache::lucene::util::AttributeSource(factory)
{
  assert(assertFinal());
}

bool TokenStream::assertFinal()
{
  try {
    constexpr type_info clazz = getClass();
    if (!clazz.desiredAssertionStatus()) {
      return true;
    }
    assert((clazz.isAnonymousClass() ||
                (clazz.getModifiers() &
                 (Modifier::FINAL | Modifier::PRIVATE)) != 0 ||
                Modifier::isFinal(
                    clazz.getMethod(L"incrementToken").getModifiers()),
            L"TokenStream implementation classes or at least their "
            L"incrementToken() implementation must be final"));
    return true;
  } catch (const NoSuchMethodException &nsme) {
    return false;
  }
}

void TokenStream::end() 
{
  endAttributes(); // LUCENE-3849: don't consume dirty atts
}

void TokenStream::reset()  {}

TokenStream::~TokenStream() {}
} // namespace org::apache::lucene::analysis