using namespace std;

#include "TestRuleSetupAndRestoreClassEnv.h"

namespace org::apache::lucene::util
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using AssertingDocValuesFormat =
    org::apache::lucene::codecs::asserting::AssertingDocValuesFormat;
using AssertingPostingsFormat =
    org::apache::lucene::codecs::asserting::AssertingPostingsFormat;
using CheapBastardCodec =
    org::apache::lucene::codecs::cheapbastard::CheapBastardCodec;
using CompressingCodec =
    org::apache::lucene::codecs::compressing::CompressingCodec;
using Lucene50StoredFieldsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50StoredFieldsFormat;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using MockRandomPostingsFormat =
    org::apache::lucene::codecs::mockrandom::MockRandomPostingsFormat;
using SimpleTextCodec =
    org::apache::lucene::codecs::simpletext::SimpleTextCodec;
using RandomCodec = org::apache::lucene::index::RandomCodec;
using RandomSimilarity =
    org::apache::lucene::search::similarities::RandomSimilarity;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using com::carrotsearch::randomizedtesting::RandomizedContext;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;
using org::junit::internal_::AssumptionViolatedException;
//    import static org.apache.lucene.util.LuceneTestCase.INFOSTREAM;
//    import static org.apache.lucene.util.LuceneTestCase.LiveIWCFlushMode;
//    import static org.apache.lucene.util.LuceneTestCase.TEST_CODEC;
//    import static org.apache.lucene.util.LuceneTestCase.TEST_DOCVALUESFORMAT;
//    import static org.apache.lucene.util.LuceneTestCase.TEST_POSTINGSFORMAT;
//    import static org.apache.lucene.util.LuceneTestCase.VERBOSE;
//    import static org.apache.lucene.util.LuceneTestCase.assumeFalse;
//    import static org.apache.lucene.util.LuceneTestCase.localeForLanguageTag;
//    import static org.apache.lucene.util.LuceneTestCase.random;
//    import static org.apache.lucene.util.LuceneTestCase.randomLocale;
//    import static org.apache.lucene.util.LuceneTestCase.randomTimeZone;

TestRuleSetupAndRestoreClassEnv::ThreadNameFixingPrintStreamInfoStream::
    ThreadNameFixingPrintStreamInfoStream(shared_ptr<PrintStream> out)
    : PrintStreamInfoStream(out)
{
}

void TestRuleSetupAndRestoreClassEnv::ThreadNameFixingPrintStreamInfoStream::
    message(const wstring &component, const wstring &message)
{
  if (L"TP" == component) {
    return; // ignore test points!
  }
  const wstring name;
  if (Thread::currentThread().getName()->startsWith(L"TEST-")) {
    // The name of the main thread is way too
    // long when looking at IW verbose output...
    name = L"main";
  } else {
    name = Thread::currentThread().getName();
  }
  stream->println(component + L" " + to_wstring(messageID) + L" [" +
                  getTimestamp() + L"; " + name + L"]: " + message);
}

bool TestRuleSetupAndRestoreClassEnv::isInitialized() { return initialized; }

