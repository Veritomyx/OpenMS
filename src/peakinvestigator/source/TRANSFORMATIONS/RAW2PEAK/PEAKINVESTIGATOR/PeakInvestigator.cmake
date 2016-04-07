# --------------------------------------------------------------------------
#                   OpenMS -- Open-Source Mass Spectrometry
# --------------------------------------------------------------------------
# Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
# ETH Zurich, and Freie Universitaet Berlin 2016.
#
# This software is released under a three-clause BSD license:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of any author or any participating institution
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# For a full list of authors, refer to the file AUTHORS.
# --------------------------------------------------------------------------
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
# INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# --------------------------------------------------------------------------
# $Maintainer: Adam Tenderholt $
# $Authors: Adam Tenderholt$
# --------------------------------------------------------------------------

### the directory names
set(PI_HEADER_DIR include/OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR)
set(PI_SOURCE_DIR source/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR)


set(PI_HEADERS
    PeakInvestigator.h
    DIALOGS/AbstractDialogFactory.h
    DIALOGS/ConsoleDialogFactory.h
    DIALOGS/AbstractInitDialog.h
    DIALOGS/ConsoleInitDialog.h
    DIALOGS/AbstractVersionDialog.h
    DIALOGS/ConsoleVersionDialog.h
)

set(PI_SOURCES
    PeakInvestigator.cpp
    DIALOGS/ConsoleDialogFactory.cpp
    DIALOGS/ConsoleInitDialog.cpp
    DIALOGS/ConsoleVersionDialog.cpp
)

#if(WITH_GUI)
#    list(APPEND PI_HEADERS
#        DIALOGS/GUIDialogFactory.h
#        DIALOGS/GUIVersionDialog.h
#        DIALOGS/GUIInitDialog.h
#        )
#endif()

set(PeakInvestigatorHeaders)
foreach(I ${PI_HEADERS})
    list(APPEND PeakInvestigatorHeaders ${PI_HEADER_DIR}/${I})
endforeach()

set(PeakInvestigatorSources)
foreach(I ${PI_SOURCES})
    list(APPEND PeakInvestigatorSources ${PI_SOURCE_DIR}/${I})
endforeach()

# combine list of files for building library
set(PeakInvestigatorFiles)
list(APPEND PeakInvestigatorFiles ${PeakInvestigatorHeaders})
list(APPEND PeakInvestigatorFiles ${PeakInvestigatorSources})

# setup header groups for IDEs
set(PI_DIALOG_HEADERS ${PeakInvestigatorHeaders})
list(REMOVE_ITEM PI_DIALOG_HEADERS ${PI_HEADER_DIR}/PeakInvestigator.h)
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR"
    FILES ${PI_HEADER_DIR}/PeakInvestigator.h
)
source_group("Header Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\DIALOGS"
    FILES ${PI_DIALOG_HEADERS}
)

# setup source groups for IDEs
set(PI_DIALOG_SOURCES ${PeakInvestigatorSources})
list(REMOVE_ITEM PI_DIALOG_SOURCES ${PI_HEADER_DIR}/PeakInvestigator.cpp)
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR"
    FILES ${PI_SOURCE_DIR}/PeakInvestigator.cpp
)
source_group("Source Files\\TRANSFORMATIONS\\RAW2PEAK\\PEAKINVESTIGATOR\\DIALOGS"
    FILES ${PI_DIALOG_SOURCES}
)

