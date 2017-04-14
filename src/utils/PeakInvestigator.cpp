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

//#ifdef WITH_GUI
//#include <QApplication>
//#else
//#include <QtCore/QCoreApplication>
//#endif
//#include <QtCore/QTimer>

using namespace OpenMS;

//-------------------------------------------------------------
//Doxygen docu
//-------------------------------------------------------------

/**
  @page TOPP_PeakInvestigator PeakInvestigator

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

class TOPPPeakInvestigator :
  public TOPPBase
{
public:
  TOPPPeakInvestigator() :
    TOPPBase("PeakInvestigator", "Finds mass spectrometric peaks in profile mass spectra using the PeakInvestigator(TM) software.", false)
  {
  }

protected:

  /**
    @brief Helper class for the Low Memory peak-picking
  */

  void registerOptionsAndFlags_()
  {
    registerInputFile_("in", "<file>", "", "input profile data file ", true);
    setValidFormats_("in", ListUtils::create<String>("mzML"));
    registerOutputFile_("out", "<file>", "", "output peak file ", false);
    setValidFormats_("out", ListUtils::create<String>("mzML"));

    registerStringOption_("mode", "<text>", "", "mode to run PeakInvestigator job");
    setValidStrings_("mode", ListUtils::create<String>("submit,check,fetch,delete"));

    registerSubsection_("peakinvestigator", "PeakInvestigator account information");
  }

  Param getSubsectionDefaults_(const String & /*section*/) const
  {
    return PeakInvestigator().getDefaults();
  }

  ExitCodes main_(int argc, const char ** argv)
  {
    //-------------------------------------------------------------
    // parameter handling
    //-------------------------------------------------------------

    in = getStringOption_("in");
    out = getStringOption_("out");
    mode = getStringOption_("mode");

    Param pi_param = getParam_().copy("peakinvestigator:", true);
    writeDebug_("Parameters passed to PeakInvestigator", pi_param, 3);
//    pi_param.parseCommandLine(argc, argv);

    //----------------------------------------------------------------
    // Open file
    //----------------------------------------------------------------
    MSExperiment experiment;
    MzMLFile input;
    input.load(in, experiment);

    //-------------------------------------------------------------
    // Setup a QCoreApplication for handling network requests
    //-------------------------------------------------------------
//    char **argv2 = const_cast<char**>(argv);
//#ifdef WITH_GUI
//    QApplication app(argc, argv2);
//#else
//    QCoreApplication app(argc, argv2);
//#endif

    PeakInvestigator pp;
    pp.setLogType(log_type_);
    pp.setParameters(pi_param);

    if (!pp.setExperiment(experiment))
    {
      return TOPPBase::INCOMPATIBLE_INPUT_DATA;
    }

    //-----------------------------------------------------------------------------
    // Set the filename with parsed JobID for later saving as needed
    //-----------------------------------------------------------------------------
    String filename;
//    if (mode != "submit")
//    {
//        filename = in.toQString();
//        const String jid("JobID_");
//        const String mzml(".mzML");
//        if(!filename.contains(jid)) {
//            // The filename does not contain a JobID, error!
//            std::cout << std::endl << "Error!:  input filename does not contain a Job ID!" << std::endl;
//            return ILLEGAL_PARAMETERS;
//        } else {
//            int startIndex = filename.indexOf(jid, 1) + jid.count();
//            int endIndex = filename.indexOf(mzml, startIndex);
//            pp.setJobID(filename.mid(startIndex , endIndex - startIndex));
//        }
//    }

    if (mode == "submit")
    {
        pp.setMode(PeakInvestigator::SUBMIT);
    }

    else if (mode == "check")
    {
        pp.setMode(PeakInvestigator::CHECK);
        pp.setJobID(experiment.getMetaValue(OpenMS::PeakInvestigator::META_JOB));
    }

    else if (mode == "fetch")
    {
        pp.setMode(PeakInvestigator::FETCH);
        pp.setJobID(experiment.getMetaValue(OpenMS::PeakInvestigator::META_JOB));
    }

    pp.run();
    filename = out;

// TODO: handle naming files

    // Set the filename and save the data if appropriate
//    if (out == String::EMPTY)
//    {
//      if (mode == "submit")
//      {
//        filename = in.toQString().section(".", 0, -2) + ".JobID_" + pp.getJobID() + ".mzML";
//      }
//      else if (mode == "fetch")
//      {
//        filename = in.toQString().section(".", 0, -3) + ".peaks.mzML";
//      }
//    }
//    else
//    {
//      filename = out.toQString();
//    }
    if (mode != "check" && mode != "delete")
    {
      input.store(filename, pp.getExperiment());
      std::cout << std::endl << "The job was saved in the file " << filename << " for use in status checking and retrieving results." << std::endl;
    }

    return TOPPBase::EXECUTION_OK;

  }

  // parameters
  String in;
  String out;
  String mode;
};


int main(int argc, const char ** argv)
{
  TOPPPeakInvestigator tool;
  return tool.main(argc, argv);
}

/// @endcond