void TestRuleSetupAndRestoreClassEnv::before() 
{
  // enable this by default, for IDE consistency with ant tests (as it's the
  // default from ant)
  // TODO: really should be in solr base classes, but some extend LTC directly.
  // we do this in beforeClass, because some tests currently disable it
  if (System::getProperty(L"solr.directoryFactory") == nullptr) {
    System::setProperty(L"solr.directoryFactory",
                        L"org.apache.solr.core.MockDirectoryFactory");
  }

  // if verbose: print some debugging stuff about which codecs are loaded.
  if (VERBOSE) {
    wcout << L"Loaded codecs: " << Codec::availableCodecs() << endl;
    wcout << L"Loaded postingsFormats: "
          << PostingsFormat::availablePostingsFormats() << endl;
  }

  savedInfoStream = InfoStream::getDefault();
  shared_ptr<Random> *const random = RandomizedContext::current().getRandom();
  constexpr bool v = random->nextBoolean();
  if (INFOSTREAM) {
    InfoStream::setDefault(
        make_shared<ThreadNameFixingPrintStreamInfoStream>(System::out));
  } else if (v) {
    InfoStream::setDefault(make_shared<NullInfoStream>());
  }

  type_info targetClass = RandomizedContext::current().getTargetClass();
  avoidCodecs = unordered_set<wstring>();
  if (targetClass.isAnnotationPresent(SuppressCodecs::typeid)) {
    shared_ptr<SuppressCodecs> a =
        targetClass.getAnnotation(SuppressCodecs::typeid);
    avoidCodecs.addAll(Arrays::asList(a->value()));
  }

  savedCodec = Codec::getDefault();
  int randomVal = random->nextInt(11);
  if (L"default" == TEST_CODEC) {
    codec = savedCodec; // just use the default, don't randomize
  } else if ((L"random" == TEST_POSTINGSFORMAT == false) ||
             (L"random" == TEST_DOCVALUESFORMAT == false)) {
    // the user wired postings or DV: this is messy
    // refactor into RandomCodec....

    shared_ptr<PostingsFormat> *const format;
    if (L"random" == TEST_POSTINGSFORMAT) {
      format = make_shared<AssertingPostingsFormat>();
    } else if (L"MockRandom" == TEST_POSTINGSFORMAT) {
      format = make_shared<MockRandomPostingsFormat>(
          make_shared<Random>(random->nextLong()));
    } else {
      format = PostingsFormat::forName(TEST_POSTINGSFORMAT);
    }

    shared_ptr<DocValuesFormat> *const dvFormat;
    if (L"random" == TEST_DOCVALUESFORMAT) {
      dvFormat = make_shared<AssertingDocValuesFormat>();
    } else {
      dvFormat = DocValuesFormat::forName(TEST_DOCVALUESFORMAT);
    }

    codec = make_shared<AssertingCodecAnonymousInnerClass>(shared_from_this(),
                                                           format, dvFormat);
  } else if (L"SimpleText" == TEST_CODEC ||
             (L"random" == TEST_CODEC && randomVal == 9 &&
              LuceneTestCase::rarely(random) &&
              !shouldAvoidCodec(L"SimpleText"))) {
    codec = make_shared<SimpleTextCodec>();
  } else if (L"CheapBastard" == TEST_CODEC ||
             (L"random" == TEST_CODEC && randomVal == 8 &&
              !shouldAvoidCodec(L"CheapBastard") &&
              !shouldAvoidCodec(L"Lucene41"))) {
    // we also avoid this codec if Lucene41 is avoided, since thats the postings
    // format it uses.
    codec = make_shared<CheapBastardCodec>();
  } else if (L"Asserting" == TEST_CODEC ||
             (L"random" == TEST_CODEC && randomVal == 7 &&
              !shouldAvoidCodec(L"Asserting"))) {
    codec = make_shared<AssertingCodec>();
  } else if (L"Compressing" == TEST_CODEC ||
             (L"random" == TEST_CODEC && randomVal == 6 &&
              !shouldAvoidCodec(L"Compressing"))) {
    codec = CompressingCodec::randomInstance(random);
  } else if (L"Lucene70" == TEST_CODEC ||
             (L"random" == TEST_CODEC && randomVal == 5 &&
              !shouldAvoidCodec(L"Lucene70"))) {
    codec = make_shared<Lucene70Codec>(RandomPicks::randomFrom(
        random, Lucene50StoredFieldsFormat::Mode::values()));
  } else if (L"random" != TEST_CODEC) {
    codec = Codec::forName(TEST_CODEC);
  } else if (L"random" == TEST_POSTINGSFORMAT) {
    codec = make_shared<RandomCodec>(random, avoidCodecs);
  } else {
    assert(false);
  }
  Codec::setDefault(codec);

  // Initialize locale/ timezone.
  wstring testLocale = System::getProperty(L"tests.locale", L"random");
  wstring testTimeZone = System::getProperty(L"tests.timezone", L"random");

  // Always pick a random one for consistency (whether tests.locale was
  // specified or not).
  savedLocale = Locale::getDefault();
  shared_ptr<Locale> randomLocale = randomLocale(random);
  locale =
      testLocale == L"random" ? randomLocale : localeForLanguageTag(testLocale);
  Locale::setDefault(locale);

  savedTimeZone = TimeZone::getDefault();
  shared_ptr<TimeZone> randomTimeZone = randomTimeZone(random());
  timeZone = testTimeZone == L"random" ? randomTimeZone
                                       : TimeZone::getTimeZone(testTimeZone);
  TimeZone::setDefault(timeZone);
  similarity = make_shared<RandomSimilarity>(random());

  // Check codec restrictions once at class level.
  try {
    checkCodecRestrictions(codec);
  } catch (const AssumptionViolatedException &e) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    System::err::println(L"NOTE: " + e->getMessage() + L" Suppressed codecs: " +
                         Arrays->toString(avoidCodecs.toArray()));
    throw e;
  }

  // We have "stickiness" so that sometimes all we do is vary the RAM buffer
  // size, other times just the doc count to flush by, else both. This way the
  // assertMemory in DocumentsWriterFlushControl sometimes runs (when we always
  // flush by RAM).
  LiveIWCFlushMode flushMode;
  switch (random().nextInt(3)) {
  case 0:
    flushMode = LiveIWCFlushMode::BY_RAM;
    break;
  case 1:
    flushMode = LiveIWCFlushMode::BY_DOCS;
    break;
  case 2:
    flushMode = LiveIWCFlushMode::EITHER;
    break;
  default:
    throw make_shared<AssertionError>();
  }

  LuceneTestCase::setLiveIWCFlushMode(flushMode);

  initialized = true;
}

