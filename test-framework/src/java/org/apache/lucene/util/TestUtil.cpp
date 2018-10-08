using namespace std;

#include "TestUtil.h"

namespace org::apache::lucene::util
{
using Codec = org::apache::lucene::codecs::Codec;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using LuceneFixedGap = org::apache::lucene::codecs::blockterms::LuceneFixedGap;
using BlockTreeOrdsPostingsFormat =
    org::apache::lucene::codecs::blocktreeords::BlockTreeOrdsPostingsFormat;
using Lucene50PostingsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using Lucene70DocValuesFormat =
    org::apache::lucene::codecs::lucene70::Lucene70DocValuesFormat;
using PerFieldDocValuesFormat =
    org::apache::lucene::codecs::perfield::PerFieldDocValuesFormat;
using PerFieldPostingsFormat =
    org::apache::lucene::codecs::perfield::PerFieldPostingsFormat;
using BinaryDocValuesField =
    org::apache::lucene::document::BinaryDocValuesField;
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using CheckIndex = org::apache::lucene::index::CheckIndex;
using CodecReader = org::apache::lucene::index::CodecReader;
using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexableField = org::apache::lucene::index::IndexableField;
using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using MultiFields = org::apache::lucene::index::MultiFields;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using SegmentReader = org::apache::lucene::index::SegmentReader;
using SlowCodecReaderWrapper =
    org::apache::lucene::index::SlowCodecReaderWrapper;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using TieredMergePolicy = org::apache::lucene::index::TieredMergePolicy;
using FilterFileSystem = org::apache::lucene::mockfile::FilterFileSystem;
using VirusCheckingFS = org::apache::lucene::mockfile::VirusCheckingFS;
using WindowsFS = org::apache::lucene::mockfile::WindowsFS;
using FieldDoc = org::apache::lucene::search::FieldDoc;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using FilterDirectory = org::apache::lucene::store::FilterDirectory;
using IOContext = org::apache::lucene::store::IOContext;
using NoLockFactory = org::apache::lucene::store::NoLockFactory;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using com::carrotsearch::randomizedtesting::generators::RandomNumbers;
using com::carrotsearch::randomizedtesting::generators::RandomPicks;
using org::junit::Assert;

TestUtil::TestUtil()
{
  //
}

const shared_ptr<java::util::Comparator<std::shared_ptr<std::wstring>>>
    TestUtil::STRING_CODEPOINT_COMPARATOR = [&](a, b) {
      const std::deque<int> aCodePoints = a::codePoints().toArray();
      const std::deque<int> bCodePoints = b::codePoints().toArray();
      for (int i = 0, c = min(aCodePoints.length, bCodePoints.length); i < c;
           i++) {
        if (aCodePoints[i] < bCodePoints[i]) {
          return -1;
        } else if (aCodePoints[i] > bCodePoints[i]) {
          return 1;
        }
      }
      return aCodePoints.length - bCodePoints.length;
    };

void TestUtil::unzip(shared_ptr<InputStream> in_,
                     shared_ptr<Path> destDir) 
{
  in_ = make_shared<BufferedInputStream>(in_);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.util.zip.ZipInputStream zipInput = new
  // java.util.zip.ZipInputStream(in))
  {
    java::util::zip::ZipInputStream zipInput =
        java::util::zip::ZipInputStream(in_);
    shared_ptr<ZipEntry> entry;
    std::deque<char> buffer(8192);
    while ((entry = zipInput.getNextEntry()) != nullptr) {
      shared_ptr<Path> targetFile = destDir->resolve(entry->getName());

      // be on the safe side: do not rely on that directories are always
      // extracted before their children (although this makes sense, but is it
      // guaranteed?)
      Files::createDirectories(targetFile->getParent());
      if (!entry->isDirectory()) {
        shared_ptr<OutputStream> out = Files::newOutputStream(targetFile);
        int len;
        while ((len = zipInput.read(buffer)) >= 0) {
          out->write(buffer, 0, len);
        }
        out->close();
      }
      zipInput.closeEntry();
    }
  }
}

template <typename T>
void TestUtil::checkIterator(shared_ptr<Iterator<T>> iterator,
                             int64_t expectedSize, bool allowNull)
{
  for (int64_t i = 0; i < expectedSize; i++) {
    bool hasNext = iterator->hasNext();
    assert(hasNext);
    T v = iterator->next();
    assert(allowNull || v != nullptr);
    // for the first element, check that remove is not supported
    if (i == 0) {
      try {
        iterator->remove();
        throw make_shared<AssertionError>(
            L"broken iterator (supports remove): " + iterator);
      } catch (const UnsupportedOperationException &expected) {
        // ok
      }
    }
  }
  assert(!iterator->hasNext());
  try {
    iterator->next();
    throw make_shared<AssertionError>(
        L"broken iterator (allows next() when hasNext==false) " + iterator);
  } catch (const NoSuchElementException &expected) {
    // ok
  }
}

template <typename T>
void TestUtil::checkIterator(shared_ptr<Iterator<T>> iterator)
{
  while (iterator->hasNext()) {
    T v = iterator->next();
    assert(v != nullptr);
    try {
      iterator->remove();
      throw make_shared<AssertionError>(L"broken iterator (supports remove): " +
                                        iterator);
    } catch (const UnsupportedOperationException &expected) {
      // ok
    }
    iterator++;
  }
  try {
    iterator->next();
    throw make_shared<AssertionError>(
        L"broken iterator (allows next() when hasNext==false) " + iterator);
  } catch (const NoSuchElementException &expected) {
    // ok
  }
}

template <typename T>
void TestUtil::checkReadOnly(shared_ptr<deque<T>> coll)
{
  int size = 0;
  for (shared_ptr<deque<T>::const_iterator> it = coll->begin();
       it != coll->end(); ++it) {
    *it;
    size += 1;
  }
  if (size != coll->size()) {
    throw make_shared<AssertionError>(L"broken collection, reported size is " +
                                      coll->size() + L" but iterator has " +
                                      to_wstring(size) + L" elements: " + coll);
  }

  if (coll->isEmpty() == false) {
    try {
      coll->remove(coll->begin()->next());
      throw make_shared<AssertionError>(
          L"broken collection (supports remove): " + coll);
    } catch (const UnsupportedOperationException &e) {
      // ok
    }
  }

  try {
    coll->add(nullptr);
    throw make_shared<AssertionError>(L"broken collection (supports add): " +
                                      coll);
  } catch (const UnsupportedOperationException &e) {
    // ok
  }

  try {
    coll->addAll(Collections::singleton(nullptr));
    throw make_shared<AssertionError>(L"broken collection (supports addAll): " +
                                      coll);
  } catch (const UnsupportedOperationException &e) {
    // ok
  }

  checkIterator(coll->begin());
}

void TestUtil::syncConcurrentMerges(shared_ptr<IndexWriter> writer)
{
  syncConcurrentMerges(writer->getConfig()->getMergeScheduler());
}

void TestUtil::syncConcurrentMerges(shared_ptr<MergeScheduler> ms)
{
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))->sync();
  }
}

shared_ptr<CheckIndex::Status>
TestUtil::checkIndex(shared_ptr<Directory> dir) 
{
  return checkIndex(dir, true);
}

shared_ptr<CheckIndex::Status>
TestUtil::checkIndex(shared_ptr<Directory> dir,
                     bool crossCheckTermVectors) 
{
  return checkIndex(dir, crossCheckTermVectors, false, nullptr);
}

