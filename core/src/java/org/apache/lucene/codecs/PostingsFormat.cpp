using namespace std;

#include "PostingsFormat.h"
#include "../index/SegmentReadState.h"
#include "../index/SegmentWriteState.h"
#include "FieldsConsumer.h"
#include "FieldsProducer.h"

namespace org::apache::lucene::codecs
{
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;
const shared_ptr<
    org::apache::lucene::util::NamedSPILoader<std::shared_ptr<PostingsFormat>>>
    PostingsFormat::Holder::LOADER =
        make_shared<org::apache::lucene::util::NamedSPILoader<
            std::shared_ptr<PostingsFormat>>>(PostingsFormat::typeid);

PostingsFormat::Holder::Holder() {}

shared_ptr<NamedSPILoader<std::shared_ptr<PostingsFormat>>>
PostingsFormat::Holder::getLoader()
{
  if (LOADER->empty()) {
    throw make_shared<IllegalStateException>(
        wstring(L"You tried to lookup a PostingsFormat by name before all "
                L"formats could be initialized. ") +
        L"This likely happens if you call PostingsFormat#forName from a "
        L"PostingsFormat's ctor.");
  }
  return LOADER;
}

std::deque<std::shared_ptr<PostingsFormat>> const PostingsFormat::EMPTY =
    std::deque<std::shared_ptr<PostingsFormat>>(0);

PostingsFormat::PostingsFormat(const wstring &name) : name(name)
{
  // TODO: can we somehow detect name conflicts here?  Two different classes
  // trying to claim the same name?  Otherwise you see confusing errors...
  NamedSPILoader::checkServiceName(name);
}

wstring PostingsFormat::getName() { return name; }

wstring PostingsFormat::toString()
{
  return L"PostingsFormat(name=" + name + L")";
}

shared_ptr<PostingsFormat> PostingsFormat::forName(const wstring &name)
{
  return Holder::getLoader()->lookup(name);
}

shared_ptr<Set<wstring>> PostingsFormat::availablePostingsFormats()
{
  return Holder::getLoader()->availableServices();
}

void PostingsFormat::reloadPostingsFormats(shared_ptr<ClassLoader> classloader)
{
  Holder::getLoader()->reload(classloader);
}
} // namespace org::apache::lucene::codecs