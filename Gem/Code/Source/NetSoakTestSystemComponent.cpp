/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include <AzNetworking/Framework/INetworking.h>

#include "NetSoakTestSystemComponent.h"

namespace AzNetworking
{
    enum class SoakMode
    {
        Loopback,
        Client,
        Host
    };
}

namespace AZ::ConsoleTypeHelpers
{
    template <>
    inline CVarFixedString ValueToString(const AzNetworking::ProtocolType& value)
    {
        return (value == AzNetworking::ProtocolType::Tcp) ? "tcp" : "udp";
    }

    template <>
    inline bool StringSetToValue<AzNetworking::ProtocolType>(AzNetworking::ProtocolType& outValue, const ConsoleCommandContainer& arguments)
    {
        if (!arguments.empty())
        {
            if (arguments.front() == "tcp")
            {
                outValue = AzNetworking::ProtocolType::Tcp;
                return true;
            }
            else if (arguments.front() == "udp")
            {
                outValue = AzNetworking::ProtocolType::Udp;
                return true;
            }
        }
        return false;
    }

    template <>
    inline CVarFixedString ValueToString(const AzNetworking::SoakMode& value)
    {
        if (value == AzNetworking::SoakMode::Client)
        {
            return "client";
        }
        if (value == AzNetworking::SoakMode::Host)
        {
            return "host";
        }
        return "loopback";
    }

    template <>
    inline bool StringSetToValue<AzNetworking::SoakMode>(AzNetworking::SoakMode& outValue, const ConsoleCommandContainer& arguments)
    {
        if (!arguments.empty())
        {
            if (arguments.front() == "client")
            {
                outValue = AzNetworking::SoakMode::Client;
                return true;
            }
            else if (arguments.front() == "host")
            {
                outValue = AzNetworking::SoakMode::Host;
                return true;
            }
        }
        outValue = AzNetworking::SoakMode::Loopback;
        return true;
    }
}

namespace NetSoakTest
{
    using namespace AzNetworking;

    static const AZStd::string_view s_networkInterfaceName("NetSoakNetworkInterface");
    static const AZStd::string_view s_loopbackInterfaceName("NetSoakLoopbackInterface");