shared_ptr<CheckIndex::Status> TestUtil::checkIndex(
    shared_ptr<Directory> dir, bool crossCheckTermVectors, bool failFast,
    shared_ptr<ByteArrayOutputStream> output) 
{
  if (output == nullptr) {
    output = make_shared<ByteArrayOutputStream>(1024);
  }
  // TODO: actually use the dir's locking, unless test uses a special method?
  // some tests e.g. exception tests become much more complicated if they have
  // to close the writer
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.index.CheckIndex checker =
  // new org.apache.lucene.index.CheckIndex(dir,
  // org.apache.lucene.store.NoLockFactory.INSTANCE.obtainLock(dir, "bogus")))
  {
    org::apache::lucene::index::CheckIndex checker =
        org::apache::lucene::index::CheckIndex(
            dir,
            org::apache::lucene::store::NoLockFactory::INSTANCE->obtainLock(
                dir, L"bogus"));
    checker->setCrossCheckTermVectors(crossCheckTermVectors);
    checker->setFailFast(failFast);
    checker->setInfoStream(
        make_shared<PrintStream>(output, false, IOUtils::UTF_8), false);
    shared_ptr<CheckIndex::Status> indexStatus = checker->checkIndex(nullptr);

    if (indexStatus == nullptr || indexStatus->clean == false) {
      wcout << L"CheckIndex failed" << endl;
      // C++ TODO: There is no native C++ equivalent to 'toString':
      wcout << output->toString(IOUtils::UTF_8) << endl;
      throw runtime_error(L"CheckIndex failed");
    } else {
      if (LuceneTestCase::INFOSTREAM) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        wcout << output->toString(IOUtils::UTF_8) << endl;
      }
      return indexStatus;
    }
  }
}

void TestUtil::checkReader(shared_ptr<IndexReader> reader) 
{
  for (auto context : reader->leaves()) {
    checkReader(context->reader(), true);
  }
}

void TestUtil::checkReader(shared_ptr<LeafReader> reader,
                           bool crossCheckTermVectors) 
{
  shared_ptr<ByteArrayOutputStream> bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<PrintStream> infoStream =
      make_shared<PrintStream>(bos, false, IOUtils::UTF_8);

  shared_ptr<CodecReader> *const codecReader;
  if (std::dynamic_pointer_cast<CodecReader>(reader) != nullptr) {
    codecReader = std::static_pointer_cast<CodecReader>(reader);
    reader->checkIntegrity();
  } else {
    codecReader = SlowCodecReaderWrapper::wrap(reader);
  }
  CheckIndex::testLiveDocs(codecReader, infoStream, true);
  CheckIndex::testFieldInfos(codecReader, infoStream, true);
  CheckIndex::testFieldNorms(codecReader, infoStream, true);
  CheckIndex::testPostings(codecReader, infoStream, false, true,
                           Version::LUCENE_7_0_0);
  CheckIndex::testStoredFields(codecReader, infoStream, true);
  CheckIndex::testTermVectors(codecReader, infoStream, false,
                              crossCheckTermVectors, true,
                              Version::LUCENE_7_0_0);
  CheckIndex::testDocValues(codecReader, infoStream, true);
  CheckIndex::testPoints(codecReader, infoStream, true);

  // some checks really against the reader API
  checkReaderSanity(reader);

  if (LuceneTestCase::INFOSTREAM) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << bos->toString(IOUtils::UTF_8) << endl;
  }

  shared_ptr<LeafReader> unwrapped = FilterLeafReader::unwrap(reader);
  if (std::dynamic_pointer_cast<SegmentReader>(unwrapped) != nullptr) {
    shared_ptr<SegmentReader> sr =
        std::static_pointer_cast<SegmentReader>(unwrapped);
    int64_t bytesUsed = sr->ramBytesUsed();
    if (sr->ramBytesUsed() < 0) {
      throw make_shared<IllegalStateException>(
          L"invalid ramBytesUsed for reader: " + to_wstring(bytesUsed));
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assert(Accountables::toString(sr) != L"");
  }
}

void TestUtil::checkReaderSanity(shared_ptr<LeafReader> reader) throw(
    IOException)
{
  for (auto info : reader->getFieldInfos()) {

    // reader shouldn't return normValues if the field does not have them
    if (!info->hasNorms()) {
      if (reader->getNormValues(info->name) != nullptr) {
        throw runtime_error(L"field: " + info->name +
                            L" should omit norms but has them!");
      }
    }

    // reader shouldn't return docValues if the field does not have them
    // reader shouldn't return multiple docvalues types for the same field.
    switch (info->getDocValuesType()) {
    case PostingsEnum::NONE:
      if (reader->getBinaryDocValues(info->name) != nullptr ||
          reader->getNumericDocValues(info->name) != nullptr ||
          reader->getSortedDocValues(info->name) != nullptr ||
          reader->getSortedSetDocValues(info->name) != nullptr) {
        throw runtime_error(L"field: " + info->name +
                            L" has docvalues but should omit them!");
      }
      break;
    case Lucene70DocValuesFormat::SORTED:
      if (reader->getBinaryDocValues(info->name) != nullptr ||
          reader->getNumericDocValues(info->name) != nullptr ||
          reader->getSortedNumericDocValues(info->name) != nullptr ||
          reader->getSortedSetDocValues(info->name) != nullptr) {
        throw runtime_error(info->name + L" returns multiple docvalues types!");
      }
      break;
    case Lucene70DocValuesFormat::SORTED_NUMERIC:
      if (reader->getBinaryDocValues(info->name) != nullptr ||
          reader->getNumericDocValues(info->name) != nullptr ||
          reader->getSortedSetDocValues(info->name) != nullptr ||
          reader->getSortedDocValues(info->name) != nullptr) {
        throw runtime_error(info->name + L" returns multiple docvalues types!");
      }
      break;
    case Lucene70DocValuesFormat::SORTED_SET:
      if (reader->getBinaryDocValues(info->name) != nullptr ||
          reader->getNumericDocValues(info->name) != nullptr ||
          reader->getSortedNumericDocValues(info->name) != nullptr ||
          reader->getSortedDocValues(info->name) != nullptr) {
        throw runtime_error(info->name + L" returns multiple docvalues types!");
      }
      break;
    case Lucene70DocValuesFormat::BINARY:
      if (reader->getNumericDocValues(info->name) != nullptr ||
          reader->getSortedDocValues(info->name) != nullptr ||
          reader->getSortedNumericDocValues(info->name) != nullptr ||
          reader->getSortedSetDocValues(info->name) != nullptr) {
        throw runtime_error(info->name + L" returns multiple docvalues types!");
      }
      break;
    case Lucene70DocValuesFormat::NUMERIC:
      if (reader->getBinaryDocValues(info->name) != nullptr ||
          reader->getSortedDocValues(info->name) != nullptr ||
          reader->getSortedNumericDocValues(info->name) != nullptr ||
          reader->getSortedSetDocValues(info->name) != nullptr) {
        throw runtime_error(info->name + L" returns multiple docvalues types!");
      }
      break;
    default:
      throw make_shared<AssertionError>();
    }
  }
}

int TestUtil::nextInt(shared_ptr<Random> r, int start, int end)
{
  return RandomNumbers::randomIntBetween(r, start, end);
}

int64_t TestUtil::nextLong(shared_ptr<Random> r, int64_t start,
                             int64_t end)
{
  assert(end >= start);
  shared_ptr<int64_t> *const range =
      static_cast<int64_t>(end)
          .add(static_cast<int64_t>(1))
          .subtract(static_cast<int64_t>(start));
  if (range->compareTo(static_cast<int64_t>(numeric_limits<int>::max())) <=
      0) {
    return start + r->nextInt(range->intValue());
  } else {
    // probably not evenly distributed when range is large, but OK for tests
    shared_ptr<int64_t> *const augend =
        (BigDecimal(range))
            .multiply(BigDecimal(r->nextDouble()))
            .toBigInteger();
    constexpr int64_t result =
        static_cast<int64_t>(start).add(augend).longValue();
    assert(result >= start);
    assert(result <= end);
    return result;
  }
}

