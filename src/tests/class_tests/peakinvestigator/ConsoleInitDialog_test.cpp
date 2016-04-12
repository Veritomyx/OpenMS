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

#include <PeakInvestigator/Actions/InitAction.h>

#include <OpenMS/DATASTRUCTURES/String.h>
#include <OpenMS/CONCEPT/ClassTest.h>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleInitDialog.h>

#include "PeakInvestigator_test_config.h"

using namespace OpenMS;
using namespace Veritomyx::PeakInvestigator;

EstimatedCosts buildEstimatedCosts()
{
  JobAttributes attrs;
  attrs.min_mass = 100;
  attrs.max_mass = 2000;
  attrs.max_points = 12345;

  InitAction action("username", "password", 1234, "1.2", 10, attrs);
  action.processResponse(InitAction::EXAMPLE_RESPONSE);
  EstimatedCosts costs = action.getEstimatedCosts();

  costs["TOF"]["RTO-8"] = 1234.50;
  costs["IonTrap"]["RTO-8"] = 85.0;
  costs["Orbitrap"]["RTO-8"] = 300.12;

  return costs;
}

std::string getFileContents(String filename)
{
  std::fstream file(GET_TEST_DATA_PATH(filename));
  std::stringstream contents;
  contents << file.rdbuf();
  file.close();

  return contents.str();
}

START_TEST(ConsoleInitDialog, "$Id")

//-----------------------------------------------------------------------------
START_SECTION(static String formatTableLine(String, ResponseTimeCosts))
  ResponseTimeCosts costs;
  costs["RTO-24"] = 27.60;
  costs["RTO-8"] = 36.22;

  String line = "\n";
  line += ConsoleInitDialog::formatTableLine("TOF", costs);
  line += "\n";

  String expected =
      "\n"
      "\tTOF        27.60   36.22\n"
      "\n";

  TEST_STRING_EQUAL(line, expected);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(static String formatTableHeader(std::list<std::string>))
  std::list<std::string> RTOs;
  RTOs.push_back("RTO-24");
  RTOs.push_back("RTO-8");

  String line = "\n";
  line += ConsoleInitDialog::formatTableHeader(RTOs);
  line += "\n";

  String expected =
      "\n"
      "\t          RTO-24   RTO-8\n"
      "\t          ------   -----\n"
      "\n";

  TEST_STRING_EQUAL(line, expected);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(static String formatTable(EstimatedCosts))
  EstimatedCosts costs = buildEstimatedCosts();

  String line = "\n";
  line += ConsoleInitDialog::formatTable(costs);
  line += "\n";

  String expected = "\n";
  expected += getFileContents("ConsoleInitDialog_output_table.txt");
  expected += "\n";

  TEST_STRING_EQUAL(line, expected);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(exec() with Cancel)
  EstimatedCosts costs = buildEstimatedCosts();

  std::stringstream istream, ostream;
  istream << 0;

  ConsoleInitDialog dialog("RTOs", costs, 10087.00, &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, false);

  String contents = getFileContents("ConsoleInitDialog_output_ok.txt");

  TEST_STRING_EQUAL(ostream.str(), contents);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(exec() with select first)
  EstimatedCosts costs = buildEstimatedCosts();

  std::stringstream istream, ostream;
  istream << 1;

  ConsoleInitDialog dialog("RTOs", costs, 10087.00, &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, true);
  TEST_STRING_EQUAL(dialog.getSelectedRTO(), "RTO-24");

  String contents = getFileContents("ConsoleInitDialog_output_ok.txt");

  TEST_STRING_EQUAL(ostream.str(), contents);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(exec() with select second)
  EstimatedCosts costs = buildEstimatedCosts();

  std::stringstream istream, ostream;
  istream << 2;

  ConsoleInitDialog dialog("RTOs", costs, 10087.00, &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, true);
  TEST_STRING_EQUAL(dialog.getSelectedRTO(), "RTO-8");

  String contents = getFileContents("ConsoleInitDialog_output_ok.txt");

  TEST_STRING_EQUAL(ostream.str(), contents);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(exec() with invalid number)
  EstimatedCosts costs = buildEstimatedCosts();

  std::stringstream istream, ostream;
  istream << 12 << 0;

  ConsoleInitDialog dialog("RTOs", costs, 10087.00, &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, false);

  String contents = getFileContents("ConsoleInitDialog_output_invalid_number.txt");

  TEST_STRING_EQUAL(ostream.str(), contents);
END_SECTION

//-----------------------------------------------------------------------------
START_SECTION(exec() with invalid entry)
  EstimatedCosts costs = buildEstimatedCosts();

  std::stringstream istream, ostream;
  istream << "RTO-8";

  ConsoleInitDialog dialog("RTOs", costs, 10087.00, &istream, &ostream);
  bool retval = dialog.exec();

  TEST_EQUAL(retval, false);

  String contents = getFileContents("ConsoleInitDialog_output_invalid_entry.txt");

  TEST_STRING_EQUAL(ostream.str(), contents);
END_SECTION

END_TEST
