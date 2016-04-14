// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2016.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Adam Tenderholt $
// $Author: Adam Tenderholt $
// --------------------------------------------------------------------------
//

#include <list>
#include <sstream>
#include <fstream>

#include <OpenMS/DATASTRUCTURES/String.h>
#include <OpenMS/CONCEPT/ClassTest.h>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleVersionDialog.h>

#include "PeakInvestigator_test_config.h"

using namespace OpenMS;

START_TEST(ConsoleVersionDialog, "$Id")

START_SECTION(static String formatLine(int i, String version))
  String line = ConsoleVersionDialog::formatLine(0, "1.2");
  TEST_STRING_EQUAL(line, "\t(0) 1.2\n");
END_SECTION

START_SECTION(exec() cancel)
  std::list<std::string> versions;
  versions.push_back("1.2");
  versions.push_back("1.0");

  std::stringstream istream, ostream;
  istream << 0;

  ConsoleVersionDialog dialog("Foobar", versions, "1.2", "", &istream, &ostream);
  bool retval = dialog.exec();
  TEST_EQUAL(retval, false);

  std::fstream file(GET_TEST_DATA_PATH("ConsoleVersionDialog_output_ok.txt"));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  TEST_STRING_EQUAL(ostream.str(), contents.str());
END_SECTION

START_SECTION(exec() select first)
  std::list<std::string> versions;
  versions.push_back("1.2");
  versions.push_back("1.0");

  std::stringstream istream, ostream;
  istream << 1;

  ConsoleVersionDialog dialog("Foobar", versions, "1.2", "", &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, true);
  TEST_STRING_EQUAL(dialog.getSelectedVersion(), "1.2");

  std::fstream file(GET_TEST_DATA_PATH("ConsoleVersionDialog_output_ok.txt"));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  TEST_STRING_EQUAL(ostream.str(), contents.str());
END_SECTION

START_SECTION(exec() select second)
  std::list<std::string> versions;
  versions.push_back("1.2");
  versions.push_back("1.0");

  std::stringstream istream, ostream;
  istream << 2;

  ConsoleVersionDialog dialog("Foobar", versions, "1.2", "", &istream, &ostream);
  bool retval = dialog.exec();
  TEST_EQUAL(retval, true);
  TEST_STRING_EQUAL(dialog.getSelectedVersion(), "1.0");

  std::fstream file(GET_TEST_DATA_PATH("ConsoleVersionDialog_output_ok.txt"));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  TEST_STRING_EQUAL(ostream.str(), contents.str());
END_SECTION

START_SECTION(exec() select invalid number)
  std::list<std::string> versions;
  versions.push_back("1.2");
  versions.push_back("1.0");

  std::stringstream istream, ostream;
  istream << 12 << "\n" << 0;

  ConsoleVersionDialog dialog("Foobar", versions, "1.2", "", &istream, &ostream);
  bool retval = dialog.exec();
  TEST_EQUAL(retval, false);

  std::fstream file(GET_TEST_DATA_PATH("ConsoleVersionDialog_output_invalid_number.txt"));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  TEST_STRING_EQUAL(ostream.str(), contents.str());
END_SECTION

START_SECTION(exec() select string)
  std::list<std::string> versions;
  versions.push_back("1.2");
  versions.push_back("1.0");

  std::stringstream istream, ostream;
  istream << "1.2";

  ConsoleVersionDialog dialog("Foobar", versions, "1.2", "", &istream, &ostream);
  bool retval = dialog.exec();
  TEST_EQUAL(retval, false);

  std::fstream file(GET_TEST_DATA_PATH("ConsoleVersionDialog_output_invalid_entry.txt"));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  TEST_STRING_EQUAL(ostream.str(), contents.str());
END_SECTION


END_TEST