shared_ptr<int64_t> TestUtil::nextBigInteger(shared_ptr<Random> random,
                                                int maxBytes)
{
  int length = TestUtil::nextInt(random, 1, maxBytes);
  std::deque<char> buffer(length);
  random->nextBytes(buffer);
  return make_shared<int64_t>(buffer);
}

wstring TestUtil::randomSimpleString(shared_ptr<Random> r, int maxLength)
{
  return randomSimpleString(r, 0, maxLength);
}

wstring TestUtil::randomSimpleString(shared_ptr<Random> r, int minLength,
                                     int maxLength)
{
  constexpr int end = nextInt(r, minLength, maxLength);
  if (end == 0) {
    // allow 0 length
    return L"";
  }
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(end);
  for (int i = 0; i < end; i++) {
    buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, L'a', L'z'));
  }
  return wstring(buffer, 0, end);
}

wstring TestUtil::randomSimpleStringRange(shared_ptr<Random> r, wchar_t minChar,
                                          wchar_t maxChar, int maxLength)
{
  constexpr int end = nextInt(r, 0, maxLength);
  if (end == 0) {
    // allow 0 length
    return L"";
  }
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(end);
  for (int i = 0; i < end; i++) {
    buffer[i] = static_cast<wchar_t>(TestUtil::nextInt(r, minChar, maxChar));
  }
  return wstring(buffer, 0, end);
}

wstring TestUtil::randomSimpleString(shared_ptr<Random> r)
{
  return randomSimpleString(r, 0, 10);
}

wstring TestUtil::randomUnicodeString(shared_ptr<Random> r)
{
  return randomUnicodeString(r, 20);
}

wstring TestUtil::randomUnicodeString(shared_ptr<Random> r, int maxLength)
{
  constexpr int end = nextInt(r, 0, maxLength);
  if (end == 0) {
    // allow 0 length
    return L"";
  }
  const std::deque<wchar_t> buffer = std::deque<wchar_t>(end);
  randomFixedLengthUnicodeString(r, buffer, 0, buffer.size());
  return wstring(buffer, 0, end);
}

void TestUtil::randomFixedLengthUnicodeString(shared_ptr<Random> random,
                                              std::deque<wchar_t> &chars,
                                              int offset, int length)
{
  int i = offset;
  constexpr int end = offset + length;
  while (i < end) {
    constexpr int t = random->nextInt(5);
    if (0 == t && i < length - 1) {
      // Make a surrogate pair
      // High surrogate
      chars[i++] = static_cast<wchar_t>(nextInt(random, 0xd800, 0xdbff));
      // Low surrogate
      chars[i++] = static_cast<wchar_t>(nextInt(random, 0xdc00, 0xdfff));
    } else if (t <= 1) {
      chars[i++] = static_cast<wchar_t>(random->nextInt(0x80));
    } else if (2 == t) {
      chars[i++] = static_cast<wchar_t>(nextInt(random, 0x80, 0x7ff));
    } else if (3 == t) {
      chars[i++] = static_cast<wchar_t>(nextInt(random, 0x800, 0xd7ff));
    } else if (4 == t) {
      chars[i++] = static_cast<wchar_t>(nextInt(random, 0xe000, 0xffff));
    }
  }
}

wstring TestUtil::randomRegexpishString(shared_ptr<Random> r)
{
  return randomRegexpishString(r, 20);
}

const deque<wstring> TestUtil::ops = java::util::Arrays::asList(
    L".", L"?", L"{0," + to_wstring(maxRecursionBound) + L"}",
    L"{1," + to_wstring(maxRecursionBound) + L"}", L"(", L")", L"-", L"[", L"]",
    L"|");

wstring TestUtil::randomRegexpishString(shared_ptr<Random> r, int maxLength)
{
  shared_ptr<StringBuilder> *const regexp =
      make_shared<StringBuilder>(maxLength);
  for (int i = nextInt(r, 0, maxLength); i > 0; i--) {
    if (r->nextBoolean()) {
      regexp->append(
          static_cast<wchar_t>(RandomNumbers::randomIntBetween(r, L'a', L'z')));
    } else {
      regexp->append(RandomPicks::randomFrom(r, ops));
    }
  }
  return regexp->toString();
}

std::deque<wstring> const TestUtil::HTML_CHAR_ENTITIES = {
    L"AElig",  L"Aacute",  L"Acirc",    L"Agrave",  L"Alpha",   L"AMP",
    L"Aring",  L"Atilde",  L"Auml",     L"Beta",    L"COPY",    L"Ccedil",
    L"Chi",    L"Dagger",  L"Delta",    L"ETH",     L"Eacute",  L"Ecirc",
    L"Egrave", L"Epsilon", L"Eta",      L"Euml",    L"Gamma",   L"GT",
    L"Iacute", L"Icirc",   L"Igrave",   L"Iota",    L"Iuml",    L"Kappa",
    L"Lambda", L"LT",      L"Mu",       L"Ntilde",  L"Nu",      L"OElig",
    L"Oacute", L"Ocirc",   L"Ograve",   L"Omega",   L"Omicron", L"Oslash",
    L"Otilde", L"Ouml",    L"Phi",      L"Pi",      L"Prime",   L"Psi",
    L"QUOT",   L"REG",     L"Rho",      L"Scaron",  L"Sigma",   L"THORN",
    L"Tau",    L"Theta",   L"Uacute",   L"Ucirc",   L"Ugrave",  L"Upsilon",
    L"Uuml",   L"Xi",      L"Yacute",   L"Yuml",    L"Zeta",    L"aacute",
    L"acirc",  L"acute",   L"aelig",    L"agrave",  L"alefsym", L"alpha",
    L"amp",    L"and",     L"ang",      L"apos",    L"aring",   L"asymp",
    L"atilde", L"auml",    L"bdquo",    L"beta",    L"brvbar",  L"bull",
    L"cap",    L"ccedil",  L"cedil",    L"cent",    L"chi",     L"circ",
    L"clubs",  L"cong",    L"copy",     L"crarr",   L"cup",     L"curren",
    L"dArr",   L"dagger",  L"darr",     L"deg",     L"delta",   L"diams",
    L"divide", L"eacute",  L"ecirc",    L"egrave",  L"empty",   L"emsp",
    L"ensp",   L"epsilon", L"equiv",    L"eta",     L"eth",     L"euml",
    L"euro",   L"exist",   L"fnof",     L"forall",  L"frac12",  L"frac14",
    L"frac34", L"frasl",   L"gamma",    L"ge",      L"gt",      L"hArr",
    L"harr",   L"hearts",  L"hellip",   L"iacute",  L"icirc",   L"iexcl",
    L"igrave", L"image",   L"infin",    L"int",     L"iota",    L"iquest",
    L"isin",   L"iuml",    L"kappa",    L"lArr",    L"lambda",  L"lang",
    L"laquo",  L"larr",    L"lceil",    L"ldquo",   L"le",      L"lfloor",
    L"lowast", L"loz",     L"lrm",      L"lsaquo",  L"lsquo",   L"lt",
    L"macr",   L"mdash",   L"micro",    L"middot",  L"minus",   L"mu",
    L"nabla",  L"nbsp",    L"ndash",    L"ne",      L"ni",      L"not",
    L"notin",  L"nsub",    L"ntilde",   L"nu",      L"oacute",  L"ocirc",
    L"oelig",  L"ograve",  L"oline",    L"omega",   L"omicron", L"oplus",
    L"or",     L"ordf",    L"ordm",     L"oslash",  L"otilde",  L"otimes",
    L"ouml",   L"para",    L"part",     L"permil",  L"perp",    L"phi",
    L"pi",     L"piv",     L"plusmn",   L"pound",   L"prime",   L"prod",
    L"prop",   L"psi",     L"quot",     L"rArr",    L"radic",   L"rang",
    L"raquo",  L"rarr",    L"rceil",    L"rdquo",   L"real",    L"reg",
    L"rfloor", L"rho",     L"rlm",      L"rsaquo",  L"rsquo",   L"sbquo",
    L"scaron", L"sdot",    L"sect",     L"shy",     L"sigma",   L"sigmaf",
    L"sim",    L"spades",  L"sub",      L"sube",    L"sum",     L"sup",
    L"sup1",   L"sup2",    L"sup3",     L"supe",    L"szlig",   L"tau",
    L"there4", L"theta",   L"thetasym", L"thinsp",  L"thorn",   L"tilde",
    L"times",  L"trade",   L"uArr",     L"uacute",  L"uarr",    L"ucirc",
    L"ugrave", L"uml",     L"upsih",    L"upsilon", L"uuml",    L"weierp",
    L"xi",     L"yacute",  L"yen",      L"yuml",    L"zeta",    L"zwj",
    L"zwnj"};

