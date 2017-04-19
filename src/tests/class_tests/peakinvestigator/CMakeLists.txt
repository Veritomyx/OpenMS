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
# $Maintainer: David Rivkin $
# $Authors: David Rivkin, Adam Tenderholt $
# --------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
project("OpenMS_class_tests_peakinvestigator")
set(CMAKE_CXX_STANDARD 11)

set(PI_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/PeakInvestigator_test_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/PeakInvestigator_test_config.h
)

# --------------------------------------------------------------------------
# add include directories
include_directories(SYSTEM ${OpenMS_INCLUDE_DIRECTORIES} 
    ${CMAKE_SOURCE_DIR}/src/peakinvestigator/include
    ${CMAKE_BINARY_DIR}/src/peakinvestigator/include
    ${CMAKE_CURRENT_BINARY_DIR}
)

# --------------------------------------------------------------------------
# list of available tests
set(PI_TESTS
    ConsoleVersionDialog_test
    ConsoleInitDialog_test
    ConsolePasswordDialog_test
)

# --------------------------------------------------------------------------
# add targets for the executables
foreach(i ${PI_TESTS})
  add_executable(${i} ${i}.cpp)
  target_link_libraries(${i} PeakInvestigator OpenMS)
  add_test(${i} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${i})
endforeach(i)

