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
// $Author: Adam Tenderholt, David Rivkin $
// --------------------------------------------------------------------------
//

#include <cmath>
#include <regex>

#include <PeakInvestigator/PeakInvestigatorSaaS.h>
#include <PeakInvestigator/TarFile.h>
#include <PeakInvestigator/Actions/DeleteAction.h>
#include <PeakInvestigator/Actions/PiVersionsAction.h>
#include <PeakInvestigator/Actions/RunAction.h>
#include <PeakInvestigator/Actions/SandboxAction.h>

#include <OpenMS/FORMAT/PeakTypeEstimator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsoleDialogFactory.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/AbstractVersionDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/AbstractInitDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/AbstractPasswordDialog.h>

using namespace Veritomyx::PeakInvestigator;

#define SCANS_EXT ".scans.tar"
#define CALIB_EXT ".calib.tar"

#define KEY_PI_SERVER "server"
#define KEY_PI_VERSION "Version"


#define SANDBOX 1

namespace OpenMS
{
  const std::string PeakInvestigator::META_JOB("peakinvestigator:job");
  const std::string PeakInvestigator::META_VERSION("peakinvestigator:version");

  PeakInvestigator::PeakInvestigator(String mode, int debug_level) :
    DefaultParamHandler("PeakInvestigator"),
    ProgressLogger(),
    debug_(debug_level > 0)
  {

#ifdef SANDBOX
    LOG_INFO << "*** Using API sandbox. ****" << std::endl;
    defaults_.setValue("sandbox:init", 0, "Sandbox value for INIT call");
    defaults_.setValue("sandbox:run", 0, "Sandbox value for RUN call");
    defaults_.setValue("sandbox:status", "Done", "Sandbox value for STATUS call");
    defaults_.setValue("sandbox:delete", 0, "Sandbox value for DELETE call");
#endif

    // set default parameter values
    defaults_.setValue(KEY_PI_SERVER, "peakinvestigator.veritomyx.com", "Server address for PeakInvestigator (without https://)");

    if (mode == "submit")
    {
      mode_ = PeakInvestigator::SUBMIT;
      defaults_.setValue(KEY_PI_VERSION, "select", "Version of PeakInvestigator to use");
    } else if (mode == "fetch")
    {
      mode_ = PeakInvestigator::FETCH;
    } else
    {
      throw Exception::IllegalArgument(__FILE__, __LINE__, "PeakInvestigator::PeakInvestigator",
                                       "Invalid mode: " + mode);
    }

    // write defaults into Param object param_
    defaultsToParam_();
    updateMembers_();

    dialog_factory_ = new ConsoleDialogFactory();

  }

  PeakInvestigator::~PeakInvestigator()
  {
    delete dialog_factory_;
  }

  void PeakInvestigator::setDialogFactory(AbstractDialogFactory *factory)
  {
    delete dialog_factory_;
    dialog_factory_ = factory;
  }

  void PeakInvestigator::initialize(const int total, const std::string label)
  {
    startProgress(0, total, label);
  }

  void PeakInvestigator::setProgress(const int progress)
  {
    ProgressLogger::setProgress(progress);
  }

  void PeakInvestigator::finish()
  {
    endProgress();
  }

  bool PeakInvestigator::setExperiment(MSExperiment& experiment)
  {
    bool retval = validateExperiment_(experiment);
    if (retval)
    {
      experiment_ = experiment;
    }

    return retval;
  }

  bool PeakInvestigator::setCharacterization(MSExperiment& characterization)
  {
    bool retval = validateExperiment_(characterization);
    if (retval)
    {
      characterization_ = characterization;
    }

    return retval;
  }

  void PeakInvestigator::run()
  {

    service_ = new PeakInvestigatorSaaS(server_);
    service_->setDebug(debug_);

    password_ = getPassword_();
    if (password_ == "")
    {
      return;
    }

    // filenames for the tar'd scans/results
    String zipfilename;
    String localFilename;
    String remoteFilename;

    switch(mode_)
    {
    case SUBMIT:
      submit_();
      break;
    case FETCH:
      fetch_();
      break;
    }

    delete service_;
  }