wstring TestUtil::randomHtmlishString(shared_ptr<Random> random,
                                      int numElements)
{
  constexpr int end = nextInt(random, 0, numElements);
  if (end == 0) {
    // allow 0 length
    return L"";
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < end; i++) {
    int val = random->nextInt(25);
    switch (val) {
    case 0:
      sb->append(L"<p>");
      break;
    case 1: {
      sb->append(L"<");
      sb->append((wstring(L"    ")).substr(nextInt(random, 0, 4)));
      sb->append(randomSimpleString(random));
      for (int j = 0; j < nextInt(random, 0, 10); ++j) {
        sb->append(L' ');
        sb->append(randomSimpleString(random));
        sb->append((wstring(L" ")).substr(nextInt(random, 0, 1)));
        sb->append(L'=');
        sb->append((wstring(L" ")).substr(nextInt(random, 0, 1)));
        sb->append((wstring(L"\"")).substr(nextInt(random, 0, 1)));
        sb->append(randomSimpleString(random));
        sb->append((wstring(L"\"")).substr(nextInt(random, 0, 1)));
      }
      sb->append((wstring(L"    ")).substr(nextInt(random, 0, 4)));
      sb->append((wstring(L"/")).substr(nextInt(random, 0, 1)));
      sb->append((wstring(L">")).substr(nextInt(random, 0, 1)));
      break;
    }
    case 2: {
      sb->append(L"</");
      sb->append((wstring(L"    ")).substr(nextInt(random, 0, 4)));
      sb->append(randomSimpleString(random));
      sb->append((wstring(L"    ")).substr(nextInt(random, 0, 4)));
      sb->append((wstring(L">")).substr(nextInt(random, 0, 1)));
      break;
    }
    case 3:
      sb->append(L">");
      break;
    case 4:
      sb->append(L"</p>");
      break;
    case 5:
      sb->append(L"<!--");
      break;
    case 6:
      sb->append(L"<!--#");
      break;
    case 7:
      sb->append(L"<script><!-- f('");
      break;
    case 8:
      sb->append(L"</script>");
      break;
    case 9:
      sb->append(L"<?");
      break;
    case 10:
      sb->append(L"?>");
      break;
    case 11:
      sb->append(L"\"");
      break;
    case 12:
      sb->append(L"\\\"");
      break;
    case 13:
      sb->append(L"'");
      break;
    case 14:
      sb->append(L"\\'");
      break;
    case 15:
      sb->append(L"-->");
      break;
    case 16: {
      sb->append(L"&");
      switch (nextInt(random, 0, 2)) {
      case 0:
        sb->append(randomSimpleString(random));
        break;
      case 1:
        sb->append(
            HTML_CHAR_ENTITIES[random->nextInt(HTML_CHAR_ENTITIES.size())]);
        break;
      }
      sb->append((wstring(L";")).substr(nextInt(random, 0, 1)));
      break;
    }
    case 17: {
      sb->append(L"&#");
      if (0 == nextInt(random, 0, 1)) {
        sb->append(nextInt(random, 0, numeric_limits<int>::max() - 1));
        sb->append((wstring(L";")).substr(nextInt(random, 0, 1)));
      }
      break;
    }
    case 18: {
      sb->append(L"&#x");
      if (0 == nextInt(random, 0, 1)) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        sb->append(Integer::toString(
            nextInt(random, 0, numeric_limits<int>::max() - 1), 16));
        sb->append((wstring(L";")).substr(nextInt(random, 0, 1)));
      }
      break;
    }

    case 19:
      sb->append(L";");
      break;
    case 20:
      sb->append(nextInt(random, 0, numeric_limits<int>::max() - 1));
      break;
    case 21:
      sb->append(L"\n");
      break;
    case 22:
      sb->append((wstring(L"          ")).substr(nextInt(random, 0, 10)));
      break;
    case 23: {
      sb->append(L"<");
      if (0 == nextInt(random, 0, 3)) {
        sb->append((wstring(L"          ")).substr(nextInt(random, 1, 10)));
      }
      if (0 == nextInt(random, 0, 1)) {
        sb->append(L"/");
        if (0 == nextInt(random, 0, 3)) {
          sb->append((wstring(L"          ")).substr(nextInt(random, 1, 10)));
        }
      }
      switch (nextInt(random, 0, 3)) {
      case 0:
        sb->append(randomlyRecaseCodePoints(random, L"script"));
        break;
      case 1:
        sb->append(randomlyRecaseCodePoints(random, L"style"));
        break;
      case 2:
        sb->append(randomlyRecaseCodePoints(random, L"br"));
        break;
        // default: append nothing
      }
      sb->append((wstring(L">")).substr(nextInt(random, 0, 1)));
      break;
    }
    default:
      sb->append(randomSimpleString(random));
    }
  }
  return sb->toString();
}

wstring TestUtil::randomlyRecaseCodePoints(shared_ptr<Random> random,
                                           const wstring &str)
{
  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  int pos = 0;
  while (pos < str.length()) {
    int codePoint = str.codePointAt(pos);
    pos += Character::charCount(codePoint);
    switch (nextInt(random, 0, 2)) {
    case 0:
      builder->appendCodePoint(towupper(codePoint));
      break;
    case 1:
      builder->appendCodePoint(towlower(codePoint));
      break;
    case 2:
      builder->appendCodePoint(codePoint); // leave intact
    }
  }
  return builder->toString();
}

