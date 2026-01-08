// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <string>

#include <wpi/nt/BooleanTopic.hpp>
#include <wpi/nt/IntegerTopic.hpp>
#include <wpi/nt/NetworkTableInstance.hpp>

#include "CachedCommand.h"

namespace eh {

struct ServoNtState {
    CachedCommand<wpi::nt::BooleanSubscriber> enabledSubscriber;
    wpi::nt::IntegerSubscriber pulseWidthSubscriber;
    CachedCommand<wpi::nt::IntegerSubscriber> framePeriodSubscriber;

    void Initialize(const wpi::nt::NetworkTableInstance& instance, int servoNum,
                    const std::string& busIdStr,
                    wpi::nt::PubSubOptions options);
};

}  // namespace eh
