#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::util
{

/**
 * Use by certain classes to match version compatibility
 * across releases of Lucene.
 *
 * <p><b>WARNING</b>: When changing the version parameter
 * that you supply to components in Lucene, do not simply
 * change the version at search-time, but instead also adjust
 * your indexing code to match, and re-index.
 */
class Version final : public std::enable_shared_from_this<Version>
{
  GET_CLASS_NAME(Version)

  /** Match settings and bugs in Lucene's 6.0 release.
   * @deprecated (7.0.0) Use latest
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_0_0 = new
  // Version(6, 0, 0);
  static const std::shared_ptr<Version> LUCENE_6_0_0;

  /**
   * Match settings and bugs in Lucene's 6.0.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_0_1 = new
  // Version(6, 0, 1);
  static const std::shared_ptr<Version> LUCENE_6_0_1;

  /**
   * Match settings and bugs in Lucene's 6.1.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_1_0 = new
  // Version(6, 1, 0);
  static const std::shared_ptr<Version> LUCENE_6_1_0;

  /**
   * Match settings and bugs in Lucene's 6.2.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_2_0 = new
  // Version(6, 2, 0);
  static const std::shared_ptr<Version> LUCENE_6_2_0;

  /**
   * Match settings and bugs in Lucene's 6.2.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_2_1 = new
  // Version(6, 2, 1);
  static const std::shared_ptr<Version> LUCENE_6_2_1;

  /**
   * Match settings and bugs in Lucene's 6.3.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_3_0 = new
  // Version(6, 3, 0);
  static const std::shared_ptr<Version> LUCENE_6_3_0;

  /**
   * Match settings and bugs in Lucene's 6.4.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_4_0 = new
  // Version(6, 4, 0);
  static const std::shared_ptr<Version> LUCENE_6_4_0;

  /**
   * Match settings and bugs in Lucene's 6.4.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_4_1 = new
  // Version(6, 4, 1);
  static const std::shared_ptr<Version> LUCENE_6_4_1;

  /**
   * Match settings and bugs in Lucene's 6.4.2 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_4_2 = new
  // Version(6, 4, 2);
  static const std::shared_ptr<Version> LUCENE_6_4_2;

  /**
   * Match settings and bugs in Lucene's 6.5.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_5_0 = new
  // Version(6, 5, 0);
  static const std::shared_ptr<Version> LUCENE_6_5_0;

  /**
   * Match settings and bugs in Lucene's 6.5.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_5_1 = new
  // Version(6, 5, 1);
  static const std::shared_ptr<Version> LUCENE_6_5_1;

  /**
   * Match settings and bugs in Lucene's 6.6.0 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_6_0 = new
  // Version(6, 6, 0);
  static const std::shared_ptr<Version> LUCENE_6_6_0;

  /**
   * Match settings and bugs in Lucene's 6.6.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_6_1 = new
  // Version(6, 6, 1);
  static const std::shared_ptr<Version> LUCENE_6_6_1;

  /**
   * Match settings and bugs in Lucene's 6.6.2 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_6_2 = new
  // Version(6, 6, 2);
  static const std::shared_ptr<Version> LUCENE_6_6_2;

  /**
   * Match settings and bugs in Lucene's 6.6.3 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_6_3 = new
  // Version(6, 6, 3);
  static const std::shared_ptr<Version> LUCENE_6_6_3;

  /**
   * Match settings and bugs in Lucene's 6.6.4 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_6_6_4 = new
  // Version(6, 6, 4);
  static const std::shared_ptr<Version> LUCENE_6_6_4;

  /**
   * Match settings and bugs in Lucene's 7.0.0 release.
   * @deprecated (7.1.0) Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_0_0 = new
  // Version(7, 0, 0);
  static const std::shared_ptr<Version> LUCENE_7_0_0;

  /**
   * Match settings and bugs in Lucene's 7.0.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_0_1 = new
  // Version(7, 0, 1);
  static const std::shared_ptr<Version> LUCENE_7_0_1;

  /**
   * Match settings and bugs in Lucene's 7.1.0 release.
   * @deprecated (7.2.0) Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_1_0 = new
  // Version(7, 1, 0);
  static const std::shared_ptr<Version> LUCENE_7_1_0;

  /**
   * Match settings and bugs in Lucene's 7.2.0 release.
   * @deprecated (7.3.0) Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_2_0 = new
  // Version(7, 2, 0);
  static const std::shared_ptr<Version> LUCENE_7_2_0;

  /**
   * Match settings and bugs in Lucene's 7.2.1 release.
   * @deprecated (7.3.0) Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_2_1 = new
  // Version(7, 2, 1);
  static const std::shared_ptr<Version> LUCENE_7_2_1;

  /**
   * Match settings and bugs in Lucene's 7.3.0 release.
   * @deprecated (7.4.0) Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_3_0 = new
  // Version(7, 3, 0);
  static const std::shared_ptr<Version> LUCENE_7_3_0;

  /**
   * Match settings and bugs in Lucene's 7.3.1 release.
   * @deprecated Use latest
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_7_3_1 = new
  // Version(7, 3, 1);
  static const std::shared_ptr<Version> LUCENE_7_3_1;

  /**
   * Match settings and bugs in Lucene's 7.4.0 release.
   * <p>
   * Use this to get the latest &amp; greatest settings, bug
   * fixes, etc, for Lucene.
   */
  static const std::shared_ptr<Version> LUCENE_7_4_0;

  // To add a new version:
  //  * Only add above this comment
  //  * If the new version is the newest, change LATEST below and deprecate the
  //  previous LATEST

  /**
   * <p><b>WARNING</b>: if you use this setting, and then
   * upgrade to a newer release of Lucene, sizable changes
   * may happen.  If backwards compatibility is important
   * then you should instead explicitly specify an actual
   * version.
   * <p>
   * If you use this constant then you  may need to
   * <b>re-index all of your documents</b> when upgrading
   * Lucene, as the way text is indexed may have changed.
   * Additionally, you may need to <b>re-test your entire
   * application</b> to ensure it behaves as expected, as
   * some defaults may have changed and may break functionality
   * in your application.
   */
  static const std::shared_ptr<Version> LATEST;

  /**
   * Constant for backwards compatibility.
   * @deprecated Use {@link #LATEST}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final Version LUCENE_CURRENT =
  // LATEST;
  static const std::shared_ptr<Version> LUCENE_CURRENT;

  /**
   * Parse a version number of the form {@code "major.minor.bugfix.prerelease"}.
   *
   * Part {@code ".bugfix"} and part {@code ".prerelease"} are optional.
   * Note that this is forwards compatible: the parsed version does not have to
   * exist as a constant.
   *
   * @lucene.internal
   */
  static std::shared_ptr<Version>
  parse(const std::wstring &version) ;

  /**
   * Parse the given version number as a constant or dot based version.
   * <p>This method allows to use {@code "LUCENE_X_Y"} constant names,
   * or version numbers in the format {@code "x.y.z"}.
   *
   * @lucene.internal
   */
  static std::shared_ptr<Version>
  parseLeniently(const std::wstring &version) ;

  /** Returns a new version based on raw numbers
   *
   *  @lucene.internal */
  static std::shared_ptr<Version> fromBits(int major, int minor, int bugfix);

  /** Major version, the difference between stable and trunk */
  const int major;
  /** Minor version, incremented within the stable branch */
  const int minor;
  /** Bugfix number, incremented on release branches */
  const int bugfix;
  /** Prerelease version, currently 0 (alpha), 1 (beta), or 2 (final) */
  const int prerelease;

  // stores the version pieces, with most significant pieces in high bits
  // ie:  | 1 byte | 1 byte | 1 byte |   2 bits   |
  //         major   minor    bugfix   prerelease
private:
  const int encodedValue;

  Version(int major, int minor, int bugfix);

  Version(int major, int minor, int bugfix, int prerelease);

  /**
   * Returns true if this version is the same or after the version from the
   * argument.
   */
public:
  bool onOrAfter(std::shared_ptr<Version> other);

  virtual std::wstring toString();

  virtual bool equals(std::any o);

  // Used only by assert:
private:
  bool encodedIsValid();

public:
  virtual int hashCode();
};

} // #include  "core/src/java/org/apache/lucene/util/
