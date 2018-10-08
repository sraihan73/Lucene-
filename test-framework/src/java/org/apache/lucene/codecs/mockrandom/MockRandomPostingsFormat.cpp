using namespace std;

#include "MockRandomPostingsFormat.h"

namespace org::apache::lucene::codecs::mockrandom
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using TermStats = org::apache::lucene::codecs::TermStats;
using BlockTermsReader =
    org::apache::lucene::codecs::blockterms::BlockTermsReader;
using BlockTermsWriter =
    org::apache::lucene::codecs::blockterms::BlockTermsWriter;
using FixedGapTermsIndexReader =
    org::apache::lucene::codecs::blockterms::FixedGapTermsIndexReader;
using FixedGapTermsIndexWriter =
    org::apache::lucene::codecs::blockterms::FixedGapTermsIndexWriter;
using TermsIndexReaderBase =
    org::apache::lucene::codecs::blockterms::TermsIndexReaderBase;
using TermsIndexWriterBase =
    org::apache::lucene::codecs::blockterms::TermsIndexWriterBase;
using VariableGapTermsIndexReader =
    org::apache::lucene::codecs::blockterms::VariableGapTermsIndexReader;
using VariableGapTermsIndexWriter =
    org::apache::lucene::codecs::blockterms::VariableGapTermsIndexWriter;
using BlockTreeTermsReader =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsReader;
using BlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktree::BlockTreeTermsWriter;
using OrdsBlockTreeTermsReader =
    org::apache::lucene::codecs::blocktreeords::OrdsBlockTreeTermsReader;
using OrdsBlockTreeTermsWriter =
    org::apache::lucene::codecs::blocktreeords::OrdsBlockTreeTermsWriter;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using Lucene50PostingsWriter =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsWriter;
using FSTOrdTermsReader =
    org::apache::lucene::codecs::memory::FSTOrdTermsReader;
using FSTOrdTermsWriter =
    org::apache::lucene::codecs::memory::FSTOrdTermsWriter;
using FSTTermsReader = org::apache::lucene::codecs::memory::FSTTermsReader;
using FSTTermsWriter = org::apache::lucene::codecs::memory::FSTTermsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
const wstring MockRandomPostingsFormat::SEED_EXT = L"sd";

MockRandomPostingsFormat::MockRandomPostingsFormat()
    : MockRandomPostingsFormat(nullptr)
{
  // This ctor should *only* be used at read-time: get NPE if you use it!
}

MockRandomPostingsFormat::MockRandomPostingsFormat(shared_ptr<Random> random)
    : org::apache::lucene::codecs::PostingsFormat(L"MockRandom")
{
  if (random == nullptr) {
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    this->seedRandom =
        make_shared<RandomAnonymousInnerClass>(shared_from_this());
  } else {
    this->seedRandom = make_shared<Random>(random->nextLong());
  }
}

MockRandomPostingsFormat::RandomAnonymousInnerClass::RandomAnonymousInnerClass(
    shared_ptr<MockRandomPostingsFormat> outerInstance)
    : java::util::Random(0LL)
{
  this->outerInstance = outerInstance;
}

int MockRandomPostingsFormat::RandomAnonymousInnerClass::next(int arg0)
{
  throw make_shared<IllegalStateException>(
      L"Please use MockRandomPostingsFormat(Random)");
}