  void PeakInvestigator::submit_()
  {
    String version = getVersion_();
    if (version.size() == 0)
    {
      return;
    }

    InitAction init_action = initializeJob_(version);
    String RTO = getRTO_(init_action);
    LOG_DEBUG << "Using RTO: " << RTO << std::endl;

    SftpAction sftp_action = getSftpInfo_();

    String scans_filename = init_action.getJob() + SCANS_EXT;
    uploadScans_(sftp_action, experiment_, scans_filename);

    String calib_filename = "";
    if (characterization_.size() > 0)
    {
      calib_filename = init_action.getJob() + CALIB_EXT;
      uploadScans_(sftp_action, characterization_, calib_filename);
    }

    runJob_(init_action.getJob(), RTO, scans_filename, calib_filename);

  }

  StatusAction PeakInvestigator::check_()
  {

    StatusAction action(username_, password_, job_);

#ifdef SANDBOX
    SandboxAction* sandbox = new SandboxAction(&action, param_.getValue("sandbox:status").toString());
    String response = service_->executeAction(sandbox);
    delete sandbox;
#else
    String response = service_->executeAction(&action);
#endif

    action.processResponse(response);

    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::check_()", action.getErrorMessage());
    }

    return action;
  }

  void PeakInvestigator::fetch_()
  {
    StatusAction status_action = check_();
    switch(status_action.getStatus())
    {
    case StatusAction::PREPARING:
    case StatusAction::RUNNING:
      LOG_INFO << "The job (" << job_ << ") is still running." << std::endl;
      return;
    case StatusAction::DELETED:
      LOG_INFO << "The job (" << job_ << ") has been deleted." << std::endl;
      return;
    case StatusAction::DONE:
      LOG_INFO << "The job (" << job_ << ") has finished. Downloading results." << std::endl;
      break;
    }

    SftpAction action = getSftpInfo_();
    LOG_DEBUG << "Results – " << status_action.getResultsFilename() << std::endl;
    LOG_DEBUG << "Log – " << status_action.getLogFilename() << std::endl;

    String mass_lists = File::getTempDirectory() + "/" + job_ + ".mass_list.tar";

    LOG_DEBUG << "Downloading mass lists to: " << mass_lists << std::endl;
    LOG_DEBUG << "Downloading log to: " << log_path_ << std::endl;

    service_->downloadFile(action, status_action.getResultsFilename(), mass_lists, this);
    service_->downloadFile(action, status_action.getLogFilename(), log_path_, this);

    loadScans_(mass_lists);

    experiment_.removeMetaValue(META_JOB);
    experiment_.removeMetaValue(META_VERSION);

    deleteJob_();
  }

  String PeakInvestigator::getVersion_()
  {
    PiVersionsAction action(username_, password_);
    String response = service_->executeAction(&action);
    action.processResponse(response);

    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::getVersion_()", action.getErrorMessage());
    }

    if(PIVersion_.toLower() == "last")
    {
      return action.getLastUsedVersion();
    }
    else if (PIVersion_.toLower() == "current")
    {
      return action.getCurrentVersion();
    }
    else if (PIVersion_.toLower() == "select")
    {
      AbstractVersionDialog* dialog = dialog_factory_->getVersionDialog("Please select a version...",
                                                                        action.getVersionsList(), action.getCurrentVersion(),
                                                                        action.getLastUsedVersion());
      bool retval = dialog->exec();
      String version = retval ? dialog->getSelectedVersion() : "";

      delete dialog;
      return version;
    }

    throw Exception::InvalidParameter(__FILE__, __LINE__, "PeakInvestigator::getVersion_()", "Invalid version specification.");

  }

  InitAction PeakInvestigator::initializeJob_(String version)
  {
    JobAttributes attributes = PeakInvestigator::getJobAttributes(experiment_);
    InitAction action = InitAction(username_, password_, projectID_, version, experiment_.size(), attributes, characterization_.size());

    LOG_DEBUG << "action.buildQuery(): " << action.buildQuery() << std::endl;

#ifdef SANDBOX
    SandboxAction* sandbox = new SandboxAction(&action, param_.getValue("sandbox:init").toString());
    String response = service_->executeAction(sandbox);
    delete sandbox;
#else
    String response = service_->executeAction(&action);
#endif
    action.processResponse(response);


    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::submit_()", action.getErrorMessage());
    }

    experiment_.setMetaValue(META_VERSION, version);
    return action;
  }

  String PeakInvestigator::getRTO_(InitAction& action)
  {
    EstimatedCosts costs = action.getEstimatedCosts();
    double funds = action.getFunds();

    AbstractInitDialog* dialog = dialog_factory_->getInitDialog("Please select a Response Time Objective...", costs, funds);
    bool retval = dialog->exec();
    if(!retval)
    {
      return "";
    }

    String RTO = dialog->getSelectedRTO();
    delete dialog;
    return RTO;
  }

  void PeakInvestigator::uploadScans_(SftpAction& sftp_action, const MSExperiment& experiment, const String filename)
  {
    String local_name = File::getTempDirectory() + "/" + filename;
    LOG_DEBUG << "Using temporary file " << local_name << std::endl;

    saveScans_(experiment, local_name);

    String remote_name = sftp_action.getDirectory() + "/" + filename;
    service_->uploadFile(sftp_action, local_name, remote_name, this);
  }

  RunAction PeakInvestigator::runJob_(String job, String RTO, String filename, String calib_filename)
  {
    RunAction action(username_, password_, job, RTO, filename, calib_filename);

#ifdef SANDBOX
    SandboxAction* sandbox = new SandboxAction(&action, param_.getValue("sandbox:run").toString());
    String response = service_->executeAction(sandbox);
    delete sandbox;
#else
    String response = service_->executeAction(&action);
#endif

    action.processResponse(response);

    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::runJob_()", action.getErrorMessage());
    }

    // remove profile data
    for (Size i = 0; i < experiment_.size(); i++)
    {
      experiment_[i].clear(false);
    }

    experiment_.setMetaValue(META_JOB, job);
    job_ = job;

    return action;
  }

  void PeakInvestigator::saveScans_(const MSExperiment& experiment, String filename)
  {
    startProgress(0, experiment.size(), "Exporting scan data...");

    TarFile file(filename, SAVE);
    file.setDebug(debug_);

    for(Size i = 0; i < experiment.size(); i++)
    {
      std::stringstream entryname, data;
      entryname << "scan" << std::setfill('0') << std::setw(5) << i << ".txt";

      MSSpectrum<Peak1D> spectrum = experiment[i];
      data << std::setprecision(std::numeric_limits<double>::digits10 + 1);
      for(Size j = 0; j < spectrum.size(); j++)
      {
        data << spectrum[j].getMZ() << "\t" << spectrum[j].getIntensity() << "\n";
      }

      file.writeFile(entryname.str(), data);

      ProgressLogger::setProgress(i);
    }

    file.close();

    endProgress();
  }

  void PeakInvestigator::loadScans_(String filename)
  {
    int progress = 0;
    startProgress(progress, experiment_.size(), "Loading mass lists...");

    boost::shared_ptr<DataProcessing> data_processing(getDataProcessing(experiment_));

    TarFile file(filename, LOAD);
    std::stringstream contents;

    std::string entry = file.readNextFile(contents);
    while(!entry.empty())
    {
      if (entry == "./")
      {
        contents.clear();
        entry = file.readNextFile(contents);
        continue;
      }

      LOG_DEBUG << "Processing " << entry << std::endl;

      std::regex regex(".*scan(\\d+)\\.mass_list\\.txt");
      std::smatch match;

      if(!std::regex_search(entry, match, regex))
      {
        throw std::runtime_error("Unable to parse entry: " + entry);
      }

      int scan_num = std::stoi(match.str(1));

      LOG_DEBUG << "... for scan " << scan_num << " , content peak: " << static_cast<char>(contents.peek()) << std::endl;

      MSSpectrum<Peak1D> spectrum = experiment_[scan_num];
      std::string line;
      while(std::getline(contents, line))
      {
        if(line.empty() || line.at(0) == '#')
        {
          LOG_DEBUG << "..... Skipping '" << line << "'." << std::endl;
          continue;
        }

        Peak1D::CoordinateType mz, mz_error, minimum_error;
        Peak1D::IntensityType intensity, intensity_error;
        int degrees_of_freedom;

        int retval = sscanf(line.c_str(), "%lf\t%f\t%lf\t%f\t%lf\t%d", &mz, &intensity,
                            &mz_error, &intensity_error, &minimum_error, &degrees_of_freedom);
        if (retval != 6)
        {
          LOG_ERROR << "Problem parsing " << entry << ": " << line << std::endl;
        }

        Peak1D peak(mz, intensity);
        spectrum.push_back(peak);
      }

      LOG_DEBUG << ".....found " << spectrum.size() << " peaks." << std::endl;

      spectrum.updateRanges();
      spectrum.setType(SpectrumSettings::PEAKS);
      spectrum.getDataProcessing().push_back(data_processing);
      experiment_[scan_num] = spectrum;

      ProgressLogger::setProgress(progress);
      progress++;

      contents.clear();
      entry = file.readNextFile(contents);
    }

    endProgress();
  }

  String PeakInvestigator::getPassword_()
  {
    String password;

    while(true)
    {
      AbstractPasswordDialog* password_dialog = dialog_factory_->getPasswordDialog();
      if(!password_dialog->exec())
      {
        return "";
      }

      password = password_dialog->getPassword();
      delete password_dialog;

      PiVersionsAction action(username_, password);
      std::string response = service_->executeAction(&action);
      action.processResponse(response);

      if(!action.hasError())
      {
        break;
      } else if (action.getErrorCode() != 3)
      {
        throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::getPassword_()",
                                       action.getErrorMessage());
      }

      std::cout << "Incorrect password. Please try again." << std::endl;
    }

    return password;
  }

  SftpAction PeakInvestigator::getSftpInfo_()
  {
    SftpAction action(username_, password_, projectID_);
    String response = service_->executeAction(&action);
    action.processResponse(response);

    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::getSftpInfo_()", action.getErrorMessage());
    }

    return action;
  }

  void PeakInvestigator::deleteJob_()
  {
    LOG_DEBUG << "Delete job " << job_ << ".\n";

    DeleteAction action(username_, password_, job_);

#ifdef SANDBOX
    SandboxAction* sandbox = new SandboxAction(&action, param_.getValue("sandbox:delete").toString());
    String response = service_->executeAction(sandbox);
    delete sandbox;
#else
    String response = service_->executeAction(&action);
#endif

    action.processResponse(response);
    if(action.hasError())
    {
      throw Exception::FailedAPICall(__FILE__, __LINE__, "PeakInvestigator::getdeleteJob_()", action.getErrorMessage());
    }

  }

  bool PeakInvestigator::validateExperiment_(OpenMS::MSExperiment &experiment)
  {
    if (experiment.empty())
    {
      LOG_ERROR << "The given file appears to not contain any m/z-intensity data points.";
      return false;
    }

    //check for peak type (profile data required)
    if (PeakTypeEstimator().estimateType(experiment[0].begin(), experiment[0].end()) == SpectrumSettings::PEAKS)
    {
      LOG_ERROR << "OpenMS peak type estimation indicates that this is not profile data!";
      return false;
    }

    return true;
  }

  JobAttributes PeakInvestigator::getJobAttributes(OpenMS::MSExperiment &experiment)
  {
    int minMass = INT_MAX, maxMass = 0, maxPoints = 0;
    for(Size s = 0; s < experiment.size(); s++)
    {
      experiment[s].sortByPosition();
      experiment[s].updateRanges();
      minMass = std::min(minMass, static_cast<int>(std::floor(experiment[s].getMin()[0])));
      maxMass = std::max(maxMass, static_cast<int>(std::ceil(experiment[s].getMax()[0])));
      maxPoints = std::max(maxPoints, static_cast<int>(experiment[s].size()));
    }

    JobAttributes attributes;
    attributes.min_mass = minMass;
    attributes.max_mass = maxMass;
//TODO: handle Start and End better later
    attributes.start_mass = minMass;
    attributes.end_mass = maxMass;
    attributes.max_points = maxPoints;

    return attributes;
  }

  DataProcessing* PeakInvestigator::getDataProcessing(MSExperiment& experiment)
  {
    DataProcessing* data_processing = new DataProcessing();

    std::set<DataProcessing::ProcessingAction> actions;
    actions.insert(DataProcessing::PEAK_PICKING);
    data_processing->setProcessingActions(actions);

    data_processing->getSoftware().setName("PeakInvestigator");
    data_processing->getSoftware().setVersion(experiment.getMetaValue(META_VERSION));
    data_processing->setCompletionTime(DateTime::now());

    return data_processing;
  }

  void PeakInvestigator::updateMembers_()
  {
    server_ = param_.getValue(KEY_PI_SERVER);

    if (mode_ == PeakInvestigator::SUBMIT)
    {
      PIVersion_ = param_.getValue(KEY_PI_VERSION);
    }

    DefaultParamHandler::updateMembers_();
  }

}