std::deque<int> const TestUtil::blockStarts = {
    0x0000,  0x0080,  0x0100,  0x0180,  0x0250,  0x02B0,  0x0300,  0x0370,
    0x0400,  0x0500,  0x0530,  0x0590,  0x0600,  0x0700,  0x0750,  0x0780,
    0x07C0,  0x0800,  0x0900,  0x0980,  0x0A00,  0x0A80,  0x0B00,  0x0B80,
    0x0C00,  0x0C80,  0x0D00,  0x0D80,  0x0E00,  0x0E80,  0x0F00,  0x1000,
    0x10A0,  0x1100,  0x1200,  0x1380,  0x13A0,  0x1400,  0x1680,  0x16A0,
    0x1700,  0x1720,  0x1740,  0x1760,  0x1780,  0x1800,  0x18B0,  0x1900,
    0x1950,  0x1980,  0x19E0,  0x1A00,  0x1A20,  0x1B00,  0x1B80,  0x1C00,
    0x1C50,  0x1CD0,  0x1D00,  0x1D80,  0x1DC0,  0x1E00,  0x1F00,  0x2000,
    0x2070,  0x20A0,  0x20D0,  0x2100,  0x2150,  0x2190,  0x2200,  0x2300,
    0x2400,  0x2440,  0x2460,  0x2500,  0x2580,  0x25A0,  0x2600,  0x2700,
    0x27C0,  0x27F0,  0x2800,  0x2900,  0x2980,  0x2A00,  0x2B00,  0x2C00,
    0x2C60,  0x2C80,  0x2D00,  0x2D30,  0x2D80,  0x2DE0,  0x2E00,  0x2E80,
    0x2F00,  0x2FF0,  0x3000,  0x3040,  0x30A0,  0x3100,  0x3130,  0x3190,
    0x31A0,  0x31C0,  0x31F0,  0x3200,  0x3300,  0x3400,  0x4DC0,  0x4E00,
    0xA000,  0xA490,  0xA4D0,  0xA500,  0xA640,  0xA6A0,  0xA700,  0xA720,
    0xA800,  0xA830,  0xA840,  0xA880,  0xA8E0,  0xA900,  0xA930,  0xA960,
    0xA980,  0xAA00,  0xAA60,  0xAA80,  0xABC0,  0xAC00,  0xD7B0,  0xE000,
    0xF900,  0xFB00,  0xFB50,  0xFE00,  0xFE10,  0xFE20,  0xFE30,  0xFE50,
    0xFE70,  0xFF00,  0xFFF0,  0x10000, 0x10080, 0x10100, 0x10140, 0x10190,
    0x101D0, 0x10280, 0x102A0, 0x10300, 0x10330, 0x10380, 0x103A0, 0x10400,
    0x10450, 0x10480, 0x10800, 0x10840, 0x10900, 0x10920, 0x10A00, 0x10A60,
    0x10B00, 0x10B40, 0x10B60, 0x10C00, 0x10E60, 0x11080, 0x12000, 0x12400,
    0x13000, 0x1D000, 0x1D100, 0x1D200, 0x1D300, 0x1D360, 0x1D400, 0x1F000,
    0x1F030, 0x1F100, 0x1F200, 0x20000, 0x2A700, 0x2F800, 0xE0000, 0xE0100,
    0xF0000, 0x100000};
std::deque<int> const TestUtil::blockEnds = {
    0x007F,  0x00FF,  0x017F,  0x024F,  0x02AF,  0x02FF,  0x036F,  0x03FF,
    0x04FF,  0x052F,  0x058F,  0x05FF,  0x06FF,  0x074F,  0x077F,  0x07BF,
    0x07FF,  0x083F,  0x097F,  0x09FF,  0x0A7F,  0x0AFF,  0x0B7F,  0x0BFF,
    0x0C7F,  0x0CFF,  0x0D7F,  0x0DFF,  0x0E7F,  0x0EFF,  0x0FFF,  0x109F,
    0x10FF,  0x11FF,  0x137F,  0x139F,  0x13FF,  0x167F,  0x169F,  0x16FF,
    0x171F,  0x173F,  0x175F,  0x177F,  0x17FF,  0x18AF,  0x18FF,  0x194F,
    0x197F,  0x19DF,  0x19FF,  0x1A1F,  0x1AAF,  0x1B7F,  0x1BBF,  0x1C4F,
    0x1C7F,  0x1CFF,  0x1D7F,  0x1DBF,  0x1DFF,  0x1EFF,  0x1FFF,  0x206F,
    0x209F,  0x20CF,  0x20FF,  0x214F,  0x218F,  0x21FF,  0x22FF,  0x23FF,
    0x243F,  0x245F,  0x24FF,  0x257F,  0x259F,  0x25FF,  0x26FF,  0x27BF,
    0x27EF,  0x27FF,  0x28FF,  0x297F,  0x29FF,  0x2AFF,  0x2BFF,  0x2C5F,
    0x2C7F,  0x2CFF,  0x2D2F,  0x2D7F,  0x2DDF,  0x2DFF,  0x2E7F,  0x2EFF,
    0x2FDF,  0x2FFF,  0x303F,  0x309F,  0x30FF,  0x312F,  0x318F,  0x319F,
    0x31BF,  0x31EF,  0x31FF,  0x32FF,  0x33FF,  0x4DBF,  0x4DFF,  0x9FFF,
    0xA48F,  0xA4CF,  0xA4FF,  0xA63F,  0xA69F,  0xA6FF,  0xA71F,  0xA7FF,
    0xA82F,  0xA83F,  0xA87F,  0xA8DF,  0xA8FF,  0xA92F,  0xA95F,  0xA97F,
    0xA9DF,  0xAA5F,  0xAA7F,  0xAADF,  0xABFF,  0xD7AF,  0xD7FF,  0xF8FF,
    0xFAFF,  0xFB4F,  0xFDFF,  0xFE0F,  0xFE1F,  0xFE2F,  0xFE4F,  0xFE6F,
    0xFEFF,  0xFFEF,  0xFFFF,  0x1007F, 0x100FF, 0x1013F, 0x1018F, 0x101CF,
    0x101FF, 0x1029F, 0x102DF, 0x1032F, 0x1034F, 0x1039F, 0x103DF, 0x1044F,
    0x1047F, 0x104AF, 0x1083F, 0x1085F, 0x1091F, 0x1093F, 0x10A5F, 0x10A7F,
    0x10B3F, 0x10B5F, 0x10B7F, 0x10C4F, 0x10E7F, 0x110CF, 0x123FF, 0x1247F,
    0x1342F, 0x1D0FF, 0x1D1FF, 0x1D24F, 0x1D35F, 0x1D37F, 0x1D7FF, 0x1F02F,
    0x1F09F, 0x1F1FF, 0x1F2FF, 0x2A6DF, 0x2B73F, 0x2FA1F, 0xE007F, 0xE01EF,
    0xFFFFF, 0x10FFFF};

wstring TestUtil::randomRealisticUnicodeString(shared_ptr<Random> r)
{
  return randomRealisticUnicodeString(r, 20);
}

wstring TestUtil::randomRealisticUnicodeString(shared_ptr<Random> r,
                                               int maxLength)
{
  return randomRealisticUnicodeString(r, 0, maxLength);
}

wstring TestUtil::randomRealisticUnicodeString(shared_ptr<Random> r,
                                               int minLength, int maxLength)
{
  constexpr int end = nextInt(r, minLength, maxLength);
  constexpr int block = r->nextInt(blockStarts.size());
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < end; i++) {
    sb->appendCodePoint(nextInt(r, blockStarts[block], blockEnds[block]));
  }
  return sb->toString();
}

wstring TestUtil::randomFixedByteLengthUnicodeString(shared_ptr<Random> r,
                                                     int length)
{

  const std::deque<wchar_t> buffer = std::deque<wchar_t>(length * 3);
  int bytes = length;
  int i = 0;
  for (; i < buffer.size() && bytes != 0; i++) {
    int t;
    if (bytes >= 4) {
      t = r->nextInt(5);
    } else if (bytes >= 3) {
      t = r->nextInt(4);
    } else if (bytes >= 2) {
      t = r->nextInt(2);
    } else {
      t = 0;
    }
    if (t == 0) {
      buffer[i] = static_cast<wchar_t>(r->nextInt(0x80));
      bytes--;
    } else if (1 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(r, 0x80, 0x7ff));
      bytes -= 2;
    } else if (2 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(r, 0x800, 0xd7ff));
      bytes -= 3;
    } else if (3 == t) {
      buffer[i] = static_cast<wchar_t>(nextInt(r, 0xe000, 0xffff));
      bytes -= 3;
    } else if (4 == t) {
      // Make a surrogate pair
      // High surrogate
      buffer[i++] = static_cast<wchar_t>(nextInt(r, 0xd800, 0xdbff));
      // Low surrogate
      buffer[i] = static_cast<wchar_t>(nextInt(r, 0xdc00, 0xdfff));
      bytes -= 4;
    }
  }
  return wstring(buffer, 0, i);
}

