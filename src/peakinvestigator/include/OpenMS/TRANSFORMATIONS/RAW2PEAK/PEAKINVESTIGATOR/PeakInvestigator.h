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

#ifndef OPENMS_TRANSFORMATIONS_RAW2PEAK_PEAKINVESTIGATOR_H
#define OPENMS_TRANSFORMATIONS_RAW2PEAK_PEAKINVESTIGATOR_H

#include <PeakInvestigator/AbstractProgress.h>
#include <PeakInvestigator/Actions/InitAction.h>
#include <PeakInvestigator/Actions/SftpAction.h>
#include <PeakInvestigator/Actions/RunAction.h>
#include <PeakInvestigator/Actions/StatusAction.h>

#include <OpenMS/FORMAT/MzMLFile.h>
#include <OpenMS/KERNEL/MSExperiment.h>
#include <OpenMS/KERNEL/MSSpectrum.h>

#include <OpenMS/DATASTRUCTURES/DefaultParamHandler.h>
#include <OpenMS/CONCEPT/ProgressLogger.h>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigatorConfig.h>

namespace Veritomyx
{
  namespace PeakInvestigator
  {
    class PeakInvestigatorSaaS;
    struct JobAttributes;
  }
}

using Veritomyx::PeakInvestigator::InitAction;
using Veritomyx::PeakInvestigator::SftpAction;
using Veritomyx::PeakInvestigator::StatusAction;
using Veritomyx::PeakInvestigator::RunAction;

namespace OpenMS
{
  /**
    @brief This class implements the <a href=https://peakinvestigator.veritomyx.com/api>
    PeakInvestigator public API</a> provided by <a href=http://www.veritomyx.com>Veritomyx</a>.

    This class has three modes of operation (submit, check, and fetch) which are
    specified by the PeakInvestigator::setMode() function.

    The default dialog factory is a ConsoleDialogFactory. This can be changed using the
    setDialogFactory() function.

    @ingroup PeakPicking
  */

  class AbstractDialogFactory;

  class PEAKINVESTIGATOR_DLLAPI PeakInvestigator :
      public DefaultParamHandler,
      public ProgressLogger,
      public Veritomyx::PeakInvestigator::AbstractProgress
  {

    public:

      enum Mode
      {
        SUBMIT,
        FETCH
      };

      static const std::string META_JOB;
      static const std::string META_VERSION;

      /// Constructor
      PeakInvestigator(String mode, int debug_level = 0);

      /// Destructor
      virtual ~PeakInvestigator();

      /** @brief Set the desired dialog factory. This allows Console vs. GUI dialogs to be
       * specified at runtime.
       *
       */
      void setDialogFactory(AbstractDialogFactory* factory);

      /// Get the experiment after processing.
      MSExperiment& getExperiment() { return experiment_; }

      /** @brief Set the experiment for processing
     *
     * This function also makes sure the experiment being set has the correct data (i.e.
     * contains mass spectra and isn't already centroided).
     * @return Bool indicating whether the experiment has the correct attributes.
     */
      bool setExperiment(MSExperiment& experiment);

      /** @brief Set the characterization file for processing
     *
     * This function also makes sure the file being set has the correct data (i.e.
     * contains mass spectra and isn't already centroided).
     * @return Bool indicating whether the experiment has the correct attributes.
     */
      bool setCharacterization(MSExperiment& characterization);

      /// Get the jobID, which is set after a call to initializeJob_().
      String getJobID() { return job_; }

      /// Set the jobID.
      void setJobID(const String jobID) { job_ = jobID; }

      /// Set the path for the log file when fetching job
      void setLogPath(const String log_path) { log_path_ = log_path; }

      /// Set the username
      void setUsername(const String& username) { username_ = username; }

      /// Set the project
      void setProject(const String& projectID) { projectID_ = projectID.toInt(); }

      void run();

      /* Methods for displaying SFTP progress */
      void initialize(const int total, const std::string label);
      void setProgress(const int progress);
      void finish();

      /// Utility function to determine job attributes in an experiment
      static Veritomyx::PeakInvestigator::JobAttributes getJobAttributes(MSExperiment& experiment);

      /// Utility function to create correct DataProcessing from an experiment
      static DataProcessing* getDataProcessing(MSExperiment& experiment);

    protected:

      /* Main modes of running. */
      void submit_();
      StatusAction check_();
      void fetch_();

      /* Methods used for submitting a job. */
      String getVersion_();
      InitAction initializeJob_(String version);
      String getRTO_(InitAction &action);
      void uploadScans_(SftpAction &sftp_action, const MSExperiment& experiment, const String filename);
      RunAction runJob_(String job, String RTO, String filename, String calib_filename);

      /* Functions for saving/loading data from tarballs. */
      void saveScans_(const MSExperiment &experiment, String filename);
      void loadScans_(String filename);

      /* Misc methods. */
      String getPassword_();
      SftpAction getSftpInfo_();
      void deleteJob_();
      bool validateExperiment_(MSExperiment& experiment);

      ///@}
      //--------------------------------------------------------------------------------------------------------


      // Veritomyx account info
      String server_; ///< @brief Veritomyx server address. Should be provided using the TOPP interface.
      String username_; ///< @brief Veritomyx account username. Should be provided using the TOPP interface.
      String password_; ///< @brief Veritomyx account password. Should be provided using the TOPP interface.
      int projectID_; ///< @brief Veritomyx project ID. Should be provided using the TOPP interface.
      String job_; ///< @brief Job number obtained from public API during INIT request.

      // Other settings
      String PIVersion_; ///< @brief PI version selected by the user.
      String RTO_; ///< @brief Response time objective selected by the user.
      int min_mass_; ///< @brief Minimum mass selected by the user.
      int max_mass_; ///< @brief Maximum mass selected by the user.

      // docu in base class
      void updateMembers_();

      // Misc variables
      Veritomyx::PeakInvestigator::PeakInvestigatorSaaS* service_;
      MSExperiment experiment_; ///< @brief Class used to hold spectra (raw or peak data) in memory.
      MSExperiment characterization_; ///< @brief Class used to hold spectra (raw or peak data) in memory.
      Mode mode_;
      String log_path_;
      bool debug_;

      // Dialog factory
      AbstractDialogFactory* dialog_factory_;

  }; // end PeakInvestigator

} // namespace OpenMS

#endif
