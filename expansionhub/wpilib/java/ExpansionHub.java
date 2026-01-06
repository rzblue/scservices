// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

package frc.robot;

import edu.wpi.first.networktables.BooleanSubscriber;
import edu.wpi.first.networktables.DoubleSubscriber;
import edu.wpi.first.networktables.NetworkTableInstance;
import edu.wpi.first.wpilibj.SystemServer;

public class ExpansionHub {
  private final DoubleSubscriber batteryVoltageSubscriber;
  private final BooleanSubscriber connectedSubscriber;
  private final int hubNumber;

  public ExpansionHub(int hubNumber) {
    if (hubNumber < 0 || hubNumber > 3) {
      throw new IllegalArgumentException(
          "Hub number out of range, must be between 0 and 3 inclusive (Matching USB numbers)");
    }

    this.hubNumber = hubNumber;

    NetworkTableInstance systemServer = SystemServer.getSystemServer();

    batteryVoltageSubscriber = systemServer.getDoubleTopic("/rhsp/" + hubNumber + "/battery").subscribe(0);
    connectedSubscriber = systemServer.getBooleanTopic("/rhsp/" + hubNumber + "/connected").subscribe(false);
  }

  public boolean isConnected() {
    return connectedSubscriber.get(false);
  }

  public double getBatteryVoltage() {
    return batteryVoltageSubscriber.get(0.0);
  }

  public ExpansionHubMotor getMotor(int motorNumber) {
    return new ExpansionHubMotor(hubNumber, motorNumber);
  }
}
