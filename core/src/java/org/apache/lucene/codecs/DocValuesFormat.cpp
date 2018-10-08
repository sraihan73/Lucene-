using namespace std;

#include "DocValuesFormat.h"
#include "../index/SegmentReadState.h"
#include "../index/SegmentWriteState.h"
#include "DocValuesConsumer.h"
#include "DocValuesProducer.h"

namespace org::apache::lucene::codecs
{
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;
const shared_ptr<
    org::apache::lucene::util::NamedSPILoader<std::shared_ptr<DocValuesFormat>>>
    DocValuesFormat::Holder::LOADER =
        make_shared<org::apache::lucene::util::NamedSPILoader<
            std::shared_ptr<DocValuesFormat>>>(DocValuesFormat::typeid);

DocValuesFormat::Holder::Holder() {}

shared_ptr<NamedSPILoader<std::shared_ptr<DocValuesFormat>>>
DocValuesFormat::Holder::getLoader()
{
  if (LOADER->empty()) {
    throw make_shared<IllegalStateException>(
        wstring(L"You tried to lookup a DocValuesFormat by name before all "
                L"formats could be initialized. ") +
        L"This likely happens if you call DocValuesFormat#forName from a "
        L"DocValuesFormat's ctor.");
  }
  return LOADER;
}

DocValuesFormat::DocValuesFormat(const wstring &name) : name(name)
{
  NamedSPILoader::checkServiceName(name);
}

wstring DocValuesFormat::getName() { return name; }

wstring DocValuesFormat::toString()
{
  return L"DocValuesFormat(name=" + name + L")";
}

shared_ptr<DocValuesFormat> DocValuesFormat::forName(const wstring &name)
{
  return Holder::getLoader()->lookup(name);
}

shared_ptr<Set<wstring>> DocValuesFormat::availableDocValuesFormats()
{
  return Holder::getLoader()->availableServices();
}

void DocValuesFormat::reloadDocValuesFormats(
    shared_ptr<ClassLoader> classloader)
{
  Holder::getLoader()->reload(classloader);
}
} // namespace org::apache::lucene::codecs