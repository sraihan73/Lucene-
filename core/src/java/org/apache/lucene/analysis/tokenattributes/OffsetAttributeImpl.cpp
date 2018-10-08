using namespace std;

#include "OffsetAttributeImpl.h"
#include "../../util/AttributeReflector.h"

namespace org::apache::lucene::analysis::tokenattributes
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

OffsetAttributeImpl::OffsetAttributeImpl() {}

int OffsetAttributeImpl::startOffset() { return startOffset_; }

void OffsetAttributeImpl::setOffset(int startOffset, int endOffset)
{

  // TODO: we could assert that this is set-once, ie,
  // current values are -1?  Very few token filters should
  // change offsets once set by the tokenizer... and
  // tokenizer should call clearAtts before re-using
  // OffsetAtt

  if (startOffset < 0 || endOffset < startOffset) {
    throw invalid_argument(wstring(L"startOffset must be non-negative, and "
                                   L"endOffset must be >= startOffset; got ") +
                           L"startOffset=" + to_wstring(startOffset) +
                           L",endOffset=" + to_wstring(endOffset));
  }

  this->startOffset_ = startOffset;
  this->endOffset_ = endOffset;
}

int OffsetAttributeImpl::endOffset() { return endOffset_; }

void OffsetAttributeImpl::clear()
{
  // TODO: we could use -1 as default here?  Then we can
  // assert in setOffset...
  startOffset_ = 0;
  endOffset_ = 0;
}

bool OffsetAttributeImpl::equals(any other)
{
  if (other == shared_from_this()) {
    return true;
  }

  if (std::dynamic_pointer_cast<OffsetAttributeImpl>(other) != nullptr) {
    shared_ptr<OffsetAttributeImpl> o =
        any_cast<std::shared_ptr<OffsetAttributeImpl>>(other);
    return o->startOffset_ == startOffset_ && o->endOffset_ == endOffset_;
  }

  return false;
}

int OffsetAttributeImpl::hashCode()
{
  int code = startOffset_;
  code = code * 31 + endOffset_;
  return code;
}

void OffsetAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  shared_ptr<OffsetAttribute> t =
      std::static_pointer_cast<OffsetAttribute>(target);
  t->setOffset(startOffset_, endOffset_);
}

void OffsetAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(OffsetAttribute::typeid, L"startOffset", startOffset_);
  reflector(OffsetAttribute::typeid, L"endOffset", endOffset_);
}
} // namespace org::apache::lucene::analysis::tokenattributes