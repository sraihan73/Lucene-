using namespace std;

#include "Codec.h"
#include "CompoundFormat.h"
#include "DocValuesFormat.h"
#include "FieldInfosFormat.h"
#include "LiveDocsFormat.h"
#include "NormsFormat.h"
#include "PointsFormat.h"
#include "PostingsFormat.h"
#include "SegmentInfoFormat.h"
#include "StoredFieldsFormat.h"
#include "TermVectorsFormat.h"

namespace org::apache::lucene::codecs
{
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using NamedSPILoader = org::apache::lucene::util::NamedSPILoader;
const shared_ptr<
    org::apache::lucene::util::NamedSPILoader<std::shared_ptr<Codec>>>
    Codec::Holder::LOADER = make_shared<
        org::apache::lucene::util::NamedSPILoader<std::shared_ptr<Codec>>>(
        Codec::typeid);

Codec::Holder::Holder() {}

shared_ptr<NamedSPILoader<std::shared_ptr<Codec>>> Codec::Holder::getLoader()
{
  if (LOADER->empty()) {
    throw make_shared<IllegalStateException>(
        wstring(L"You tried to lookup a Codec by name before all Codecs could "
                L"be initialized. ") +
        L"This likely happens if you call Codec#forName from a Codec's ctor.");
  }
  return LOADER;
}

shared_ptr<Codec> Codec::Holder::defaultCodec = LOADER->lookup(L"Lucene70");

Codec::Codec(const wstring &name) : name(name)
{
  NamedSPILoader::checkServiceName(name);
}

wstring Codec::getName() { return name; }

shared_ptr<Codec> Codec::forName(const wstring &name)
{
  return Holder::getLoader()->lookup(name);
}

shared_ptr<Set<wstring>> Codec::availableCodecs()
{
  return Holder::getLoader()->availableServices();
}

void Codec::reloadCodecs(shared_ptr<ClassLoader> classloader)
{
  Holder::getLoader()->reload(classloader);
}

shared_ptr<Codec> Codec::getDefault()
{
  if (Holder::defaultCodec == nullptr) {
    throw make_shared<IllegalStateException>(
        wstring(L"You tried to lookup the default Codec before all Codecs "
                L"could be initialized. ") +
        L"This likely happens if you try to get it from a Codec's ctor.");
  }
  return Holder::defaultCodec;
}

void Codec::setDefault(shared_ptr<Codec> codec)
{
  Holder::defaultCodec = Objects::requireNonNull(codec);
}

wstring Codec::toString() { return name; }
} // namespace org::apache::lucene::codecs