using namespace std;

#include "LuceneFixedGap.h"

namespace org::apache::lucene::codecs::blockterms
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
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
using Lucene50PostingsFormat =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsFormat;
using Lucene50PostingsReader =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsReader;
using Lucene50PostingsWriter =
    org::apache::lucene::codecs::lucene50::Lucene50PostingsWriter;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

LuceneFixedGap::LuceneFixedGap()
    : LuceneFixedGap(FixedGapTermsIndexWriter::DEFAULT_TERM_INDEX_INTERVAL)
{
}

LuceneFixedGap::LuceneFixedGap(int termIndexInterval)
    : org::apache::lucene::codecs::PostingsFormat(L"LuceneFixedGap"),
      termIndexInterval(termIndexInterval)
{
}

shared_ptr<FieldsConsumer> LuceneFixedGap::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  shared_ptr<PostingsWriterBase> docs =
      make_shared<Lucene50PostingsWriter>(state);

  // TODO: should we make the terms index more easily
  // pluggable?  Ie so that this codec would record which
  // index impl was used, and switch on loading?
  // Or... you must make a new Codec for this?
  shared_ptr<TermsIndexWriterBase> indexWriter;
  bool success = false;
  try {
    indexWriter =
        make_shared<FixedGapTermsIndexWriter>(state, termIndexInterval);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      delete docs;
    }
  }

  success = false;
  try {
    // Must use BlockTermsWriter (not BlockTree) because
    // BlockTree doens't support ords (yet)...
    shared_ptr<FieldsConsumer> ret =
        make_shared<BlockTermsWriter>(indexWriter, state, docs);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        delete docs;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        delete indexWriter;
      }
    }
  }
}

shared_ptr<FieldsProducer> LuceneFixedGap::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  shared_ptr<PostingsReaderBase> postings =
      make_shared<Lucene50PostingsReader>(state);
  shared_ptr<TermsIndexReaderBase> indexReader;

  bool success = false;
  try {
    indexReader = make_shared<FixedGapTermsIndexReader>(state);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      delete postings;
    }
  }

  success = false;
  try {
    shared_ptr<FieldsProducer> ret =
        make_shared<BlockTermsReader>(indexReader, postings, state);
    success = true;
    return ret;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        delete postings;
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        delete indexReader;
      }
    }
  }
}

const wstring LuceneFixedGap::FREQ_EXTENSION = L"frq";
const wstring LuceneFixedGap::PROX_EXTENSION = L"prx";
} // namespace org::apache::lucene::codecs::blockterms