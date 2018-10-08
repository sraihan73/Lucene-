using namespace std;

#include "NormalizeCharMap.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/CharSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"

namespace org::apache::lucene::analysis::charfilter
{
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using CharSequenceOutputs = org::apache::lucene::util::fst::CharSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;
using Util = org::apache::lucene::util::fst::Util;

NormalizeCharMap::NormalizeCharMap(
    shared_ptr<FST<std::shared_ptr<CharsRef>>> map_obj)
    : map_obj(map_obj)
{
  if (map_obj != nullptr) {
    try {
      // Pre-cache root arcs:
      shared_ptr<FST::Arc<std::shared_ptr<CharsRef>>> *const scratchArc =
          make_shared<FST::Arc<std::shared_ptr<CharsRef>>>();
      shared_ptr<FST::BytesReader> *const fstReader = map_obj->getBytesReader();
      map_obj->getFirstArc(scratchArc);
      if (FST::targetHasArcs(scratchArc)) {
        map_obj->readFirstRealTargetArc(scratchArc->target, scratchArc, fstReader);
        while (true) {
          assert(scratchArc->label != FST::END_LABEL);
          cachedRootArcs.emplace(
              static_cast<Character>(static_cast<wchar_t>(scratchArc->label)),
              (make_shared<FST::Arc<std::shared_ptr<CharsRef>>>())
                  ->copyFrom(scratchArc));
          if (scratchArc->isLast()) {
            break;
          }
          map_obj->readNextRealArc(scratchArc, fstReader);
        }
      }
      // System.out.println("cached " + cachedRootArcs.size() + " root arcs");
    } catch (const IOException &ioe) {
      // Bogus FST IOExceptions!!  (will never happen)
      throw runtime_error(ioe);
    }
  }
}

void NormalizeCharMap::Builder::add(const wstring &match,
                                    const wstring &replacement)
{
  if (match.length() == 0) {
    throw invalid_argument(L"cannot match the empty string");
  }
  if (pendingPairs.find(match) != pendingPairs.end()) {
    throw invalid_argument(L"match \"" + match + L"\" was already added");
  }
  pendingPairs.emplace(match, replacement);
}

shared_ptr<NormalizeCharMap> NormalizeCharMap::Builder::build()
{

  shared_ptr<FST<std::shared_ptr<CharsRef>>> *const map_obj;
  try {
    shared_ptr<Outputs<std::shared_ptr<CharsRef>>> *const outputs =
        CharSequenceOutputs::getSingleton();
    shared_ptr<
        org::apache::lucene::util::fst::Builder<std::shared_ptr<CharsRef>>>
        *const builder = make_shared<
            org::apache::lucene::util::fst::Builder<std::shared_ptr<CharsRef>>>(
            FST::INPUT_TYPE::BYTE2, outputs);
    shared_ptr<IntsRefBuilder> *const scratch = make_shared<IntsRefBuilder>();
    for (auto ent : pendingPairs) {
      builder->add(Util::toUTF16(ent.first, scratch),
                   make_shared<CharsRef>(ent.second));
    }
    map_obj = builder->finish();
    pendingPairs.clear();
  } catch (const IOException &ioe) {
    // Bogus FST IOExceptions!!  (will never happen)
    throw runtime_error(ioe);
  }

  return make_shared<NormalizeCharMap>(map_obj);
}
} // namespace org::apache::lucene::analysis::charfilter