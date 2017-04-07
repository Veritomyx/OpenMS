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

#include <OpenMS/DATASTRUCTURES/String.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/GUIVersionDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/UIC/ui_GUIVersionDialog.h>

namespace OpenMS
{

  GUIVersionDialog::GUIVersionDialog(String title, std::list<std::string> versions, String current, String previous)
    : QDialog(), AbstractVersionDialog(title, versions, current, previous), ui_(new Ui::VersionDialog())
  {
    ui_->setupUi(this);
    versions_ = versions;

    ui_->comboBox->clear();
    std::list<std::string>::const_iterator iter;

    for(iter = versions.begin(); iter != versions.end(); iter++)
    {
      QString version = QString::fromStdString(*iter);
      QString current_ = current.toQString();
      QString previous_ = previous.toQString();

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

      ui_->comboBox->addItem(version);
    }

    ui_->comboBox->setCurrentIndex(0);
  }

  GUIVersionDialog::~GUIVersionDialog()
  {
    delete ui_;
  }

  bool GUIVersionDialog::exec()
  {
    int retval = QDialog::exec();
    return retval == QDialog::Accepted;
  }

  void GUIVersionDialog::on_comboBox_currentIndexChanged(int index)
  {
    std::list<std::string>::const_iterator iter = versions_.begin();
    for(int i = 0; i <= index; i++)
    {
      selectedVersion_ = *iter;
      ++iter;
    }
  }

}