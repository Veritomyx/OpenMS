// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2013.
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
// $Authors: Adam Tenderholt $
// --------------------------------------------------------------------------

#include <OpenMS/APPLICATIONS/TOPPBase.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleDialogFactory.h>

#ifdef WITH_GUI
#include <QApplication>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/GUIDialogFactory.h>
#endif

using namespace OpenMS;

//-------------------------------------------------------------
//Doxygen docu
//-------------------------------------------------------------

/**
  @page TOPP_PeakInvestigator PeakInvestigatorSubmitJob

  @brief A tool for peak detection in profile data. Executes the peak picking using the @ref
  OpenMS::PeakInvestigator algorithm (see www.veritomyx.com for more details).
 
  The conversion of the "raw" ion count data acquired by the machine into peak lists for further processing
  is usually called peak picking or centroiding. Unlike most centroiding algorithms, there is no need to perform
  any additional signal processing algorithms such as data smoothing or baseline removal prior to centroiding with
  PeakInvestigator.

  <B>The command line parameters of this tool are:</B>
  @verbinclude TOPP_PeakInvestigator.cli
  <B>INI file documentation of this tool:</B>
  @htmlinclude TOPP_PeakInvestigator.html

*/

// We do not want this class to show up in the docu:
/// @cond TOPPCLASSES

class TOPPPeakInvestigatorSubmitJob :
  public TOPPBase
{
public:
  TOPPPeakInvestigatorSubmitJob() :
    TOPPBase("PeakInvestigatorSubmitJob",
                "Submit a job to the PeakInvestigator(R) centroiding and deconvolution software service.",
                false)
  {
  }

  ~TOPPPeakInvestigatorSubmitJob()
  {
  }

protected:

  void registerOptionsAndFlags_()
  {
    registerInputFile_("in", "<file>", "", "input profile data file ", true);
    setValidFormats_("in", ListUtils::create<String>("mzML"));

    registerInputFile_("ch", "<file>", "", "input characterization file", false);
    setValidFormats_("ch", ListUtils::create<String>("mzML"));

    registerOutputFile_("out", "<file>", "", "output meta data file ", false);
    setValidFormats_("out", ListUtils::create<String>("mzML"));

    registerStringOption_("username", "<text>", "", "username for PeakInvestigator services", true);
    registerStringOption_("project", "<text>", "", "project for PeakInvestigator services", true);

#if WITH_GUI
    registerFlag_("console", "Use the console for dialogs");
#endif

    registerSubsection_("peakinvestigator", "PeakInvestigator account information and options");
  }

  Param getSubsectionDefaults_(const String & /*section*/) const
  {
    return PeakInvestigator("submit", 0, new ConsoleDialogFactory(0, NULL)).getDefaults();
  }

  ExitCodes main_(int argc, const char ** argv)
  {
    //-------------------------------------------------------------
    // parameter handling
    //-------------------------------------------------------------

    in = getStringOption_("in");
    ch = getStringOption_("ch");
    out = getStringOption_("out");
    username = getStringOption_("username");
    project = getStringOption_("project");

    Param pi_param = getParam_().copy("peakinvestigator:", true);
    writeDebug_("Parameters passed to PeakInvestigator", pi_param, 3);

    //----------------------------------------------------------------
    // Open file
    //----------------------------------------------------------------
    MSExperiment experiment;
    MzMLFile input;
    input.setLogType(log_type_);
    input.load(in, experiment);

    MSExperiment characterization;
    if (ch != String::EMPTY)
    {
      input.load(ch, characterization);
    }

#if WITH_GUI
    AbstractDialogFactory* factory = getFlag_("console") ? static_cast<AbstractDialogFactory*>(new ConsoleDialogFactory(argc, argv))
                                                         : static_cast<AbstractDialogFactory*>(new GUIDialogFactory(argc, argv));
    PeakInvestigator pp("submit", debug_level_, factory);
#else
    PeakInvestigator pp("submit", debug_level_, new ConsoleDialogFactory(argc, argv));
#endif

    pp.setLogType(log_type_);
    pp.setParameters(pi_param);
    pp.setUsername(username);
    pp.setProject(project);

    if (!pp.setExperiment(experiment))
    {
      return TOPPBase::INCOMPATIBLE_INPUT_DATA;
    }

    if (ch != String::EMPTY && !pp.setCharacterization(characterization))
    {
      return TOPPBase::INCOMPATIBLE_INPUT_DATA;
    }

    try
    {
      pp.run();
    } catch (std::exception& e) {
      LOG_ERROR << e.what() << std::endl;
      return TOPPBase::INTERNAL_ERROR;
    }

    if (out.empty())
    {
      std::vector<String> infos;

      in.split('.', infos);
      out.concatenate(infos.begin(), infos.end() - 1, '.');
      out += ".job-" + pp.getJobID() + ".mzML";

    }

    input.store(out, pp.getExperiment());

    return TOPPBase::EXECUTION_OK;

  }

  // parameters
  String in;
  String ch;
  String out;
  String username;
  String project;

};


int main(int argc, const char ** argv)
{
  TOPPPeakInvestigatorSubmitJob tool;
  return tool.main(argc, argv);
}

/// @endcond
