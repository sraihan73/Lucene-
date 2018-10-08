using namespace std;

#include "TermsEnum.h"

namespace org::apache::lucene::index
{
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

TermsEnum::TermsEnum() {}

shared_ptr<AttributeSource> TermsEnum::attributes()
{
  if (atts == nullptr) {
    atts = make_shared<AttributeSource>();
  }
  return atts;
}

bool TermsEnum::seekExact(shared_ptr<BytesRef> text) 
{
  return seekCeil(text) == SeekStatus::FOUND;
}

void TermsEnum::seekExact(shared_ptr<BytesRef> term,
                          shared_ptr<TermState> state) 
{
  if (!seekExact(term)) {
    throw invalid_argument(L"term=" + term + L" does not exist");
  }
}

shared_ptr<PostingsEnum>
TermsEnum::postings(shared_ptr<PostingsEnum> reuse) 
{
  return postings(reuse, PostingsEnum::FREQS);
}

shared_ptr<TermState> TermsEnum::termState() 
{
  return make_shared<TermStateAnonymousInnerClass>(shared_from_this());
}

TermsEnum::TermStateAnonymousInnerClass::TermStateAnonymousInnerClass(
    shared_ptr<TermsEnum> outerInstance)
{
  this->outerInstance = outerInstance;
}

void TermsEnum::TermStateAnonymousInnerClass::copyFrom(
    shared_ptr<TermState> other)
{
  throw make_shared<UnsupportedOperationException>();
}

const shared_ptr<TermsEnum> TermsEnum::EMPTY =
    make_shared<TermsEnumAnonymousInnerClass>();

TermsEnum::TermsEnumAnonymousInnerClass::TermsEnumAnonymousInnerClass() {}

SeekStatus
TermsEnum::TermsEnumAnonymousInnerClass::seekCeil(shared_ptr<BytesRef> term)
{
  return SeekStatus::END;
}

void TermsEnum::TermsEnumAnonymousInnerClass::seekExact(int64_t ord) {}

shared_ptr<BytesRef> TermsEnum::TermsEnumAnonymousInnerClass::term()
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

int TermsEnum::TermsEnumAnonymousInnerClass::docFreq()
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

int64_t TermsEnum::TermsEnumAnonymousInnerClass::totalTermFreq()
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

int64_t TermsEnum::TermsEnumAnonymousInnerClass::ord()
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

shared_ptr<PostingsEnum> TermsEnum::TermsEnumAnonymousInnerClass::postings(
    shared_ptr<PostingsEnum> reuse, int flags)
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

shared_ptr<BytesRef> TermsEnum::TermsEnumAnonymousInnerClass::next()
{
  return nullptr;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<AttributeSource>
TermsEnum::TermsEnumAnonymousInnerClass::attributes()
{
  return TermsEnum::attributes();
}

shared_ptr<TermState> TermsEnum::TermsEnumAnonymousInnerClass::termState()
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}

void TermsEnum::TermsEnumAnonymousInnerClass::seekExact(
    shared_ptr<BytesRef> term, shared_ptr<TermState> state)
{
  throw make_shared<IllegalStateException>(
      L"this method should never be called");
}
} // namespace org::apache::lucene::index