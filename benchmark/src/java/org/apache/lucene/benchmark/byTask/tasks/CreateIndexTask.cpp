using namespace std;

#include "CreateIndexTask.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/codecs/Codec.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/ConcurrentMergeScheduler.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexCommit.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexDeletionPolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/LogMergePolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MergePolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MergeScheduler.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoDeletionPolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoMergePolicy.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/NoMergeScheduler.h"
#include "../PerfRunData.h"
#include "../utils/Config.h"
#include "OpenIndexTask.h"

namespace org::apache::lucene::benchmark::byTask::tasks
{
using PerfRunData = org::apache::lucene::benchmark::byTask::PerfRunData;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Codec = org::apache::lucene::codecs::Codec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using Lucene70Codec = org::apache::lucene::codecs::lucene70::Lucene70Codec;
using ConcurrentMergeScheduler =
    org::apache::lucene::index::ConcurrentMergeScheduler;
using IndexCommit = org::apache::lucene::index::IndexCommit;
using IndexDeletionPolicy = org::apache::lucene::index::IndexDeletionPolicy;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using LogMergePolicy = org::apache::lucene::index::LogMergePolicy;
using MergePolicy = org::apache::lucene::index::MergePolicy;
using MergeScheduler = org::apache::lucene::index::MergeScheduler;
using NoDeletionPolicy = org::apache::lucene::index::NoDeletionPolicy;
using NoMergePolicy = org::apache::lucene::index::NoMergePolicy;
using NoMergeScheduler = org::apache::lucene::index::NoMergeScheduler;

CreateIndexTask::CreateIndexTask(shared_ptr<PerfRunData> runData)
    : PerfTask(runData)
{
}

shared_ptr<IndexDeletionPolicy>
CreateIndexTask::getIndexDeletionPolicy(shared_ptr<Config> config)
{
  wstring deletionPolicyName =
      config->get(L"deletion.policy",
                  L"org.apache.lucene.index.KeepOnlyLastCommitDeletionPolicy");
  if (deletionPolicyName == NoDeletionPolicy::typeid->getName()) {
    return NoDeletionPolicy::INSTANCE;
  } else {
    try {
      return type_info::forName(deletionPolicyName)
          .asSubclass(IndexDeletionPolicy::typeid)
          .newInstance();
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("unable to instantiate class
      // '" + deletionPolicyName + "' as IndexDeletionPolicy", e);
      throw runtime_error(L"unable to instantiate class '" +
                          deletionPolicyName + L"' as IndexDeletionPolicy");
    }
  }
}

int CreateIndexTask::doLogic() 
{
  shared_ptr<PerfRunData> runData = getRunData();
  shared_ptr<Config> config = runData->getConfig();
  runData->setIndexWriter(
      configureWriter(config, runData, OpenMode::CREATE, nullptr));
  return 1;
}

shared_ptr<IndexWriterConfig> CreateIndexTask::createWriterConfig(
    shared_ptr<Config> config, shared_ptr<PerfRunData> runData, OpenMode mode,
    shared_ptr<IndexCommit> commit)
{
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecation")
  // org.apache.lucene.index.IndexWriterConfig iwConf = new
  // org.apache.lucene.index.IndexWriterConfig(runData.getAnalyzer());
  shared_ptr<IndexWriterConfig> iwConf =
      make_shared<IndexWriterConfig>(runData->getAnalyzer());
  iwConf->setOpenMode(mode);
  shared_ptr<IndexDeletionPolicy> indexDeletionPolicy =
      getIndexDeletionPolicy(config);
  iwConf->setIndexDeletionPolicy(indexDeletionPolicy);
  if (commit != nullptr) {
    iwConf->setIndexCommit(commit);
  }

  const wstring mergeScheduler = config->get(
      L"merge.scheduler", L"org.apache.lucene.index.ConcurrentMergeScheduler");
  if (mergeScheduler == NoMergeScheduler::typeid->getName()) {
    iwConf->setMergeScheduler(NoMergeScheduler::INSTANCE);
  } else {
    try {
      iwConf->setMergeScheduler(type_info::forName(mergeScheduler)
                                    .asSubclass(MergeScheduler::typeid)
                                    .newInstance());
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("unable to instantiate class
      // '" + mergeScheduler + "' as merge scheduler", e);
      throw runtime_error(L"unable to instantiate class '" + mergeScheduler +
                          L"' as merge scheduler");
    }

    if (mergeScheduler == L"org.apache.lucene.index.ConcurrentMergeScheduler") {
      shared_ptr<ConcurrentMergeScheduler> cms =
          std::static_pointer_cast<ConcurrentMergeScheduler>(
              iwConf->getMergeScheduler());
      int maxThreadCount =
          config->get(L"concurrent.merge.scheduler.max.thread.count",
                      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
      int maxMergeCount =
          config->get(L"concurrent.merge.scheduler.max.merge.count",
                      ConcurrentMergeScheduler::AUTO_DETECT_MERGES_AND_THREADS);
      cms->setMaxMergesAndThreads(maxMergeCount, maxThreadCount);
    }
  }

  const wstring defaultCodec = config->get(L"default.codec", L"");
  if (defaultCodec != L"") {
    try {
      type_info clazz =
          type_info::forName(defaultCodec).asSubclass(Codec::typeid);
      iwConf->setCodec(clazz.newInstance());
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Couldn't instantiate Codec:
      // " + defaultCodec, e);
      throw runtime_error(L"Couldn't instantiate Codec: " + defaultCodec);
    }
  }

  const wstring postingsFormat = config->get(L"codec.postingsFormat", L"");
  if (defaultCodec == L"" && postingsFormat != L"") {
    try {
      shared_ptr<PostingsFormat> *const postingsFormatChosen =
          PostingsFormat::forName(postingsFormat);
      iwConf->setCodec(
          make_shared<Lucene70CodecAnonymousInnerClass>(postingsFormatChosen));
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Couldn't instantiate
      // Postings Format: " + postingsFormat, e);
      throw runtime_error(L"Couldn't instantiate Postings Format: " +
                          postingsFormat);
    }
  }

  const wstring mergePolicy = config->get(
      L"merge.policy", L"org.apache.lucene.index.LogByteSizeMergePolicy");
  bool isCompound = config->get(L"compound", true);
  iwConf->setUseCompoundFile(isCompound);
  if (mergePolicy == NoMergePolicy::typeid->getName()) {
    iwConf->setMergePolicy(NoMergePolicy::INSTANCE);
  } else {
    try {
      iwConf->setMergePolicy(type_info::forName(mergePolicy)
                                 .asSubclass(MergePolicy::typeid)
                                 .newInstance());
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("unable to instantiate class
      // '" + mergePolicy + "' as merge policy", e);
      throw runtime_error(L"unable to instantiate class '" + mergePolicy +
                          L"' as merge policy");
    }
    iwConf->getMergePolicy()->setNoCFSRatio(isCompound ? 1.0 : 0.0);
    if (std::dynamic_pointer_cast<LogMergePolicy>(iwConf->getMergePolicy()) !=
        nullptr) {
      shared_ptr<LogMergePolicy> logMergePolicy =
          std::static_pointer_cast<LogMergePolicy>(iwConf->getMergePolicy());
      logMergePolicy->setMergeFactor(
          config->get(L"merge.factor", OpenIndexTask::DEFAULT_MERGE_PFACTOR));
    }
  }
  constexpr double ramBuffer =
      config->get(L"ram.flush.mb", OpenIndexTask::DEFAULT_RAM_FLUSH_MB);
  constexpr int maxBuffered =
      config->get(L"max.buffered", OpenIndexTask::DEFAULT_MAX_BUFFERED);
  if (maxBuffered == IndexWriterConfig::DISABLE_AUTO_FLUSH) {
    iwConf->setRAMBufferSizeMB(ramBuffer);
    iwConf->setMaxBufferedDocs(maxBuffered);
  } else {
    iwConf->setMaxBufferedDocs(maxBuffered);
    iwConf->setRAMBufferSizeMB(ramBuffer);
  }

  return iwConf;
}

CreateIndexTask::Lucene70CodecAnonymousInnerClass::
    Lucene70CodecAnonymousInnerClass(
        shared_ptr<PostingsFormat> postingsFormatChosen)
{
  this->postingsFormatChosen = postingsFormatChosen;
}

shared_ptr<PostingsFormat>
CreateIndexTask::Lucene70CodecAnonymousInnerClass::getPostingsFormatForField(
    const wstring &field)
{
  return postingsFormatChosen;
}

shared_ptr<IndexWriter> CreateIndexTask::configureWriter(
    shared_ptr<Config> config, shared_ptr<PerfRunData> runData, OpenMode mode,
    shared_ptr<IndexCommit> commit) 
{
  shared_ptr<IndexWriterConfig> iwc =
      createWriterConfig(config, runData, mode, commit);
  wstring infoStreamVal = config->get(L"writer.info.stream", L"");
  if (infoStreamVal != L"") {
    if (infoStreamVal == L"SystemOut") {
      iwc->setInfoStream(System::out);
    } else if (infoStreamVal == L"SystemErr") {
      iwc->setInfoStream(System::err);
    } else {
      shared_ptr<Path> f = Paths->get(infoStreamVal);
      iwc->setInfoStream(make_shared<PrintStream>(
          make_shared<BufferedOutputStream>(Files::newOutputStream(f)), false,
          Charset::defaultCharset().name()));
    }
  }
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(runData->getDirectory(), iwc);
  return writer;
}
} // namespace org::apache::lucene::benchmark::byTask::tasks