using namespace std;

#include "MorphosyntacticTagsAttributeImpl.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeReflector.h"

namespace org::apache::lucene::analysis::morfologik
{
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

MorphosyntacticTagsAttributeImpl::MorphosyntacticTagsAttributeImpl() {}

deque<std::shared_ptr<StringBuilder>>
MorphosyntacticTagsAttributeImpl::getTags()
{
  return tags;
}

void MorphosyntacticTagsAttributeImpl::clear() { tags.clear(); }

bool MorphosyntacticTagsAttributeImpl::equals(any other)
{
  if (std::dynamic_pointer_cast<MorphosyntacticTagsAttribute>(other) !=
      nullptr) {
    return equal(
        this->getTags(),
        (any_cast<std::shared_ptr<MorphosyntacticTagsAttribute>>(other))
            .getTags());
  }
  return false;
}

bool MorphosyntacticTagsAttributeImpl::equal(any l1, any l2)
{
  return l1 == nullptr ? (l2 == nullptr) : (l1.equals(l2));
}

int MorphosyntacticTagsAttributeImpl::hashCode()
{
  return this->tags.empty() ? 0 : tags.hashCode();
}

void MorphosyntacticTagsAttributeImpl::setTags(
    deque<std::shared_ptr<StringBuilder>> &tags)
{
  this->tags = tags;
}

void MorphosyntacticTagsAttributeImpl::copyTo(shared_ptr<AttributeImpl> target)
{
  deque<std::shared_ptr<StringBuilder>> cloned;
  if (tags.size() > 0) {
    cloned = deque<>(tags.size());
    for (auto b : tags) {
      cloned.push_back(make_shared<StringBuilder>(b));
    }
  }
  (std::static_pointer_cast<MorphosyntacticTagsAttribute>(target))
      ->setTags(cloned);
}

shared_ptr<MorphosyntacticTagsAttributeImpl>
MorphosyntacticTagsAttributeImpl::clone()
{
  shared_ptr<MorphosyntacticTagsAttributeImpl> cloned =
      make_shared<MorphosyntacticTagsAttributeImpl>();
  this->copyTo(cloned);
  return cloned;
}

void MorphosyntacticTagsAttributeImpl::reflectWith(AttributeReflector reflector)
{
  reflector(MorphosyntacticTagsAttribute::typeid, L"tags", tags);
}
} // namespace org::apache::lucene::analysis::morfologik