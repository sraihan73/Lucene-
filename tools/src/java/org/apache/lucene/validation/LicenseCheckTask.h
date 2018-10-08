#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::validation
{

using org::apache::tools::ant::BuildException;
using org::apache::tools::ant::Project;
using org::apache::tools::ant::Task;
using org::apache::tools::ant::types::Mapper;
using org::apache::tools::ant::types::ResourceCollection;
using org::apache::tools::ant::types::resources::Resources;
using org::apache::tools::ant::util::FileNameMapper;

/**
 * An ANT task that verifies if JAR file have associated <tt>LICENSE</tt>,
 * <tt>NOTICE</tt>, and <tt>sha1</tt> files.
 */
class LicenseCheckTask : public Task
{
  GET_CLASS_NAME(LicenseCheckTask)

public:
  static const std::wstring CHECKSUM_TYPE;

private:
  static constexpr int CHECKSUM_BUFFER_SIZE = 8 * 1024;
  static constexpr int CHECKSUM_BYTE_MASK = 0xFF;
  static const std::wstring FAILURE_MESSAGE;

  std::shared_ptr<Pattern> skipRegexChecksum;
  bool skipSnapshotsChecksum = false;
  bool skipChecksum = false;

  /**
   * All JAR files to check.
   */
  std::shared_ptr<Resources> jarResources = std::make_shared<Resources>();

  /**
   * Directory containing licenses
   */
  std::shared_ptr<File> licenseDirectory;

  /**
   * License file mapper.
   */
  std::shared_ptr<FileNameMapper> licenseMapper;

  /**
   * A logging level associated with verbose logging.
   */
  int verboseLevel = Project::MSG_VERBOSE;

  /**
   * Failure flag.
   */
  bool failures = false;

  /**
   * Adds a set of JAR resources to check.
   */
public:
  virtual void add(std::shared_ptr<ResourceCollection> rc);

  /**
   * Adds a license mapper.
   */
  virtual void addConfiguredLicenseMapper(std::shared_ptr<Mapper> mapper);

  virtual void setVerbose(bool verbose);

  virtual void setLicenseDirectory(std::shared_ptr<File> file);

  virtual void setSkipSnapshotsChecksum(bool skipSnapshotsChecksum);

  virtual void setSkipChecksum(bool skipChecksum);

  virtual void setSkipRegexChecksum(const std::wstring &skipRegexChecksum);

  /**
   * Execute the task.
   */
  void execute()  override;

  /**
   * Process all JARs.
   */
private:
  void processJars();

  /**
   * Check a single JAR file.
   */
  bool checkJarFile(std::shared_ptr<File> jarFile);

  static std::wstring createChecksumString(std::deque<char> &digest);
  static std::wstring readChecksumFile(std::shared_ptr<File> f);

  static bool matchesRegexChecksum(std::shared_ptr<File> jarFile,
                                   std::shared_ptr<Pattern> skipRegexChecksum);

protected:
  std::shared_ptr<LicenseCheckTask> shared_from_this()
  {
    return std::static_pointer_cast<LicenseCheckTask>(
        org.apache.tools.ant.Task::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/validation/