TestRuleSetupAndRestoreClassEnv::AssertingCodecAnonymousInnerClass::
    AssertingCodecAnonymousInnerClass(
        shared_ptr<TestRuleSetupAndRestoreClassEnv> outerInstance,
        shared_ptr<PostingsFormat> format, shared_ptr<DocValuesFormat> dvFormat)
{
  this->outerInstance = outerInstance;
  this->format = format;
  this->dvFormat = dvFormat;
}

shared_ptr<PostingsFormat>
TestRuleSetupAndRestoreClassEnv::AssertingCodecAnonymousInnerClass::
    getPostingsFormatForField(const wstring &field)
{
  return format;
}

shared_ptr<DocValuesFormat>
TestRuleSetupAndRestoreClassEnv::AssertingCodecAnonymousInnerClass::
    getDocValuesFormatForField(const wstring &field)
{
  return dvFormat;
}

wstring
TestRuleSetupAndRestoreClassEnv::AssertingCodecAnonymousInnerClass::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return outerInstance->super->toString() + L": " + format->toString() + L", " +
         dvFormat->toString();
}

void TestRuleSetupAndRestoreClassEnv::checkCodecRestrictions(
    shared_ptr<Codec> codec)
{
  assumeFalse(L"Class not allowed to use codec: " + codec->getName() + L".",
              shouldAvoidCodec(codec->getName()));

  if (std::dynamic_pointer_cast<RandomCodec>(codec) != nullptr &&
      !avoidCodecs.empty()) {
    for (auto name :
         (std::static_pointer_cast<RandomCodec>(codec))->formatNames) {
      assumeFalse(L"Class not allowed to use postings format: " + name + L".",
                  shouldAvoidCodec(name));
    }
  }

  shared_ptr<PostingsFormat> pf = codec->postingsFormat();
  assumeFalse(L"Class not allowed to use postings format: " + pf->getName() +
                  L".",
              shouldAvoidCodec(pf->getName()));

  assumeFalse(L"Class not allowed to use postings format: " +
                  LuceneTestCase::TEST_POSTINGSFORMAT + L".",
              shouldAvoidCodec(LuceneTestCase::TEST_POSTINGSFORMAT));
}

void TestRuleSetupAndRestoreClassEnv::after() 
{
  Codec::setDefault(savedCodec);
  InfoStream::setDefault(savedInfoStream);
  if (savedLocale != nullptr) {
    Locale::setDefault(savedLocale);
  }
  if (savedTimeZone != nullptr) {
    TimeZone::setDefault(savedTimeZone);
  }
}

bool TestRuleSetupAndRestoreClassEnv::shouldAvoidCodec(const wstring &codec)
{
  return !avoidCodecs.empty() && find(avoidCodecs.begin(), avoidCodecs.end(),
                                      codec) != avoidCodecs.end();
}
} // namespace org::apache::lucene::util