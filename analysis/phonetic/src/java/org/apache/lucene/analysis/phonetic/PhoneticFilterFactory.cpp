using namespace std;

#include "PhoneticFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "PhoneticFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::Encoder;
using org::apache::commons::codec::language::Caverphone2;
using org::apache::commons::codec::language::ColognePhonetic;
using org::apache::commons::codec::language::DoubleMetaphone;
using org::apache::commons::codec::language::Metaphone;
using org::apache::commons::codec::language::Nysiis;
using org::apache::commons::codec::language::RefinedSoundex;
using org::apache::commons::codec::language::Soundex;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring PhoneticFilterFactory::ENCODER = L"encoder";
const wstring PhoneticFilterFactory::INJECT = L"inject";
const wstring PhoneticFilterFactory::MAX_CODE_LENGTH = L"maxCodeLength";
const wstring PhoneticFilterFactory::PACKAGE_CONTAINING_ENCODERS =
    L"org.apache.commons.codec.language.";
const unordered_map<wstring, type_info> PhoneticFilterFactory::registry =
    unordered_map<wstring, type_info>(6);

PhoneticFilterFactory::StaticConstructor::StaticConstructor()
{
  registry.emplace((wstring(L"DoubleMetaphone")).toUpperCase(Locale::ROOT),
                   DoubleMetaphone::typeid);
  registry.emplace((wstring(L"Metaphone")).toUpperCase(Locale::ROOT),
                   Metaphone::typeid);
  registry.emplace((wstring(L"Soundex")).toUpperCase(Locale::ROOT),
                   Soundex::typeid);
  registry.emplace((wstring(L"RefinedSoundex")).toUpperCase(Locale::ROOT),
                   RefinedSoundex::typeid);
  registry.emplace((wstring(L"Caverphone")).toUpperCase(Locale::ROOT),
                   Caverphone2::typeid);
  registry.emplace((wstring(L"ColognePhonetic")).toUpperCase(Locale::ROOT),
                   ColognePhonetic::typeid);
  registry.emplace((wstring(L"Nysiis")).toUpperCase(Locale::ROOT),
                   Nysiis::typeid);
}

PhoneticFilterFactory::StaticConstructor
    PhoneticFilterFactory::staticConstructor;

PhoneticFilterFactory::PhoneticFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      inject(getBoolean(args, INJECT, true)), name(require(args, ENCODER))
{
  wstring v = get(args, MAX_CODE_LENGTH);
  if (v != L"") {
    maxCodeLength = stoi(v);
  } else {
    maxCodeLength = nullopt;
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void PhoneticFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  clazz = registry[name.toUpperCase(Locale::ROOT)];
  if (clazz == nullptr) {
    clazz = resolveEncoder(name, loader);
  }

  if (maxCodeLength) {
    try {
      setMaxCodeLenMethod = clazz.getMethod(L"setMaxCodeLen", int ::typeid);
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new IllegalArgumentException("Encoder " + name + "
      // / " + clazz + " does not support " + MAX_CODE_LENGTH, e);
      throw invalid_argument(L"Encoder " + name + L" / " + clazz +
                             L" does not support " + MAX_CODE_LENGTH);
    }
  }

  getEncoder(); // trigger initialization for potential problems to be thrown
                // now
}

type_info
PhoneticFilterFactory::resolveEncoder(const wstring &name,
                                      shared_ptr<ResourceLoader> loader)
{
  wstring lookupName = name;
  if (name.find(L'.') == wstring::npos) {
    lookupName = PACKAGE_CONTAINING_ENCODERS + name;
  }
  try {
    return loader->newInstance(lookupName, Encoder::typeid)->getClass();
  } catch (const runtime_error &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Error loading encoder
    // '" + name + "': must be full class name or one of " + registry.keySet(),
    // e);
    throw invalid_argument(L"Error loading encoder '" + name +
                           L"': must be full class name or one of " +
                           registry.keySet());
  }
}

shared_ptr<Encoder> PhoneticFilterFactory::getEncoder()
{
  // Unfortunately, Commons-Codec doesn't offer any thread-safe guarantees so we
  // must play it safe and instantiate every time.  A simple benchmark showed
  // this as negligible.
  try {
    shared_ptr<Encoder> encoder = clazz.newInstance();
    // Try to set the maxCodeLength
    if (maxCodeLength && setMaxCodeLenMethod != nullptr) {
      setMaxCodeLenMethod->invoke(encoder, maxCodeLength);
    }
    return encoder;
  } catch (const runtime_error &e) {
    constexpr runtime_error t =
        (std::dynamic_pointer_cast<InvocationTargetException>(e) != nullptr)
            ? e.getCause()
            : e;
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Error initializing
    // encoder: " + name + " / " + clazz, t);
    throw invalid_argument(L"Error initializing encoder: " + name + L" / " +
                           clazz);
  }
}

shared_ptr<PhoneticFilter>
PhoneticFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<PhoneticFilter>(input, getEncoder(), inject);
}
} // namespace org::apache::lucene::analysis::phonetic