// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright Veritomyx, Inc. 2016.
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
// $Author: David Rivkin $
// --------------------------------------------------------------------------
//

#ifndef VERSIONDIALOG_H
#define VERSIONDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFrame>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QString>
#include <QStringList>

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigatorSaaSConfig.h>

namespace OpenMS
{

  class PEAKINVESTIGATORSAAS_DLLAPI VersionDialog : public QDialog
  {

      Q_OBJECT

    public:
      VersionDialog(QString title, QStringList versions, QString lastUsedVersion, QString currentVersion,
                    int minMass, int maxMass, QWidget * parent = 0, Qt::WindowFlags f = 0);
      QString version(void) { return verSelect->currentText(); }
      int minMass(void) { return minEdit->value(); }
      int maxMass(void) { return maxEdit->value(); }

    public slots:
      void newMax(int value);
      void newMin(int value);

    protected:
      QVBoxLayout *mainLayout;
      QGroupBox *verGB;
      QFormLayout *verForm;
      QComboBox *verSelect;
      QGroupBox *massGB;
      QFormLayout *form;
      QSpinBox *maxEdit;
      QSpinBox *minEdit;
      QFrame *btnFrame;
      QPushButton *okBtn;
      QPushButton *rejectBtn;
      QHBoxLayout *buttonLayout;

  };

}

#endif // VERSIONDIALOG_H
