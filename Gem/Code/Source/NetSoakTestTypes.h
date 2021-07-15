/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 * 
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/RTTI/TypeSafeIntegral.h>
#include <AzCore/std/string/fixed_string.h>

namespace NetSoakTest
{
    AZ_TYPE_SAFE_INTEGRAL(HostId, uint32_t);
    static constexpr HostId InvalidHostId = static_cast<HostId>(-1);
    static constexpr size_t TestSize = 6400;

    using TestNetworkBuffer = AZStd::basic_fixed_string<char, TestSize, AZStd::char_traits<char>>;

    constexpr TestNetworkBuffer CreateMassiveBuffer()
    {
        constexpr char hexchar[] = "0123456789ABCDEF";

        TestNetworkBuffer rBuffer;
        for (int i = 0; i < TestSize; ++i)
        {
            rBuffer.append(1, hexchar[i % (sizeof(hexchar) - 1)]);
        }

        return rBuffer;
    }
    constexpr TestNetworkBuffer MassiveBuffer = CreateMassiveBuffer();
}

AZ_TYPE_SAFE_INTEGRAL_SERIALIZEBINDING(NetSoakTest::HostId);