shared_ptr<BytesRef> TestUtil::randomBinaryTerm(shared_ptr<Random> r)
{
  int length = r->nextInt(15);
  shared_ptr<BytesRef> b = make_shared<BytesRef>(length);
  r->nextBytes(b->bytes);
  b->length = length;
  return b;
}

shared_ptr<Codec>
TestUtil::alwaysPostingsFormat(shared_ptr<PostingsFormat> format)
{
  // TODO: we really need for postings impls etc to announce themselves
  // (and maybe their params, too) to infostream on flush and merge.
  // otherwise in a real debugging situation we won't know whats going on!
  if (LuceneTestCase::VERBOSE) {
    wcout << L"forcing postings format to:" << format << endl;
  }
  return make_shared<AssertingCodecAnonymousInnerClass>(format);
}

TestUtil::AssertingCodecAnonymousInnerClass::AssertingCodecAnonymousInnerClass(
    shared_ptr<PostingsFormat> format)
{
  this->format = format;
}

shared_ptr<PostingsFormat>
TestUtil::AssertingCodecAnonymousInnerClass::getPostingsFormatForField(
    const wstring &field)
{
  return format;
}

shared_ptr<Codec>
TestUtil::alwaysDocValuesFormat(shared_ptr<DocValuesFormat> format)
{
  // TODO: we really need for docvalues impls etc to announce themselves
  // (and maybe their params, too) to infostream on flush and merge.
  // otherwise in a real debugging situation we won't know whats going on!
  if (LuceneTestCase::VERBOSE) {
    wcout << L"TestUtil: forcing docvalues format to:" << format << endl;
  }
  return make_shared<AssertingCodecAnonymousInnerClass2>(format);
}

TestUtil::AssertingCodecAnonymousInnerClass2::
    AssertingCodecAnonymousInnerClass2(shared_ptr<DocValuesFormat> format)
{
  this->format = format;
}

shared_ptr<DocValuesFormat>
TestUtil::AssertingCodecAnonymousInnerClass2::getDocValuesFormatForField(
    const wstring &field)
{
  return format;
}

shared_ptr<Codec> TestUtil::getDefaultCodec()
{
  return make_shared<Lucene70Codec>();
}

shared_ptr<PostingsFormat> TestUtil::getDefaultPostingsFormat()
{
  return make_shared<Lucene50PostingsFormat>();
}

shared_ptr<PostingsFormat>
TestUtil::getDefaultPostingsFormat(int minItemsPerBlock, int maxItemsPerBlock)
{
  return make_shared<Lucene50PostingsFormat>(minItemsPerBlock,
                                             maxItemsPerBlock);
}

shared_ptr<PostingsFormat>
TestUtil::getPostingsFormatWithOrds(shared_ptr<Random> r)
{
  switch (r->nextInt(2)) {
  case 0:
    return make_shared<LuceneFixedGap>();
  case 1:
    return make_shared<BlockTreeOrdsPostingsFormat>();
  // TODO: these don't actually support ords!
  // case 2: return new FSTOrdPostingsFormat();
  default:
    throw make_shared<AssertionError>();
  }
}

shared_ptr<DocValuesFormat> TestUtil::getDefaultDocValuesFormat()
{
  return make_shared<Lucene70DocValuesFormat>();
}

wstring TestUtil::getPostingsFormat(const wstring &field)
{
  return getPostingsFormat(Codec::getDefault(), field);
}

wstring TestUtil::getPostingsFormat(shared_ptr<Codec> codec,
                                    const wstring &field)
{
  shared_ptr<PostingsFormat> p = codec->postingsFormat();
  if (std::dynamic_pointer_cast<PerFieldPostingsFormat>(p) != nullptr) {
    return (std::static_pointer_cast<PerFieldPostingsFormat>(p))
        ->getPostingsFormatForField(field)
        ->getName();
  } else {
    return p->getName();
  }
}

wstring TestUtil::getDocValuesFormat(const wstring &field)
{
  return getDocValuesFormat(Codec::getDefault(), field);
}

wstring TestUtil::getDocValuesFormat(shared_ptr<Codec> codec,
                                     const wstring &field)
{
  shared_ptr<DocValuesFormat> f = codec->docValuesFormat();
  if (std::dynamic_pointer_cast<PerFieldDocValuesFormat>(f) != nullptr) {
    return (std::static_pointer_cast<PerFieldDocValuesFormat>(f))
        ->getDocValuesFormatForField(field)
        ->getName();
  } else {
    return f->getName();
  }
}

bool TestUtil::fieldSupportsHugeBinaryDocValues(const wstring &field)
{
  wstring dvFormat = getDocValuesFormat(field);
  if (dvFormat == L"Lucene40" || dvFormat == L"Lucene42" ||
      dvFormat == L"Memory") {
    return false;
  }
  return true;
}

bool TestUtil::anyFilesExceptWriteLock(shared_ptr<Directory> dir) throw(
    IOException)
{
  std::deque<wstring> files = dir->listAll();
  if (files.size() > 1 || (files.size() == 1 && files[0] != L"write.lock")) {
    return true;
  } else {
    return false;
  }
}

void TestUtil::addIndexesSlowly(
    shared_ptr<IndexWriter> writer,
    deque<DirectoryReader> &readers) 
{
  deque<std::shared_ptr<CodecReader>> leaves =
      deque<std::shared_ptr<CodecReader>>();
  for (shared_ptr<DirectoryReader> reader : readers) {
    for (shared_ptr<LeafReaderContext> context : reader->leaves()) {
      leaves.push_back(SlowCodecReaderWrapper::wrap(context->reader()));
    }
  }
  writer->addIndexes({leaves.toArray(
      std::deque<std::shared_ptr<CodecReader>>(leaves.size()))});
}

void TestUtil::reduceOpenFiles(shared_ptr<IndexWriter> w)
{
  // keep number of open files lowish
  shared_ptr<MergePolicy> mp = w->getConfig()->getMergePolicy();
  mp->setNoCFSRatio(1.0);
  if (std::dynamic_pointer_cast<LogMergePolicy>(mp) != nullptr) {
    shared_ptr<LogMergePolicy> lmp =
        std::static_pointer_cast<LogMergePolicy>(mp);
    lmp->setMergeFactor(min(5, lmp->getMergeFactor()));
  } else if (std::dynamic_pointer_cast<TieredMergePolicy>(mp) != nullptr) {
    shared_ptr<TieredMergePolicy> tmp =
        std::static_pointer_cast<TieredMergePolicy>(mp);
    tmp->setMaxMergeAtOnce(min(5, tmp->getMaxMergeAtOnce()));
    tmp->setSegmentsPerTier(min(5, tmp->getSegmentsPerTier()));
  }
  shared_ptr<MergeScheduler> ms = w->getConfig()->getMergeScheduler();
  if (std::dynamic_pointer_cast<ConcurrentMergeScheduler>(ms) != nullptr) {
    // wtf... shouldnt it be even lower since it's 1 by default?!?!
    (std::static_pointer_cast<ConcurrentMergeScheduler>(ms))
        ->setMaxMergesAndThreads(3, 2);
  }
}

