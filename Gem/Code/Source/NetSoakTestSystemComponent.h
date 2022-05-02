/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */
#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

#include <AzNetworking/ConnectionLayer/IConnectionListener.h>

#include <NetSoakTest/NetSoakTestBus.h>

#include "Source/AutoGen/NetSoakTest.AutoPacketDispatcher.h"

namespace AzNetworking
{
    class INetworkInterface;
}

namespace NetSoakTest
{
    class NetSoakTestSystemComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzNetworking::IConnectionListener
        , protected NetSoakTestRequestBus::Handler
    {
    public:
        AZ_COMPONENT(NetSoakTestSystemComponent, "{C048F4A4-A1BA-4673-B668-967B587F2275}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        bool IsHandshakeComplete() const;
        bool HandleRequest(AzNetworking::IConnection* connection, const AzNetworking::IPacketHeader& packetHeader, const NetSoakTestPackets::Small& packet);
        bool HandleRequest(AzNetworking::IConnection* connection, const AzNetworking::IPacketHeader& packetHeader, const NetSoakTestPackets::Large& packet);

    protected:
        ////////////////////////////////////////////////////////////////////////
        // NetSoakTestRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////

        
        // AZ::TickBus::Handler overrides
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
        int GetTickOrder() override;
        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // IConnectionListener interface
        AzNetworking::ConnectResult ValidateConnect(const AzNetworking::IpAddress& remoteAddress, const AzNetworking::IPacketHeader& packetHeader, AzNetworking::ISerializer& serializer) override;
        void OnConnect(AzNetworking::IConnection* connection) override;
        AzNetworking::PacketDispatchResult OnPacketReceived(AzNetworking::IConnection* connection, const AzNetworking::IPacketHeader& packetHeader, AzNetworking::ISerializer& serializer) override;
        void OnPacketLost(AzNetworking::IConnection* connection, AzNetworking::PacketId packetId) override;
        void OnDisconnect(AzNetworking::IConnection* connection, AzNetworking::DisconnectReason reason, AzNetworking::TerminationEndpoint endpoint) override;
        ////////////////////////////////////////////////////////////////////////

    private:
        AzNetworking::INetworkInterface* m_networkInterface = nullptr;
        AzNetworking::INetworkInterface* m_loopbackInterface = nullptr;

        AZ::TimeMs m_totalElapsedMs = AZ::TimeMs(0);
    };
}
