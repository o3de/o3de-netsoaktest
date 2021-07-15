#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
# 
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
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

# if enabled, NetSoakTest is used by all kinds of applications
ly_create_alias(NAME NetSoakTest.Clients  NAMESPACE Gem TARGETS Gem::NetSoakTest)
ly_create_alias(NAME NetSoakTest.Servers  NAMESPACE Gem TARGETS Gem::NetSoakTest)

################################################################################
# Gem dependencies
################################################################################

# The GameLauncher uses "Clients" gem variants:
ly_enable_gems(PROJECT_NAME NetSoakTest GEM_FILE enabled_gems.cmake
    TARGETS NetSoakTest.GameLauncher
    VARIANTS Clients)

# If we build a server, then apply the gems to the server
if(PAL_TRAIT_BUILD_SERVER_SUPPORTED)
    # if we're making a server, then add the "Server" gem variants to it:
    ly_enable_gems(PROJECT_NAME NetSoakTest GEM_FILE enabled_gems.cmake
        TARGETS NetSoakTest.ServerLauncher
        VARIANTS Servers)
    
    set_property(GLOBAL APPEND PROPERTY LY_LAUNCHER_SERVER_PROJECTS NetSoakTest)
endif()

if (PAL_TRAIT_BUILD_HOST_TOOLS)
    # The Editor uses "Tools" gem variants:
    ly_enable_gems(
        PROJECT_NAME NetSoakTest GEM_FILE enabled_gems.cmake
        TARGETS Editor
        VARIANTS Tools)

    # The pipeline tools use "Builders" gem variants:
    ly_enable_gems(
        PROJECT_NAME NetSoakTest GEM_FILE enabled_gems.cmake
        TARGETS AssetBuilder AssetProcessor AssetProcessorBatch
        VARIANTS Builders)
endif()
