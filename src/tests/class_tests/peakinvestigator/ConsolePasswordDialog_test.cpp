// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2017.
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
//

#include "PeakInvestigator_test_config.h"

#include <OpenMS/CONCEPT/ClassTest.h>
#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsolePasswordDialog.h>
#include <OpenMS/DATASTRUCTURES/String.h>

#include <functional>

using OpenMS::AbstractPasswordDialog;
using OpenMS::ConsolePasswordDialog;
using OpenMS::String;

class KeyboardSimulator
{
  public:
    KeyboardSimulator(String text)
    {
      text_ = text;
      index_ = 0;
    }

    int getChar()
    {
      return text_.at(index_++);
    }

  private:
    String text_;
    int index_;
};

//typedef struct KeyboardSimulator_t
//{
//    String text_;
//    int index_;

//    int getChar() { return text_.at(index_++); }
//    int (*getCharPtr)();

//} KeyboardSimulator;

START_TEST(ConsoleInitDialog, "$Id")

START_SECTION(KeyboardSimulator test)
  KeyboardSimulator simulator("abc");

  TEST_EQUAL(static_cast<char>(simulator.getChar()), 'a');
  TEST_EQUAL(static_cast<char>(simulator.getChar()), 'b');
  TEST_EQUAL(static_cast<char>(simulator.getChar()), 'c');
END_SECTION

START_SECTION(exec() with simple password)
  KeyboardSimulator simulator("abc\n");
  std::function<int(void)> func = std::bind( &KeyboardSimulator::getChar, simulator);

  AbstractPasswordDialog *dialog = new ConsolePasswordDialog(func);

  TEST_EQUAL(dialog->exec(), true);

  String password = dialog->getPassword();
  delete dialog;

  TEST_STRING_EQUAL(password, "abc");
END_SECTION

START_SECTION(exec() with complex password)
  KeyboardSimulator simulator("4bc?d5\n");
  std::function<int(void)> func = std::bind( &KeyboardSimulator::getChar, simulator);

  AbstractPasswordDialog *dialog = new ConsolePasswordDialog(func);

  TEST_EQUAL(dialog->exec(), true);

  String password = dialog->getPassword();
  delete dialog;

  TEST_STRING_EQUAL(password, "4bc?d5");
END_SECTION

START_SECTION(exec() with backspace)
  KeyboardSimulator simulator("abC\bc\n");
  std::function<int(void)> func = std::bind( &KeyboardSimulator::getChar, simulator);

  AbstractPasswordDialog *dialog = new ConsolePasswordDialog(func);

  TEST_EQUAL(dialog->exec(), true);

  String password = dialog->getPassword();
  delete dialog;

  TEST_STRING_EQUAL(password, "abc");
END_SECTION

END_TEST
