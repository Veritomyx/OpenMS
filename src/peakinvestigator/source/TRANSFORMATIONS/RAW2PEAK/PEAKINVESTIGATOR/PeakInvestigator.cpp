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

//#ifdef WITH_GUI
//#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/RtoDialog.h>
//#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/VersionDialog.h>
//#endif

using namespace Veritomyx::PeakInvestigator;

#define SCANS_EXT ".scans.tar"
#define CALIB_EXT ".calib.tar"

#define KEY_PI_SERVER "server"
#define KEY_PI_USERNAME "username"
#define KEY_PI_PASSWORD "password"
#define KEY_PI_PROJECT "project"

#define KEY_PI_MZ "m/z"
#define KEY_PI_RTO "RTO"
#define KEY_PI_VERSION "Version"


#define SANDBOX 1

namespace OpenMS
{
  const std::string PeakInvestigator::META_JOB("peakinvestigator:job");
  const std::string PeakInvestigator::META_VERSION("peakinvestigator:version");

  PeakInvestigator::PeakInvestigator(int debug_level) :
    DefaultParamHandler("PeakInvestigator"),
    ProgressLogger()
  {
    // set default parameter values
    defaults_.setValue(KEY_PI_SERVER, "peakinvestigator.veritomyx.com", "Server address for PeakInvestigator (without https://)");
    defaults_.setValue(KEY_PI_USERNAME, "USERNAME", "Username for account registered with Veritomyx");
    defaults_.setValue(KEY_PI_PASSWORD, "PASSWORD", "Password for account registered with Veritomyx");
    defaults_.setValue(KEY_PI_PROJECT, 12345, "The project number used for the PeakInvestigator job");

    defaults_.setValue(KEY_PI_MZ, "[min]:[max]", "m/z range to extract (applies to ALL ms levels!");

    defaults_.setValue(KEY_PI_RTO, "RTO-24", "Response Time Objective to use");
    defaults_.setValue(KEY_PI_VERSION, "select", "Version of PeakInvestigator to use");

    // write defaults into Param object param_
    defaultsToParam_();
    updateMembers_();

    service_ = new PeakInvestigatorSaaS(server_);
    dialog_factory_ = new ConsoleDialogFactory();

    if (debug_level > 0)
    {
      service_->setDebug();
    }

#ifdef SANDBOX
    LOG_INFO << "*** Using API sandbox. ****" << std::endl;
#endif

  }

  PeakInvestigator::~PeakInvestigator()
  {
    delete service_;
    delete dialog_factory_;
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

    // filenames for the tar'd scans/results
    String zipfilename;
    String localFilename;
    String remoteFilename;

    switch(mode_)
    {
    case SUBMIT:
      submit_();
      break;
    case CHECK:
      check_();
      break;
    case FETCH:
      fetch_();
      break;
    }

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
    SandboxAction* sandbox = new SandboxAction(&action, "Done");
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

    String mass_lists = job_ + ".mass_list.tar";

    service_->downloadFile(action, status_action.getResultsFilename(), mass_lists, this);
    service_->downloadFile(action, status_action.getLogFilename(), job_ + ".log.txt", this);

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
    SandboxAction* sandbox = new SandboxAction(&action, 0);
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
    // TODO:    String filename = File::getTempDirectory() + "/" + initAction.getJob() + ".tar";
    String local_name = filename;
    saveScans_(experiment, filename);

    String remote_name = sftp_action.getDirectory() + "/" + filename;
    service_->uploadFile(sftp_action, local_name, remote_name, this);
  }