    AZ_CVAR(AZ::TimeMs, soak_latencyms, AZ::TimeMs(0), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Simulated connection quality latency");
    AZ_CVAR(AZ::TimeMs, soak_variancems, AZ::TimeMs(0), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Simulated connection quality variance");
    AZ_CVAR(uint16_t, soak_losspercentage, 0, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Simulated connection quality packet drop rate");
    AZ_CVAR(AZ::CVarFixedString, soak_serveraddr, AZ::CVarFixedString("127.0.0.1"), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "The address of the server or host to connect to");
    AZ_CVAR(uint16_t, soak_port, 33450, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "The port that this soak test will bind to for game traffic");
    AZ_CVAR(ProtocolType, soak_protocol, ProtocolType::Udp, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Soak test protocol");
    AZ_CVAR(SoakMode, soak_mode, SoakMode::Loopback, nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "Soak test mode");
    AZ_CVAR(SoakMode, soak_runtime, AZ::TimeMs(0), nullptr, AZ::ConsoleFunctorFlags::DontReplicate, "How long to run the soak test for before dumping stats");

    void NetSoakTestSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<NetSoakTestSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<NetSoakTestSystemComponent>("NetSoakTest", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void NetSoakTestSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("NetSoakTestService"));
    }

    void NetSoakTestSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("NetSoakTestService"));
    }

    void NetSoakTestSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("NetworkingService"));
    }

    void NetSoakTestSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void NetSoakTestSystemComponent::Init()
    {
    }

    void NetSoakTestSystemComponent::Activate()
    {
        m_networkInterface = AZ::Interface<INetworking>::Get()->CreateNetworkInterface(AZ::Name(s_networkInterfaceName), soak_protocol, TrustZone::ExternalClientToServer, *this);
        m_loopbackInterface = AZ::Interface<INetworking>::Get()->CreateNetworkInterface(AZ::Name(s_loopbackInterfaceName), soak_protocol, TrustZone::ExternalClientToServer, *this);

        if (soak_mode == SoakMode::Loopback || soak_mode == SoakMode::Host)
        {
            m_networkInterface->Listen(soak_port);
        }

        if (soak_mode == SoakMode::Loopback || soak_mode == SoakMode::Client)
        {
            const AZ::CVarFixedString localAddress = soak_mode == SoakMode::Loopback ? AZ::CVarFixedString("127.0.0.1") : soak_serveraddr;
            const IpAddress ipAddress(localAddress.c_str(), soak_port, m_loopbackInterface->GetType());
            m_loopbackInterface->Connect(ipAddress);
        }

        NetSoakTestRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void NetSoakTestSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        NetSoakTestRequestBus::Handler::BusDisconnect();
    }

    void NetSoakTestSystemComponent::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        AZ::TimeMs elapsedMs = aznumeric_cast<AZ::TimeMs>(aznumeric_cast<int64_t>(deltaTime / 1000.0f));

        total_elapsedMs += elapsedMs;
        if (soak_runtime != AZ::TimeMs(0) && total_elapsedMs > soak_runtime)
        {
            DumpSoakStats();
            exit(0);
        }

        NetSoakTestPackets::Small packet;

        auto visitor = [&packet](IConnection& connection)
        {
            connection.SendReliablePacket(packet);

            uint32_t rand_int = rand();
            if (rand_int % 10 == 1)
            {
                NetSoakTest::TestNetworkBuffer buffer = NetSoakTest::TestNetworkBuffer(NetSoakTest::MassiveBuffer);
                NetSoakTestPackets::Large* large = new NetSoakTestPackets::Large(buffer);
                connection.SendReliablePacket(*large);
                delete large;
            }
            else if (rand_int % 10 == 2)
            {
                NetSoakTestPackets::Small unreliable;
                // Flag the unreliable packet by setting a specific datum value
                unreliable.SetSmallDatum(2);
                connection.SendUnreliablePacket(unreliable);
            }
            
        };

        m_networkInterface->GetConnectionSet().VisitConnections(visitor);
        m_loopbackInterface->GetConnectionSet().VisitConnections(visitor);
    }

    int NetSoakTestSystemComponent::GetTickOrder()
    {
        // Tick immediately after the multiplayer system component
        return AZ::TICK_PLACEMENT + 2;
    }

    bool NetSoakTestSystemComponent::IsHandshakeComplete() const
    {
        return true;
    }

    bool NetSoakTestSystemComponent::HandleRequest([[maybe_unused]] IConnection* connection,
        [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] const NetSoakTestPackets::Small& packet)
    {
        return true;
    }

    bool NetSoakTestSystemComponent::HandleRequest([[maybe_unused]] IConnection* connection,
        [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] const NetSoakTestPackets::Large& packet)
    {
        return true;
    }

    ConnectResult NetSoakTestSystemComponent::ValidateConnect([[maybe_unused]] const IpAddress& remoteAddress,
        [[maybe_unused]] const IPacketHeader& packetHeader, [[maybe_unused]] ISerializer& serializer)
    {
        return ConnectResult::Accepted;
    }

    void NetSoakTestSystemComponent::OnConnect(IConnection* connection)
    {
        ConnectionQuality& testQuality = connection->GetConnectionQuality();
        testQuality.m_latencyMs = soak_latencyms;
        testQuality.m_lossPercentage = soak_losspercentage;
        testQuality.m_varianceMs = soak_variancems;

        if (connection->GetConnectionRole() == ConnectionRole::Connector)
        {
            AZLOG_INFO("New outgoing connection to remote address: %s", connection->GetRemoteAddress().GetString().c_str());
        }
        else
        {
            AZLOG_INFO("New incoming connection from remote address: %s", connection->GetRemoteAddress().GetString().c_str());
        }
    }

    PacketDispatchResult NetSoakTestSystemComponent::OnPacketReceived(IConnection* connection, const IPacketHeader& packetHeader, ISerializer& serializer)
    {
        return NetSoakTestPackets::DispatchPacket(connection, packetHeader, serializer, *this);
    }

    void NetSoakTestSystemComponent::OnPacketLost([[maybe_unused]] IConnection* connection, [[maybe_unused]] PacketId packetId)
    {
        ;
    }

    void NetSoakTestSystemComponent::OnDisconnect(IConnection* connection, [[maybe_unused]] DisconnectReason reason, [[maybe_unused]] AzNetworking::TerminationEndpoint endpoint)
    {
        AZLOG_INFO("Disconnected from remote address: %s", connection->GetRemoteAddress().GetString().c_str());
    }

    void DumpSoakStats([[maybe_unused]] const AZ::ConsoleCommandContainer& arguments)
    {
        AZStd::vector<INetworkInterface*> networkInterfaces;
        networkInterfaces.push_back(AZ::Interface<INetworking>::Get()->RetrieveNetworkInterface(AZ::Name(s_networkInterfaceName)));
        networkInterfaces.push_back(AZ::Interface<INetworking>::Get()->RetrieveNetworkInterface(AZ::Name(s_loopbackInterfaceName)));

        for (auto& networkInterface : networkInterfaces)
        {
            const char* protocol = networkInterface->GetType() == ProtocolType::Tcp ? "Tcp" : "Udp";
            const char* trustZone = networkInterface->GetTrustZone() == TrustZone::ExternalClientToServer ? "ExternalClientToServer" : "InternalServerToServer";
            const uint32_t port = aznumeric_cast<uint32_t>(networkInterface->GetPort());
            AZLOG_INFO("%sNetworkInterface: %s - open to %s on port %u", protocol, networkInterface->GetName().GetCStr(), trustZone, port);

            const NetworkInterfaceMetrics& metrics = networkInterface->GetMetrics();
            AZLOG_INFO(" - Total time spent updating in milliseconds: %lld", aznumeric_cast<AZ::s64>(metrics.m_updateTimeMs));
            AZLOG_INFO(" - Total number of connections: %llu", aznumeric_cast<AZ::u64>(metrics.m_connectionCount));
            AZLOG_INFO(" - Total send time in milliseconds: %lld", aznumeric_cast<AZ::s64>(metrics.m_sendTimeMs));
            AZLOG_INFO(" - Total sent packets: %llu", aznumeric_cast<AZ::s64>(metrics.m_sendPackets));
            AZLOG_INFO(" - Total sent bytes after compression: %llu", aznumeric_cast<AZ::u64>(metrics.m_sendBytes));
            AZLOG_INFO(" - Total sent bytes before compression: %llu", aznumeric_cast<AZ::u64>(metrics.m_sendBytesUncompressed));
            AZLOG_INFO(" - Total sent compressed packets without benefit: %llu", aznumeric_cast<AZ::u64>(metrics.m_sendCompressedPacketsNoGain));
            AZLOG_INFO(" - Total gain from packet compression: %lld", aznumeric_cast<AZ::s64>(metrics.m_sendBytesCompressedDelta));
            AZLOG_INFO(" - Total packets resent: %llu", aznumeric_cast<AZ::u64>(metrics.m_resentPackets));
            AZLOG_INFO(" - Total receive time in milliseconds: %lld", aznumeric_cast<AZ::s64>(metrics.m_recvTimeMs));
            AZLOG_INFO(" - Total received packets: %llu", aznumeric_cast<AZ::u64>(metrics.m_recvPackets));
            AZLOG_INFO(" - Total received bytes after compression: %llu", aznumeric_cast<AZ::u64>(metrics.m_recvBytes));
            AZLOG_INFO(" - Total received bytes before compression: %llu", aznumeric_cast<AZ::u64>(metrics.m_recvBytesUncompressed));
            AZLOG_INFO(" - Total packets discarded due to load: %llu", aznumeric_cast<AZ::u64>(metrics.m_discardedPackets));
        }
    }
    AZ_CONSOLEFREEFUNC(DumpSoakStats, AZ::ConsoleFunctorFlags::Null, "Dump soak test networking stats out");
}
