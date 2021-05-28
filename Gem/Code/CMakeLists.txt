#
# All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
# its licensors.
#
# For complete copyright and license terms please see the LICENSE at the root of this
# distribution (the "License"). All use of this software is governed by the License,
# or, if provided, by the license below or the license accompanying this file. Do not
# remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

ly_get_list_relative_pal_filename(pal_dir ${CMAKE_CURRENT_LIST_DIR}/Platform/${PAL_PLATFORM_NAME})

ly_add_target(
    NAME NetSoakTest.Static STATIC
    NAMESPACE Gem
    FILES_CMAKE
        netsoaktest_files.cmake
        netsoaktest_autogen_files.cmake
        ${pal_dir}/netsoaktest_${PAL_PLATFORM_NAME_LOWERCASE}_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            AZ::AzNetworking
            .
        PUBLIC
            Include
    BUILD_DEPENDENCIES
        PUBLIC
            AZ::AzGameFramework
            AZ::AzNetworking
    AUTOGEN_RULES
        *.AutoPackets.xml,AutoPackets_Header.jinja,$path/$fileprefix.AutoPackets.h
        *.AutoPackets.xml,AutoPackets_Inline.jinja,$path/$fileprefix.AutoPackets.inl
        *.AutoPackets.xml,AutoPackets_Source.jinja,$path/$fileprefix.AutoPackets.cpp
        *.AutoPackets.xml,AutoPacketDispatcher_Header.jinja,$path/$fileprefix.AutoPacketDispatcher.h
        *.AutoPackets.xml,AutoPacketDispatcher_Inline.jinja,$path/$fileprefix.AutoPacketDispatcher.inl
)

ly_add_target(
    NAME NetSoakTest ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
    NAMESPACE Gem
    OUTPUT_NAME Gem.NetSoakTest.5111dee500ed4812a7905093a73d3837.v0.1.0
    FILES_CMAKE
        netsoaktest_shared_files.cmake
    INCLUDE_DIRECTORIES
        PRIVATE
            Source
            .
    BUILD_DEPENDENCIES
        PRIVATE
            Gem::Atom_AtomBridge.Static
            Gem::NetSoakTest.Static
)

################################################################################
# Gem dependencies
################################################################################

if(PAL_TRAIT_BUILD_HOST_TOOLS)
    ly_add_project_dependencies(
        PROJECT_NAME
            NetSoakTest
        TARGETS
            AssetBuilder
            AssetProcessor
            AssetProcessorBatch
            Editor
        DEPENDENCIES_FILES
            tool_dependencies.cmake
    )
endif()

if(PAL_TRAIT_BUILD_SERVER_SUPPORTED)
    ly_add_target(
        NAME NetSoakTest.Server ${PAL_TRAIT_MONOLITHIC_DRIVEN_MODULE_TYPE}
        NAMESPACE Gem
        OUTPUT_NAME Gem.NetSoakTest.Server.5111dee500ed4812a7905093a73d3837.v0.1.0
        FILES_CMAKE
            netsoaktest_shared_files.cmake
        INCLUDE_DIRECTORIES
            PRIVATE
                Source
                .
        BUILD_DEPENDENCIES
            PRIVATE
                Gem::NetSoakTest.Static
    )

    ly_add_project_dependencies(
        PROJECT_NAME
            NetSoakTest
        TARGETS
            NetSoakTest.ServerLauncher
        DEPENDENCIES_FILES
            runtime_dependencies.cmake
    )
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_SERVER_PROJECTS NetSoakTest)

endif()
