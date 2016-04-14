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

#include <stdexcept>
#include <sstream>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleDialogSelector.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleVersionDialog.h>

#define INPUT *input_
#define OUTPUT *output_

struct Entry
{
    std::string value;
};

std::istream& operator>>(std::istream& is, Entry& entry)
{
  std::istream::sentry s(is);
  if (s) while (is.good())
  {
    char c = is.get();
    if (std::isdigit(c, is.getloc()))
    {
      entry.value += c;
    }
    else if (std::iscntrl(c, is.getloc()) || c < 0) // seems to be terminating char for streams (testing)
    {
      break;
    }
    else
    {
      throw std::invalid_argument("Only digits are allowed.");
    }
  }

  return is;
}

namespace OpenMS
{
  ConsoleVersionDialog::ConsoleVersionDialog(String title, std::list<std::string> versions, String current, String previous, std::istream *input, std::ostream *output)
    : AbstractVersionDialog(title, versions, current, previous)
  {
    input_ = input;
    output_ = output;
  }

  ConsoleVersionDialog::~ConsoleVersionDialog()
  {

  }

  bool ConsoleVersionDialog::exec()
  {
    OUTPUT << "\n\nThe following versions of PeakInvestigator are available:\n\n";

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
        OUTPUT << "\nInvalid entry. Please select from the following versions of PeakInvestigator:\n\n";
        continue;
      }

      if (index == 0) // Cancel
      {
        return false;
      }
      else if (index <= versions_.size())
      {
        selectVersion_(index);
        return true;
      }
      else
      {
        OUTPUT << "\nInvalid number. Please select from the following versions of PeakInvestigator:\n\n";
      }
    }
  }

  void ConsoleVersionDialog::printMenu()
  {
    std::list<std::string>::const_iterator iter;
    unsigned int i;
    for(i = 0, iter = versions_.begin(); iter != versions_.end(); ++i, ++iter)
    {
      String version = *iter;
      if (version == current_ && version == previous_)
      {
        version.append(" (current and last used)");
      }
      else if (version == current_)
      {
        version.append(" (current)");
      }
      else if (version == previous_)
      {
        version.append(" (last used)");
      }

      String line = formatLine(i + 1, version);
      OUTPUT << line;
    }

    OUTPUT << "\nPlease enter your choice (0 to cancel): ";
  }

  String ConsoleVersionDialog::formatLine(int i, String version)
  {
    std::ostringstream stream;
    stream << "\t(" << i << ") " << version << "\n";
    return stream.str();
  }

  void ConsoleVersionDialog::selectVersion_(unsigned int index)
  {
    std::list<std::string>::const_iterator iter = versions_.begin();
    for(unsigned int i = 0; i < index; i++)
    {
      selectedVersion_ = *iter;
      ++iter;
    }
  }
}