  RunAction PeakInvestigator::runJob_(String job, String RTO, String filename, String calib_filename)
  {
    RunAction action(username_, password_, job, RTO, filename, calib_filename);

#ifdef SANDBOX
    SandboxAction* sandbox = new SandboxAction(&action, 0);
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
    for(Size i = 0; i < experiment.size(); i++)
    {
      std::stringstream entryname, data;
      entryname << "scan" << std::setfill('0') << std::setw(5) << i << ".txt";

      MSSpectrum<Peak1D> spectrum = experiment[i];
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

      LOG_DEBUG << "... for scan " << scan_num << std::endl;

      MSSpectrum<Peak1D> spectrum = experiment_[scan_num];
      std::string line;
      while(std::getline(contents, line))
      {
        if(line.empty() || line.at(0) == '#')
        {
          continue;
        }

        Peak1D::CoordinateType mz, mz_error, minimum_error;
        Peak1D::IntensityType intensity, intensity_error;
        int degrees_of_freedom;

        sscanf(line.c_str(), "%lf\t%f\t%lf\t%f\t%lf\t%d", &mz, &intensity, &mz_error, &intensity_error,
               &minimum_error, &degrees_of_freedom);

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

      entry = file.readNextFile(contents);
    }

    endProgress();
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
    SandboxAction* sandbox = new SandboxAction(&action, 0);
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

  /*
  bool PeakInvestigator::PIVersionsJob_()
  {
    LOG_DEBUG << "Requsting credentials for " + username_ + "..." << endl;

    PiVersionsAction action(username_.toQString(), password_.toQString());
    QString params = action.buildQuery();

    bool ok, ret;
    QString contents = Post_(params, ok);
    action.processResponse(contents);

    ret = (ok && !action.hasError());
    if(ret)
    {
      CurrentVersion_ = action.getCurrentVersion();
      LastUsedVersion_ = action.getLastUsedVersion();
      PI_versions_ = action.getVersions();
      PIVersion_ = (LastUsedVersion_.isEmpty() ? CurrentVersion_ : LastUsedVersion_);
    }
    min_mass_ = 0;
    max_mass_ = INT_MAX;
    int minMass = INT_MAX,
        maxMass = 0;
    points_count_ = 0;
    // Loop over the scans
    for(Size s = 0; s < experiment_.size(); s++)
    {
      // Loop over the datapoints
      experiment_[s].sortByPosition();
      // Get the Position_/Coordinate_
      minMass = qMin(minMass, static_cast<int>(floor(experiment_[s][0].getPosition()[0])));
      Size sEnd = experiment_[s].size()-1;
      maxMass = qMax(maxMass, static_cast<int>(ceil(experiment_[s][sEnd].getPosition()[0])));
      // Get the number of datapoints in the scan
      points_count_ = qMax(points_count_, static_cast<int>(sEnd));
    }
    min_mass_ = qMax(minMass, min_mass_);
    max_mass_ = qMin(maxMass, max_mass_);

    return ret;
  }

  bool PeakInvestigator::initializeJob_()
  {
    LOG_DEBUG << "Requsting credentials for " + username_ + "..." << endl;

    // Ensure the selected PIVersion_ is available
    if(!PI_versions_.contains(PIVersion_))
    {
      LOG_ERROR << "There was an error with the seleted version: " << PIVersion_.constData() << endl;
      return false;
    }

    InitAction action(username_.toQString(), password_.toQString(),
                      account_number_, PIVersion_,
                      static_cast<int>(experiment_.size()), points_count_+1, min_mass_, max_mass_);
    // TODO:  Add calibrationCount to the InitAction

    bool ok, ret;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    ret = (ok && !action.hasError());
    if(ret)
    {
      job_ = action.getJob();
      funds_ = action.getFunds();
      projectId_ = action.getProjectId();
      estimatedCosts_ = action.getEstimatedCosts();
    }
    return ret;
  }

  bool PeakInvestigator::submitJob_()
  {
    RunAction action(username_.toQString(), password_.toQString(),
                     job_, RTO_, sftp_file_);
    // TODO:  CalibrationFile should be added calib_file_;

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok && !action.hasError())
    {

      cout << endl << "Job " << action.getJob().toAscii().constData() << " started." << endl;
      //    LOG_INFO << contents.toAscii().constData() << endl;

      experiment_.setMetaValue("peakinvestigator:job", job_);
      return ok;
    }
    else
    {
      cout << action.getErrorMessage().toAscii().constData() << endl;
      return false;
    }

  }

  bool PeakInvestigator::checkJob_()
  {
    bool retval = false;

    if(job_.isEmpty())
    {
      job_ = experiment_.getMetaValue("peakinvestigator:job").toQString();

      if (job_.isEmpty())
      {
        LOG_WARN << "Problem getting job ID from meta data.\n";
        return retval;
      }
    }

    StatusAction action(username_.toQString(), password_.toQString(),
                        job_);

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok && !action.hasError())
    {
      switch(action.getStatus())
      {
      case StatusAction::PREPARING :
        LOG_INFO << job_.toAscii().constData() << " is still preparing.\n";
        date_updated_ = action.getDateTime();
        retval = false;
        break;
      case StatusAction::RUNNING :
        LOG_INFO << job_.toAscii().constData() << " is still runing.\n";
        date_updated_ = action.getDateTime();
        retval = false;
        break;
      case StatusAction::DELETED :
        LOG_INFO << job_.toAscii().constData() << " was deleted.\n";
        date_updated_ = action.getDateTime();
        retval = false;
        break;
      case StatusAction::DONE :
        LOG_INFO << job_.toAscii().constData() << " has finished.\n";
        results_file_ = action.getResultsFilename();
        log_file_ = action.getLogFilename();
        actual_cost_ = action.getActualCost();
        date_updated_ = action.getDateTime();
        //            if( action.getNumberOfInputScans() != action.getNumberOfCompleteScans()) {
        //                LOG_ERROR << "The number of input scans does not match the number of scans completed!";
        //                retval = false;
        //            } else {
        retval = true;
        //           }
      }
      return retval;
    }
    else
    {
      cout << action.getErrorMessage().toAscii().constData() << endl;
      return false;
    }
  }

  bool PeakInvestigator::removeJob_()
  {

    DeleteAction action(username_.toQString(), password_.toQString(),
                        job_);

    bool ok;
    QString toSaas = action.buildQuery();
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);
    //    LOG_INFO << contents.toAscii().constData() << endl;
    return ok;

  }

  bool PeakInvestigator::getSFTPCredentials_()
  {
    SftpAction action(username_.toQString(), password_.toQString(),
                      account_number_) ;

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok)
    {
      sftp_host_ = action.getHost();
      sftp_port_ = action.getPort();
      sftp_dir_  = action.getDirectory();
      sftp_username_ = action.getSftpUsername();
      sftp_password_ = action.getSftpPassword();
    }
    //      LOG_INFO << contents.toAscii().constData() << endl;

    return ok;
  }

  PeakInvestigator::PIStatus PeakInvestigator::getPrepFileMessage_(QString filename)
  {
    PrepAction action(username_.toQString(), password_.toQString(),
                      account_number_,
                      filename);

    bool ok;
    QString contents = Post_(action.buildQuery(), ok);
    action.processResponse(contents);

    if(ok && !action.hasError())
    {
      switch(action.getStatus())
      {
      case PrepAction::READY :
        LOG_INFO << "Preparation of the job file completed, ";
        prep_count_ = action.getScanCount();
        LOG_INFO << "found " << prep_count_ << " scans, and mass spectrometer type ";
        prep_ms_type_ = action.getMStype();
        LOG_INFO <<  prep_ms_type_.toAscii().constData() << endl;
        //             cout << contents.toAscii().constData() << endl;
        return PREP_READY;
      case PrepAction::ANALYZING :
        prep_percent_complete_ = action.getPercentComplete().left(action.getPercentComplete().indexOf("%")).toDouble();
        LOG_INFO << "Preparation of the job file is still analyzing.  % Complete: " << action.getPercentComplete().toAscii().constData() << endl;
        return PREP_ANALYZING;
      case PrepAction::ERROR :
        LOG_INFO << "Preparation of the job file returned an error occurred" << endl;
        return PREP_ERROR;
      }
    }
    else
    {
      cout << endl << action.getErrorMessage().toAscii().constData() << endl;
      return PREP_ERROR;
    }
  }
*/

  void PeakInvestigator::updateMembers_()
  {
    server_ = param_.getValue(KEY_PI_SERVER);
    username_ = param_.getValue(KEY_PI_USERNAME);
    password_ = param_.getValue(KEY_PI_PASSWORD);
    projectID_ = param_.getValue(KEY_PI_PROJECT);

    String  minMaxString = param_.getValue(KEY_PI_MZ);
    std::vector<String> minMaxSplit;
    if((minMaxString != "[min]:[max]") && minMaxString.split(':', minMaxSplit))
    {
      min_mass_ = minMaxSplit[0].toInt();
      max_mass_ = minMaxSplit[1].toInt();
    }

    RTO_ = param_.getValue(KEY_PI_RTO);
    PIVersion_ = param_.getValue(KEY_PI_VERSION);

    DefaultParamHandler::updateMembers_();
  }

  /*
#ifdef WITH_GUI
  bool PeakInvestigator::getVersionDlg(void)
  {
    // Ask the user for a Version of PI to use and the min and max mass values
    VersionDialog verDlg("Peak Investigator", PI_versions_, LastUsedVersion_, CurrentVersion_, min_mass_, max_mass_);
    if(verDlg.exec() == QDialog::Accepted)
    {
      PIVersion_ = verDlg.version();
      int xmass = verDlg.maxMass();
      if(xmass > max_mass_)
      {
        LOG_ERROR << "The Maximum Mass must be less than " <<  max_mass_;
        return false;
      }
      else
      {
        max_mass_ = xmass;
      }
      xmass = verDlg.minMass();
      if(xmass > max_mass_)
      {
        LOG_ERROR << "The Minimum Mass must be less than the Maximum Mass";
        return false;
      }
      else
      {
        min_mass_ = xmass;
      }

      return true;
    }
    else
    {
      return false;
    }
  }

  bool PeakInvestigator::getRTODlg(void)
  {

    // Ask the user for a RTO to use
    RtoDialog rtoDlg("Peak Investigator " + PIVersion_, funds_, estimatedCosts_);
    if(rtoDlg.exec() == QDialog::Accepted)
    {
      RTO_ = rtoDlg.getRto();
      return true;
    }
    else
    {
      return false;
    }
  }

#endif // WITH_GUI
*/
}