template <typename T>
void TestUtil::assertAttributeReflection(
    shared_ptr<AttributeImpl> att, unordered_map<wstring, T> &reflectedValues)
{
  const unordered_map<wstring, any> map_obj = unordered_map<wstring, any>();
  att->reflectWith([&](type_info attClass, wstring key, any value) {
    map_obj.emplace(attClass::getName() + L'#' + key, value);
  });
  Assert::assertEquals(L"Reflection does not produce same map_obj", reflectedValues,
                       map_obj);
}

void TestUtil::assertEquals(shared_ptr<TopDocs> expected,
                            shared_ptr<TopDocs> actual)
{
  Assert::assertEquals(L"wrong total hits", expected->totalHits,
                       actual->totalHits);
  Assert::assertEquals(L"wrong maxScore", expected->getMaxScore(),
                       actual->getMaxScore(), 0.0);
  Assert::assertEquals(L"wrong hit count", expected->scoreDocs.size(),
                       actual->scoreDocs.size());
  for (int hitIDX = 0; hitIDX < expected->scoreDocs.size(); hitIDX++) {
    shared_ptr<ScoreDoc> *const expectedSD = expected->scoreDocs[hitIDX];
    shared_ptr<ScoreDoc> *const actualSD = actual->scoreDocs[hitIDX];
    Assert::assertEquals(L"wrong hit docID", expectedSD->doc, actualSD->doc);
    Assert::assertEquals(L"wrong hit score", expectedSD->score, actualSD->score,
                         0.0);
    if (std::dynamic_pointer_cast<FieldDoc>(expectedSD) != nullptr) {
      Assert::assertTrue(std::dynamic_pointer_cast<FieldDoc>(actualSD) !=
                         nullptr);
      Assert::assertArrayEquals(
          L"wrong sort field values",
          (std::static_pointer_cast<FieldDoc>(expectedSD))->fields,
          (std::static_pointer_cast<FieldDoc>(actualSD))->fields);
    } else {
      Assert::assertFalse(std::dynamic_pointer_cast<FieldDoc>(actualSD) !=
                          nullptr);
    }
  }
}

shared_ptr<Document> TestUtil::cloneDocument(shared_ptr<Document> doc1)
{
  shared_ptr<Document> *const doc2 = make_shared<Document>();
  for (auto f : doc1->getFields()) {
    shared_ptr<Field> *const field1 = std::static_pointer_cast<Field>(f);
    shared_ptr<Field> *const field2;
    constexpr DocValuesType dvType = field1->fieldType()->docValuesType();
    constexpr int dimCount = field1->fieldType()->pointDimensionCount();
    if (dvType != DocValuesType::NONE) {
      switch (dvType) {
      case DocValuesType::NUMERIC:
        field2 = make_shared<NumericDocValuesField>(
            field1->name(), field1->numericValue()->longValue());
        break;
      case DocValuesType::BINARY:
        field2 = make_shared<BinaryDocValuesField>(field1->name(),
                                                   field1->binaryValue());
        break;
      case DocValuesType::SORTED:
        field2 = make_shared<SortedDocValuesField>(field1->name(),
                                                   field1->binaryValue());
        break;
      default:
        throw make_shared<IllegalStateException>(L"unknown Type: " + dvType);
      }
    } else if (dimCount != 0) {
      shared_ptr<BytesRef> br = field1->binaryValue();
      std::deque<char> bytes(br->length);
      System::arraycopy(br->bytes, br->offset, bytes, 0, br->length);
      field2 =
          make_shared<BinaryPoint>(field1->name(), bytes, field1->fieldType());
    } else {
      field2 = make_shared<Field>(field1->name(), field1->stringValue(),
                                  field1->fieldType());
    }
    doc2->push_back(field2);
  }

  return doc2;
}

shared_ptr<PostingsEnum>
TestUtil::docs(shared_ptr<Random> random, shared_ptr<IndexReader> r,
               const wstring &field, shared_ptr<BytesRef> term,
               shared_ptr<PostingsEnum> reuse, int flags) 
{
  shared_ptr<Terms> *const terms = MultiFields::getTerms(r, field);
  if (terms == nullptr) {
    return nullptr;
  }
  shared_ptr<TermsEnum> *const termsEnum = terms->begin();
  if (!termsEnum->seekExact(term)) {
    return nullptr;
  }
  return docs(random, termsEnum, reuse, flags);
}

shared_ptr<PostingsEnum> TestUtil::docs(shared_ptr<Random> random,
                                        shared_ptr<TermsEnum> termsEnum,
                                        shared_ptr<PostingsEnum> reuse,
                                        int flags) 
{
  // TODO: simplify this method? it would be easier to randomly either use the
  // flags passed, or do the random selection, FREQS should be part fo the
  // random selection instead of outside on its own?
  if (random->nextBoolean()) {
    if (random->nextBoolean()) {
      constexpr int posFlags;
      switch (random->nextInt(4)) {
      case 0:
        posFlags = PostingsEnum::POSITIONS;
        break;
      case 1:
        posFlags = PostingsEnum::OFFSETS;
        break;
      case 2:
        posFlags = PostingsEnum::PAYLOADS;
        break;
      default:
        posFlags = PostingsEnum::ALL;
        break;
      }
      return termsEnum->postings(nullptr, posFlags);
    }
    flags |= PostingsEnum::FREQS;
  }
  return termsEnum->postings(reuse, flags);
}

shared_ptr<std::wstring>
TestUtil::stringToCharSequence(const wstring &string, shared_ptr<Random> random)
{
  return bytesToCharSequence(make_shared<BytesRef>(string), random);
}

shared_ptr<std::wstring>
TestUtil::bytesToCharSequence(shared_ptr<BytesRef> ref,
                              shared_ptr<Random> random)
{
  switch (random->nextInt(5)) {
  case 4: {
    const std::deque<wchar_t> chars = std::deque<wchar_t>(ref->length);
    constexpr int len =
        UnicodeUtil::UTF8toUTF16(ref->bytes, ref->offset, ref->length, chars);
    return make_shared<CharsRef>(chars, 0, len);
  }
  case 3:
    return CharBuffer::wrap(ref->utf8ToString());
  default:
    return ref->utf8ToString();
  }
}

void TestUtil::shutdownExecutorService(shared_ptr<ExecutorService> ex)
{
  if (ex != nullptr) {
    try {
      ex->shutdown();
      ex->awaitTermination(1, TimeUnit::SECONDS);
    } catch (const InterruptedException &e) {
      // Just report it on the syserr.
      System::err::println(L"Could not properly close executor service.");
      e->printStackTrace(System::err);
    }
  }
}

shared_ptr<Pattern> TestUtil::randomPattern(shared_ptr<Random> random)
{
  const wstring nonBmpString = L"AB\uD840\uDC00C";
  while (true) {
    try {
      shared_ptr<Pattern> p =
          Pattern::compile(TestUtil::randomRegexpishString(random));
      wstring replacement = L"";
      // ignore bugs in Sun's regex impl
      try {
        replacement = p->matcher(nonBmpString).replaceAll(L"_");
      } catch (const StringIndexOutOfBoundsException &jdkBug) {
        wcout << L"WARNING: your jdk is buggy!" << endl;
        wcout << L"Pattern.compile(\"" << p->pattern()
              << L"\").matcher(\"AB\\uD840\\uDC00C\").replaceAll(\"_\"); "
                 L"should not throw IndexOutOfBounds!"
              << endl;
      }
      // Make sure the result of applying the pattern to a string with extended
      // unicode characters is a valid utf16 string. See LUCENE-4078 for
      // discussion.
      if (replacement != L"" && UnicodeUtil::validUTF16String(replacement)) {
        return p;
      }
    } catch (const PatternSyntaxException &ignored) {
      // Loop trying until we hit something that compiles.
    }
  }
}

