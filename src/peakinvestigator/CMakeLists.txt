project(PeakInvestigator)
option(WITH_PEAKINVESTIGATOR "Build PeakInvestigator library and applications." ON)

if(WITH_PEAKINVESTIGATOR)
    find_package(PeakInvestigatorSaaS REQUIRED)
    include(source/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/PeakInvestigator.cmake)
    openms_add_library(TARGET_NAME PeakInvestigator
                   SOURCE_FILES ${PeakInvestigatorFiles} ${PI_UIC_SOURCES} ${PI_MOC_SOURCES}
                   HEADER_FILES ${PeakInvestigatorHeaders}
                   INTERNAL_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include
                                     ${CMAKE_CURRENT_BINARY_DIR}/include
                   EXTERNAL_INCLUDES ${OpenMS_INCLUDE_DIRECTORIES}
                   LINK_LIBRARIES OpenMS ${OpenMS_GUI_DEP_LIBRARIES} Veritomyx::PeakInvestigatorSaaS
                   DLL_EXPORT_PATH "OpenMS/TRANSFORMATIONS/RAW2PEAK/PEAKINVESTIGATOR/")
    openms_doc_path(${CMAKE_CURRENT_SOURCE_DIR}/include)
endif(WITH_PEAKINVESTIGATOR)