shared_ptr<FieldsConsumer> MockRandomPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  int minSkipInterval;
  if (state->segmentInfo->maxDoc() > 1000000) {
    // Test2BPostings can OOME otherwise:
    minSkipInterval = 3;
  } else {
    minSkipInterval = 2;
  }

  // we pull this before the seed intentionally: because it's not consumed at
  // runtime (the skipInterval is written into postings header). NOTE: Currently
  // not passed to postings writer.
  //       before, it was being passed in wrongly as acceptableOverhead!
  int skipInterval = TestUtil::nextInt(seedRandom, minSkipInterval, 10);

  if (LuceneTestCase::VERBOSE) {
    wcout << L"MockRandomCodec: skipInterval=" << skipInterval << endl;
  }

  constexpr int64_t seed = seedRandom->nextLong();

  if (LuceneTestCase::VERBOSE) {
    wcout << L"MockRandomCodec: writing to seg=" << state->segmentInfo->name
          << L" formatID=" << state->segmentSuffix << L" seed=" << seed << endl;
  }

  const wstring seedFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, SEED_EXT);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try(org.apache.lucene.store.IndexOutput out =
  // state.directory.createOutput(seedFileName, state.context))
  {
    org::apache::lucene::store::IndexOutput out =
        state->directory->createOutput(seedFileName, state->context);
    CodecUtil::writeIndexHeader(out, L"MockRandomSeed", 0,
                                state->segmentInfo->getId(),
                                state->segmentSuffix);
    out->writeLong(seed);
    CodecUtil::writeFooter(out);
  }

  shared_ptr<Random> *const random = make_shared<Random>(seed);

  random->nextInt(); // consume a random for buffersize

  shared_ptr<PostingsWriterBase> postingsWriter =
      make_shared<Lucene50PostingsWriter>(state);

  shared_ptr<FieldsConsumer> *const fields;
  constexpr int t1 = random->nextInt(5);

  if (t1 == 0) {
    bool success = false;
    try {
      fields = make_shared<FSTTermsWriter>(state, postingsWriter);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsWriter;
      }
    }
  } else if (t1 == 1) {
    bool success = false;
    try {
      fields = make_shared<FSTOrdTermsWriter>(state, postingsWriter);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsWriter;
      }
    }
  } else if (t1 == 2) {
    // Use BlockTree terms dict

    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: writing BlockTree terms dict" << endl;
    }

    // TODO: would be nice to allow 1 but this is very
    // slow to write
    constexpr int minTermsInBlock = TestUtil::nextInt(random, 2, 100);
    constexpr int maxTermsInBlock =
        max(2, (minTermsInBlock - 1) * 2 + random->nextInt(100));

    bool success = false;
    try {
      fields = make_shared<BlockTreeTermsWriter>(
          state, postingsWriter, minTermsInBlock, maxTermsInBlock);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsWriter;
      }
    }
  } else if (t1 == 3) {

    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: writing Block terms dict" << endl;
    }

    bool success = false;

    shared_ptr<TermsIndexWriterBase> *const indexWriter;
    try {
      if (random->nextBoolean()) {
        int termIndexInterval = TestUtil::nextInt(random, 1, 100);
        if (LuceneTestCase::VERBOSE) {
          wcout << L"MockRandomCodec: fixed-gap terms index (tii="
                << termIndexInterval << L")" << endl;
        }
        indexWriter =
            make_shared<FixedGapTermsIndexWriter>(state, termIndexInterval);
      } else {
        shared_ptr<VariableGapTermsIndexWriter::IndexTermSelector>
            *const selector;
        constexpr int n2 = random->nextInt(3);
        if (n2 == 0) {
          constexpr int tii = TestUtil::nextInt(random, 1, 100);
          selector =
              make_shared<VariableGapTermsIndexWriter::EveryNTermSelector>(tii);
          if (LuceneTestCase::VERBOSE) {
            wcout << L"MockRandomCodec: variable-gap terms index (tii=" << tii
                  << L")" << endl;
          }
        } else if (n2 == 1) {
          constexpr int docFreqThresh = TestUtil::nextInt(random, 2, 100);
          constexpr int tii = TestUtil::nextInt(random, 1, 100);
          selector = make_shared<
              VariableGapTermsIndexWriter::EveryNOrDocFreqTermSelector>(
              docFreqThresh, tii);
        } else {
          constexpr int64_t seed2 = random->nextLong();
          constexpr int gap = TestUtil::nextInt(random, 2, 40);
          if (LuceneTestCase::VERBOSE) {
            wcout << L"MockRandomCodec: random-gap terms index (max gap=" << gap
                  << L")" << endl;
          }
          selector = make_shared<IndexTermSelectorAnonymousInnerClass>(
              shared_from_this(), seed2, gap);
        }
        indexWriter = make_shared<VariableGapTermsIndexWriter>(state, selector);
      }
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsWriter;
      }
    }

    success = false;
    try {
      fields =
          make_shared<BlockTermsWriter>(indexWriter, state, postingsWriter);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        try {
          delete postingsWriter;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          delete indexWriter;
        }
      }
    }
  } else if (t1 == 4) {
    // Use OrdsBlockTree terms dict
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: writing OrdsBlockTree" << endl;
    }

    // TODO: would be nice to allow 1 but this is very
    // slow to write
    constexpr int minTermsInBlock = TestUtil::nextInt(random, 2, 100);
    constexpr int maxTermsInBlock =
        max(2, (minTermsInBlock - 1) * 2 + random->nextInt(100));

    bool success = false;
    try {
      fields = make_shared<OrdsBlockTreeTermsWriter>(
          state, postingsWriter, minTermsInBlock, maxTermsInBlock);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsWriter;
      }
    }

  } else {
    // BUG!
    throw make_shared<AssertionError>();
  }

  return fields;
}

