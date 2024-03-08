// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <random>
#include "core/hle/service/service.h"

namespace Core {
class System;
}

namespace Service::SSL {

class SSL_C final : public ServiceFramework<SSL_C> {
public:
    SSL_C();

private:
    void Initialize(Kernel::HLERequestContext& ctx);
    void GenerateRandomData(Kernel::HLERequestContext& ctx);

    std::minstd_rand deterministic_random_gen{0};

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& boost::serialization::base_object<Kernel::SessionRequestHandler>(*this);
        ar & deterministic_random_gen;
    }

    friend class boost::serialization::access;
};

void InstallInterfaces(Core::System& system);

void GenerateRandomData(std::vector<u8>& out);

} // namespace Service::SSL

BOOST_CLASS_EXPORT_KEY(Service::SSL::SSL_C)
