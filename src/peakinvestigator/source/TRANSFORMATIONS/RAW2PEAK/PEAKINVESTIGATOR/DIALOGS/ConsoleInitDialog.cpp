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
// $Maintainer:$
// $Author: Adam Tenderholt $
// --------------------------------------------------------------------------

#include <sstream>
#include <iomanip>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleInitDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleDialogSelector.h>

#define INPUT *input_
#define OUTPUT *output_

namespace OpenMS
{
  ConsoleInitDialog::ConsoleInitDialog(String title, EstimatedCosts costs, double funds, std::istream* input, std::ostream* output)
    : AbstractInitDialog(title, costs, funds)
  {
    input_ = input;
    output_ = output;

    std::map<std::string, ResponseTimeCosts>::const_iterator iter = costs.begin();
    RTOs_ = (iter->second).getRTOs();
  }

  ConsoleInitDialog::~ConsoleInitDialog()
  {

  }

  bool ConsoleInitDialog::exec()
  {
    OUTPUT << "\n\nThe following Response Time Objectives (<= XX hours), and their associated\n";
    OUTPUT << "costs (in USD), are available:\n\n";

    while(true)
    {
      printMenu();

      unsigned int index;
      try
      {
        index = ConsoleDialogSelector::select(INPUT);
      }
      catch(...)
      {
        input_->ignore(std::numeric_limits<std::streamsize>::max()); // go to end of stream;
        OUTPUT << "\nInvalid entry. Please select from the following Response Time Objectives:\n\n";
        continue;
      }

      if (index == 0) // Cancel
      {
        return false;
      }
      else if (index <= RTOs_.size())
      {
        selectRTO_(index);
        return true;
      }
      else
      {
        OUTPUT << "\nInvalid number. Please select from the following Response Time Objectives:\n\n";
      }
    }
  }

  void ConsoleInitDialog::printMenu()
  {
    String table = formatTable(costs_);
    OUTPUT << table << "\n";

    std::list<std::string>::const_iterator iter;
    unsigned int i;
    for(i = 1, iter = RTOs_.begin(); iter != RTOs_.end(); ++i, ++iter)
    {
      String line = formatLine(i, *iter);
      OUTPUT << line;
    }

    OUTPUT << "\nPlease enter your choice (0 to cancel): ";
  }

  String ConsoleInitDialog::formatLine(int i, String RTO)
  {
    std::ostringstream stream;
    stream << "\t(" << i << ") " << RTO << "\n";
    return stream.str();
  }

  String ConsoleInitDialog::formatTableHeader(const std::list<std::string>& RTOs)
  {
    std::stringstream textstream, linestream;
    textstream << "\t" << std::setw(8) << " ";
    linestream << "\t" << std::setw(8) << " ";

    textstream.setf(std::ios::right);
    linestream.setf(std::ios::right);

    std::list<std::string>::const_iterator iter;
    for(iter = RTOs.begin(); iter != RTOs.end(); ++iter)
    {
      textstream << std::setw(8) << *iter;
      linestream << std::setw(8) << std::string(iter->size(), '-');
    }

    textstream << "\n";
    linestream << "\n";
    return textstream.str() + linestream.str();
  }

  String ConsoleInitDialog::formatTableLine(String instrument, const ResponseTimeCosts &instrumentCost)
  {
    std::stringstream stream;
    stream << "\t" << std::setw(8) << std::left << instrument;

    stream.precision(2);
    stream.setf(std::ios::fixed);
    stream.setf(std::ios::right);

    std::list<std::string> RTOs = instrumentCost.getRTOs();
    std::list<std::string>::const_iterator iter;
    for(iter = RTOs.begin(); iter != RTOs.end(); ++iter)
    {
      double cost = instrumentCost.getCost(*iter);
      stream << std::setw(8) << cost;
    }

    stream << "\n";
    return stream.str();
  }

  String ConsoleInitDialog::formatTable(const EstimatedCosts& costs)
  {
    std::stringstream stream;
    std::map<std::string, ResponseTimeCosts>::const_iterator iter = costs.begin();
    if (iter == costs.end())
    {
      return "";
    }

    stream << formatTableHeader((iter->second).getRTOs());
    while (iter != costs.end())
    {
      stream << formatTableLine(iter->first, iter->second);
      iter++;
    }

    return stream.str();
  }

  void ConsoleInitDialog::selectRTO_(unsigned int index)
  {
    std::list<std::string>::const_iterator iter = RTOs_.begin();
    for(unsigned int i = 0; i < index; i++)
    {
      selectedRTO_ = *iter;
      ++iter;
    }
  }

}
