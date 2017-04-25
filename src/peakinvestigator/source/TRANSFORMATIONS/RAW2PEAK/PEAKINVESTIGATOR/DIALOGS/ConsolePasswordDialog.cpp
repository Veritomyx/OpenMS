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

#include <OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/DIALOGS/ConsolePasswordDialog.h>

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace OpenMS
{
#if defined(_WIN32)
  HANDLE ConsolePasswordDialog::hIn = GetStdHandle(STD_INPUT_HANDLE);
#endif

  ConsolePasswordDialog::ConsolePasswordDialog(std::function<int(void)> get_character)
    : AbstractPasswordDialog()
  {
    get_character_ = get_character;

#if defined(_WIN32)
	GetConsoleMode(hIn, &console_mode);
	SetConsoleMode(hIn, console_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
#endif
  }

  ConsolePasswordDialog::~ConsolePasswordDialog()
  {
#if defined(_WIN32)
	  SetConsoleMode(hIn, console_mode);
#endif
  }

  bool ConsolePasswordDialog::exec()
  {
    const char BACKSPACE_CODE = 8;
#if defined(__APPLE__) || defined(__linux__)
    const char RETURN_CODE = 10;
#endif
#if defined(_WIN32)
	const char RETURN_CODE = 13;
#endif

    const char ESCAPE_CODE = 27;
    const char DELETE_CODE = 127;

    unsigned char ch = 0;

    std::cout << "\nPassword: ";

    while((ch = get_character_()) != RETURN_CODE)
    {
      if (ch == BACKSPACE_CODE || ch == DELETE_CODE)
      {
        if (password_.length() != 0)
        {
          std::cout << "\b \b";
          password_.resize(password_.length() - 1);
        }
      }

      else if (ch != ESCAPE_CODE)
      {
        password_ += static_cast<char>(ch);
        std::cout << "*";
      }
    }

    std::cout << std::endl;

    return true;
  }

}