wstring TestUtil::randomAnalysisString(shared_ptr<Random> random, int maxLength,
                                       bool simple)
{
  assert(maxLength >= 0);

  // sometimes just a purely random string
  if (random->nextInt(31) == 0) {
    return randomSubString(random, random->nextInt(maxLength), simple);
  }

  // otherwise, try to make it more realistic with 'words' since most tests use
  // MockTokenizer first decide how big the string will really be: 0..n
  maxLength = random->nextInt(maxLength);
  int avgWordLength = TestUtil::nextInt(random, 3, 8);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  while (sb->length() < maxLength) {
    if (sb->length() > 0) {
      sb->append(L' ');
    }
    int wordLength = -1;
    while (wordLength < 0) {
      wordLength = static_cast<int>(random->nextGaussian() * 3 + avgWordLength);
    }
    wordLength = min(wordLength, maxLength - sb->length());
    sb->append(randomSubString(random, wordLength, simple));
  }
  return sb->toString();
}

wstring TestUtil::randomSubString(shared_ptr<Random> random, int wordLength,
                                  bool simple)
{
  if (wordLength == 0) {
    return L"";
  }

  int evilness = TestUtil::nextInt(random, 0, 20);

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  while (sb->length() < wordLength) {
    if (simple) {
      sb->append(random->nextBoolean()
                     ? TestUtil::randomSimpleString(random, wordLength)
                     : TestUtil::randomHtmlishString(random, wordLength));
    } else {
      if (evilness < 10) {
        sb->append(TestUtil::randomSimpleString(random, wordLength));
      } else if (evilness < 15) {
        assert(sb->length() == 0); // we should always get wordLength back!
        sb->append(TestUtil::randomRealisticUnicodeString(random, wordLength,
                                                          wordLength));
      } else if (evilness == 16) {
        sb->append(TestUtil::randomHtmlishString(random, wordLength));
      } else if (evilness == 17) {
        // gives a lot of punctuation
        sb->append(TestUtil::randomRegexpishString(random, wordLength));
      } else {
        sb->append(TestUtil::randomUnicodeString(random, wordLength));
      }
    }
  }
  if (sb->length() > wordLength) {
    sb->setLength(wordLength);
    if (Character::isHighSurrogate(sb->charAt(wordLength - 1))) {
      sb->setLength(wordLength - 1);
    }
  }

  if (random->nextInt(17) == 0) {
    // mix up case
    wstring mixedUp =
        TestUtil::randomlyRecaseCodePoints(random, sb->toString());
    assert(mixedUp.length() == sb->length());
    return mixedUp;
  } else {
    return sb->toString();
  }
}

wstring TestUtil::bytesRefToString(shared_ptr<BytesRef> br)
{
  if (br == nullptr) {
    return L"(null)";
  } else {
    try {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return br->utf8ToString() + L" " + br->toString();
    }
    // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
    catch (AssertionError | invalid_argument t) {
      // If BytesRef isn't actually UTF8, or it's eg a
      // prefix of UTF8 that ends mid-unicode-char, we
      // fallback to hex:
      // C++ TODO: There is no native C++ equivalent to 'toString':
      return br->toString();
    }
  }
}

shared_ptr<RAMDirectory>
TestUtil::ramCopyOf(shared_ptr<Directory> dir) 
{
  shared_ptr<RAMDirectory> ram = make_shared<RAMDirectory>();
  for (auto file : dir->listAll()) {
    if (file.startsWith(IndexFileNames::SEGMENTS) ||
        IndexFileNames::CODEC_FILE_PATTERN->matcher(file).matches()) {
      ram->copyFrom(dir, file, file, IOContext::DEFAULT);
    }
  }
  return ram;
}

bool TestUtil::hasWindowsFS(shared_ptr<Directory> dir)
{
  dir = FilterDirectory::unwrap(dir);
  if (std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr) {
    shared_ptr<Path> path =
        (std::static_pointer_cast<FSDirectory>(dir))->getDirectory();
    shared_ptr<FileSystem> fs = path->getFileSystem();
    while (std::dynamic_pointer_cast<FilterFileSystem>(fs) != nullptr) {
      shared_ptr<FilterFileSystem> ffs =
          std::static_pointer_cast<FilterFileSystem>(fs);
      if (std::dynamic_pointer_cast<WindowsFS>(ffs->getParent()) != nullptr) {
        return true;
      }
      fs = ffs->getDelegate();
    }
  }

  return false;
}

bool TestUtil::hasWindowsFS(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = path->getFileSystem();
  while (std::dynamic_pointer_cast<FilterFileSystem>(fs) != nullptr) {
    shared_ptr<FilterFileSystem> ffs =
        std::static_pointer_cast<FilterFileSystem>(fs);
    if (std::dynamic_pointer_cast<WindowsFS>(ffs->getParent()) != nullptr) {
      return true;
    }
    fs = ffs->getDelegate();
  }

  return false;
}

bool TestUtil::hasVirusChecker(shared_ptr<Directory> dir)
{
  dir = FilterDirectory::unwrap(dir);
  if (std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr) {
    return hasVirusChecker(
        (std::static_pointer_cast<FSDirectory>(dir))->getDirectory());
  } else {
    return false;
  }
}

bool TestUtil::hasVirusChecker(shared_ptr<Path> path)
{
  shared_ptr<FileSystem> fs = path->getFileSystem();
  while (std::dynamic_pointer_cast<FilterFileSystem>(fs) != nullptr) {
    shared_ptr<FilterFileSystem> ffs =
        std::static_pointer_cast<FilterFileSystem>(fs);
    if (std::dynamic_pointer_cast<VirusCheckingFS>(ffs->getParent()) !=
        nullptr) {
      return true;
    }
    fs = ffs->getDelegate();
  }

  return false;
}

bool TestUtil::disableVirusChecker(shared_ptr<Directory> in_)
{
  shared_ptr<Directory> dir = FilterDirectory::unwrap(in_);
  if (std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr) {

    shared_ptr<FileSystem> fs = (std::static_pointer_cast<FSDirectory>(dir))
                                    ->getDirectory()
                                    ->getFileSystem();
    while (std::dynamic_pointer_cast<FilterFileSystem>(fs) != nullptr) {
      shared_ptr<FilterFileSystem> ffs =
          std::static_pointer_cast<FilterFileSystem>(fs);
      if (std::dynamic_pointer_cast<VirusCheckingFS>(ffs->getParent()) !=
          nullptr) {
        shared_ptr<VirusCheckingFS> vfs =
            std::static_pointer_cast<VirusCheckingFS>(ffs->getParent());
        bool isEnabled = vfs->isEnabled();
        vfs->disable();
        return isEnabled;
      }
      fs = ffs->getDelegate();
    }
  }

  return false;
}

void TestUtil::enableVirusChecker(shared_ptr<Directory> in_)
{
  shared_ptr<Directory> dir = FilterDirectory::unwrap(in_);
  if (std::dynamic_pointer_cast<FSDirectory>(dir) != nullptr) {

    shared_ptr<FileSystem> fs = (std::static_pointer_cast<FSDirectory>(dir))
                                    ->getDirectory()
                                    ->getFileSystem();
    while (std::dynamic_pointer_cast<FilterFileSystem>(fs) != nullptr) {
      shared_ptr<FilterFileSystem> ffs =
          std::static_pointer_cast<FilterFileSystem>(fs);
      if (std::dynamic_pointer_cast<VirusCheckingFS>(ffs->getParent()) !=
          nullptr) {
        shared_ptr<VirusCheckingFS> vfs =
            std::static_pointer_cast<VirusCheckingFS>(ffs->getParent());
        vfs->enable();
        return;
      }
      fs = ffs->getDelegate();
    }
  }
}
} // namespace org::apache::lucene::util