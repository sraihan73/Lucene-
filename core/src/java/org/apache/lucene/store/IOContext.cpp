using namespace std;

#include "IOContext.h"

namespace org::apache::lucene::store
{

const shared_ptr<IOContext> IOContext::DEFAULT =
    make_shared<IOContext>(Context::DEFAULT);
const shared_ptr<IOContext> IOContext::READONCE = make_shared<IOContext>(true);
const shared_ptr<IOContext> IOContext::READ = make_shared<IOContext>(false);

IOContext::IOContext() : IOContext(false) {}

IOContext::IOContext(shared_ptr<FlushInfo> flushInfo)
    : context(Context::FLUSH), mergeInfo(this->mergeInfo.reset()),
      flushInfo(flushInfo), readOnce(false)
{
  assert(flushInfo != nullptr);
}

IOContext::IOContext(Context context) : IOContext(context, nullptr) {}

IOContext::IOContext(bool readOnce)
    : context(Context::READ), mergeInfo(this->mergeInfo.reset()),
      flushInfo(this->flushInfo.reset()), readOnce(readOnce)
{
}

IOContext::IOContext(shared_ptr<MergeInfo> mergeInfo)
    : IOContext(Context::MERGE, mergeInfo)
{
}

IOContext::IOContext(Context context, shared_ptr<MergeInfo> mergeInfo)
    : context(context), mergeInfo(mergeInfo),
      flushInfo(this->flushInfo.reset()), readOnce(false)
{
  assert((context != Context::MERGE || mergeInfo != nullptr,
          L"MergeInfo must not be null if context is MERGE"));
  assert((context != Context::FLUSH,
          L"Use IOContext(FlushInfo) to create a FLUSH IOContext"));
}

IOContext::IOContext(shared_ptr<IOContext> ctxt, bool readOnce)
    : context(ctxt->context), mergeInfo(ctxt->mergeInfo),
      flushInfo(ctxt->flushInfo), readOnce(readOnce)
{
}

int IOContext::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + ((context == nullptr) ? 0 : context.hashCode());
  result =
      prime * result + ((flushInfo == nullptr) ? 0 : flushInfo->hashCode());
  result =
      prime * result + ((mergeInfo == nullptr) ? 0 : mergeInfo->hashCode());
  result = prime * result + (readOnce ? 1231 : 1237);
  return result;
}

bool IOContext::equals(any obj)
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
  shared_ptr<IOContext> other = any_cast<std::shared_ptr<IOContext>>(obj);
  if (context != other->context) {
    return false;
  }
  if (flushInfo == nullptr) {
    if (other->flushInfo != nullptr) {
      return false;
    }
  } else if (!flushInfo->equals(other->flushInfo)) {
    return false;
  }
  if (mergeInfo == nullptr) {
    if (other->mergeInfo != nullptr) {
      return false;
    }
  } else if (!mergeInfo->equals(other->mergeInfo)) {
    return false;
  }
  if (readOnce != other->readOnce) {
    return false;
  }
  return true;
}

wstring IOContext::toString()
{
  return L"IOContext [context=" + context + L", mergeInfo=" + mergeInfo +
         L", flushInfo=" + flushInfo + L", readOnce=" +
         StringHelper::toString(readOnce) + L"]";
}
} // namespace org::apache::lucene::store