MockRandomPostingsFormat::IndexTermSelectorAnonymousInnerClass::
    IndexTermSelectorAnonymousInnerClass(
        shared_ptr<MockRandomPostingsFormat> outerInstance, int64_t seed2,
        int gap)
{
  this->outerInstance = outerInstance;
  this->seed2 = seed2;
  this->gap = gap;
  rand = make_shared<Random>(seed2);
}

bool MockRandomPostingsFormat::IndexTermSelectorAnonymousInnerClass::
    isIndexTerm(shared_ptr<BytesRef> term, shared_ptr<TermStats> stats)
{
  return rand::nextInt(gap) == gap / 2;
}

void MockRandomPostingsFormat::IndexTermSelectorAnonymousInnerClass::newField(
    shared_ptr<FieldInfo> fieldInfo)
{
}

shared_ptr<FieldsProducer> MockRandomPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{

  const wstring seedFileName = IndexFileNames::segmentFileName(
      state->segmentInfo->name, state->segmentSuffix, SEED_EXT);
  shared_ptr<ChecksumIndexInput> *const in_ =
      state->directory->openChecksumInput(seedFileName, state->context);
  CodecUtil::checkIndexHeader(in_, L"MockRandomSeed", 0, 0,
                              state->segmentInfo->getId(),
                              state->segmentSuffix);
  constexpr int64_t seed = in_->readLong();
  CodecUtil::checkFooter(in_);
  if (LuceneTestCase::VERBOSE) {
    wcout << L"MockRandomCodec: reading from seg=" << state->segmentInfo->name
          << L" formatID=" << state->segmentSuffix << L" seed=" << seed << endl;
  }
  delete in_;

  shared_ptr<Random> *const random = make_shared<Random>(seed);

  int readBufferSize = TestUtil::nextInt(random, 1, 4096);
  if (LuceneTestCase::VERBOSE) {
    wcout << L"MockRandomCodec: readBufferSize=" << readBufferSize << endl;
  }

  shared_ptr<PostingsReaderBase> postingsReader =
      make_shared<Lucene50PostingsReader>(state);

  shared_ptr<FieldsProducer> *const fields;
  constexpr int t1 = random->nextInt(5);
  if (t1 == 0) {
    bool success = false;
    try {
      fields = make_shared<FSTTermsReader>(state, postingsReader);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsReader;
      }
    }
  } else if (t1 == 1) {
    bool success = false;
    try {
      fields = make_shared<FSTOrdTermsReader>(state, postingsReader);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsReader;
      }
    }
  } else if (t1 == 2) {
    // Use BlockTree terms dict
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: reading BlockTree terms dict" << endl;
    }

    bool success = false;
    try {
      fields = make_shared<BlockTreeTermsReader>(postingsReader, state);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsReader;
      }
    }
  } else if (t1 == 3) {

    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: reading Block terms dict" << endl;
    }
    shared_ptr<TermsIndexReaderBase> *const indexReader;
    bool success = false;
    try {
      constexpr bool doFixedGap = random->nextBoolean();

      // randomness diverges from writer, here:

      if (doFixedGap) {
        if (LuceneTestCase::VERBOSE) {
          wcout << L"MockRandomCodec: fixed-gap terms index" << endl;
        }
        indexReader = make_shared<FixedGapTermsIndexReader>(state);
      } else {
        constexpr int n2 = random->nextInt(3);
        if (n2 == 1) {
          random->nextInt();
        } else if (n2 == 2) {
          random->nextLong();
        }
        if (LuceneTestCase::VERBOSE) {
          wcout << L"MockRandomCodec: variable-gap terms index" << endl;
        }
        indexReader = make_shared<VariableGapTermsIndexReader>(state);
      }

      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsReader;
      }
    }

    success = false;
    try {
      fields =
          make_shared<BlockTermsReader>(indexReader, postingsReader, state);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        try {
          delete postingsReader;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          delete indexReader;
        }
      }
    }
  } else if (t1 == 4) {
    // Use OrdsBlockTree terms dict
    if (LuceneTestCase::VERBOSE) {
      wcout << L"MockRandomCodec: reading OrdsBlockTree terms dict" << endl;
    }

    bool success = false;
    try {
      fields = make_shared<OrdsBlockTreeTermsReader>(postingsReader, state);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (!success) {
        delete postingsReader;
      }
    }
  } else {
    // BUG!
    throw make_shared<AssertionError>();
  }

  return fields;
}
} // namespace org::apache::lucene::codecs::mockrandom