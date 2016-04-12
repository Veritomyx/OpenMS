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

#ifndef OPENMS_TRANSFORMATIONS_RAW2PEAK_PEAKINVESTIGATOR_DIALOGS_CONSOLEINITDIALOG_H
#define OPENMS_TRANSFORMATIONS_RAW2PEAK_PEAKINVESTIGATOR_DIALOGS_CONSOLEINITDIALOG_H

#include <iostream>

#include <PeakInvestigator/Actions/InitAction.h>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigatorConfig.h>

#include "AbstractInitDialog.h"

using Veritomyx::PeakInvestigator::EstimatedCosts;
using Veritomyx::PeakInvestigator::ResponseTimeCosts;

namespace OpenMS
{
  class PEAKINVESTIGATOR_DLLAPI ConsoleInitDialog : public AbstractInitDialog
  {
    public:

      ConsoleInitDialog(String title, EstimatedCosts costs, double funds, std::istream* input = &std::cin, std::ostream* output = &std::cout);
      ~ConsoleInitDialog();

      bool exec();
      void printMenu();

      static String formatTableHeader(const std::list<std::string>& RTOs);
      static String formatTableLine(String instrument, const ResponseTimeCosts& instrumentCost);
      static String formatTable(const EstimatedCosts& costs);
      static String formatLine(int i, String RTO);

    protected:
      void selectRTO_(unsigned int index);

      std::istream* input_;
      std::ostream* output_;
      std::list<std::string> RTOs_;

  };
}

#endif // OPENMS_TRANSFORMATIONS_RAW2PEAK_PEAKINVESTIGATOR_DIALOGS_CONSOLEINITDIALOG_H
