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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/GUIInitDialog.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/UIC/ui_GUIInitDialog.h>

#include <QLabel>

using Veritomyx::PeakInvestigator::ResponseTimeCosts;

namespace OpenMS
{
  GUIInitDialog::GUIInitDialog(String title, EstimatedCosts costs, double funds)
    : QDialog(), AbstractInitDialog(title, costs, funds), ui_(new Ui::InitDialog())
  {
    ui_->setupUi(this);

    std::list<std::string> instruments = costs.getInstruments();
    std::map<std::string, int> headers;

    std::list<std::string>::const_iterator instrument;
    std::map<std::string, double>::const_iterator RTO;
    std::map<std::string, int>::const_iterator header;

    int row;
    for(instrument = instruments.begin(), row = 1; instrument != instruments.end(); ++instrument, ++row)
    {

      QLabel* instrument_label = new QLabel(QString::fromStdString(*instrument), this);
      ui_->gridLayout->addWidget(instrument_label, row, 0, 1, 1);

      ResponseTimeCosts RTOs = costs.forInstrument(*instrument);
      for(RTO = RTOs.begin(); RTO != RTOs.end(); ++RTO)
      {
        int col;
        header = headers.find(RTO->first);
        if(header != headers.end())
        {
          col = header->second;
        }
        else
        {
          col = headers.size() + 1;
          headers[RTO->first] = col;
        }

        char buffer[50];
        if (std::snprintf(buffer, 50, "%.2f", RTO->second) < 0)
        {
          buffer[0] = 'X';
          buffer[1] = '\0';
        }

        QLabel* cost_label = new QLabel(QString(buffer), this);
        ui_->gridLayout->addWidget(cost_label, row, col, 1, 1, Qt::AlignCenter);
      }
    }

    for(header = headers.begin(); header != headers.end(); ++header)
    {
      QString text = QString::fromStdString("<u>" + header->first + "</u>");
      QLabel* rto_label = new QLabel(text, this);
      rto_label->setTextFormat(Qt::RichText);
      ui_->gridLayout->addWidget(rto_label, 0, header->second, 1, 1, Qt::AlignCenter);
      ui_->comboBox->addItem(QString::fromStdString(header->first));
    }

    ui_->comboBox->setCurrentIndex(0);
  }

  GUIInitDialog::~GUIInitDialog()
  {
    delete ui_;
  }

  bool GUIInitDialog::exec()
  {
    int retval = QDialog::exec();
    return retval == QDialog::Accepted;
  }

  void GUIInitDialog::on_comboBox_currentIndexChanged(int index)
  {
    selectedRTO_ = ui_->comboBox->itemText(index);
  }
}
