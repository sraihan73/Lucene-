using namespace std;

#include "Term.h"

namespace org::apache::lucene::index
{
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

Term::Term(const wstring &fld, shared_ptr<BytesRef> bytes)
{
  field_ = fld;
  this->bytes_ = bytes == nullptr ? nullptr : BytesRef::deepCopyOf(bytes);
}

Term::Term(const wstring &fld, shared_ptr<BytesRefBuilder> bytesBuilder)
{
  field_ = fld;
  this->bytes_ = bytesBuilder->toBytesRef();
}

Term::Term(const wstring &fld, const wstring &text)
    : Term(fld, new BytesRef(text))
{
}

Term::Term(const wstring &fld) : Term(fld, new BytesRef()) {}

wstring Term::field() { return field_; }

wstring Term::text() { return toString(bytes_); }

wstring Term::toString(shared_ptr<BytesRef> termText)
{
  // the term might not be text, but usually is. so we make a best effort
  shared_ptr<CharsetDecoder> decoder =
      StandardCharsets::UTF_8::newDecoder()
          .onMalformedInput(CodingErrorAction::REPORT)
          .onUnmappableCharacter(CodingErrorAction::REPORT);
  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return decoder
        ->decode(ByteBuffer::wrap(termText->bytes, termText->offset,
                                  termText->length))
        ->toString();
  } catch (const CharacterCodingException &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return termText->toString();
  }
}

shared_ptr<BytesRef> Term::bytes() { return bytes_; }

bool Term::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<Term> other = any_cast<std::shared_ptr<Term>>(obj);
  if (field_ == L"") {
    if (other->field_ != L"") {
      return false;
    }
  } else if (field_ != other->field_) {
    return false;
  }
  if (bytes_ == nullptr) {
    if (other->bytes_ != nullptr) {
      return false;
    }
  } else if (!bytes_->equals(other->bytes_)) {
    return false;
  }
  return true;
}

int Term::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + ((field_ == L"") ? 0 : field_.hashCode());
  result = prime * result + ((bytes_ == nullptr) ? 0 : bytes_->hashCode());
  return result;
}

int Term::compareTo(shared_ptr<Term> other)
{
  if (field_ == other->field_) {
    return bytes_->compareTo(other->bytes_);
  } else {
    return field_.compare(other->field_);
  }
}

void Term::set(const wstring &fld, shared_ptr<BytesRef> bytes)
{
  field_ = fld;
  this->bytes_ = bytes;
}

wstring Term::toString() { return field_ + L":" + text(); }
} // namespace org::apache::lucene::index