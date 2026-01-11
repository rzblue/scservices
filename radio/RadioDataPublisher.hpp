#pragma once

#include <string>
#include <string_view>

#include <wpi/util/json.hpp>
#include <wpi/nt/NetworkTableInstance.hpp>
#include <wpi/nt/StructTopic.hpp>
#include <wpi/util/struct/Struct.hpp>

struct NetworkStatus {
    std::string ssid;
    std::string hashedWpaKey;
    std::string wpaKeySalt;
    bool isLinked;
    std::string macAddress;
    int64_t signalDbm;
    int64_t noiseDbm;
    int64_t signalNoiseRatio;
    double rxRateMbps;
    int64_t rxPackets;
    int64_t rxBytes;
    double txRateMbps;
    int64_t txPackets;
    int64_t txBytes;
    double bandwidthUsedMbps;
    std::string connectionQuality;
};

void to_json(wpi::util::json& j, const NetworkStatus& ns);
void from_json(const wpi::util::json& j, NetworkStatus& ns);

struct RadioStatus {
    bool valid = false;
    std::string mode;
    std::string channel;
    int64_t teamNumber;
    std::string ssidSuffix;
    NetworkStatus networkStatus24;
    NetworkStatus networkStatus6;
    std::string status;
    std::string version;
};

void to_json(wpi::util::json& j, const RadioStatus& rs);
void from_json(const wpi::util::json& j, RadioStatus& rs);

template <>
struct wpi::util::Struct<NetworkStatus> {
    static constexpr std::string_view GetTypeName() { return "NetworkStatus"; }
    static constexpr size_t GetSize() { return 226; }
    static constexpr std::string_view GetSchema() {
        return "char ssid[32];"
        "char hashedWpaKey[64];"
        "char wpaKeySalt[16];"
        "bool isLinked;"
               "char macAddress[17];"
               "int64 signalDbm;"
               "int64 noiseDbm;"
               "int64 signalNoiseRatio;"
               "double rxRateMbps;"
               "int64 rxPackets;"
               "int64 rxBytes;"
               "double txRateMbps;"
               "int64 txPackets;"
               "int64 txBytes;"
               "double bandwidthUsedMbps;"
               "char connectionQuality[16]";
    }
    static NetworkStatus Unpack(std::span<const uint8_t> data);
    static void Pack(std::span<uint8_t> data, const NetworkStatus& value);
};
static_assert(wpi::util::StructSerializable<NetworkStatus>);

template <>
struct wpi::util::Struct<RadioStatus> {
    static constexpr std::string_view GetTypeName() { return "RadioStatus"; }
    static constexpr size_t GetSize() { return 549; }
    static constexpr std::string_view GetSchema() {
        return "char mode[20];"
               "char channel[5];"
               "int64 teamNumber;"
               "char ssidSuffix[16];"
               "NetworkStatus networkStatus24;"
               "NetworkStatus networkStatus6;"
               "char status[16];"
               "char version[32]";
    }
    static RadioStatus Unpack(std::span<const uint8_t> data);
    static void Pack(std::span<uint8_t> data, const RadioStatus& value);
    static void ForEachNested(
        std::invocable<std::string_view, std::string_view> auto fn) {
        wpi::util::ForEachStructSchema<NetworkStatus>(fn);
    }
};
static_assert(wpi::util::StructSerializable<RadioStatus>);
static_assert(wpi::util::HasNestedStruct<RadioStatus>);


class RadioDataPublisher {
   public:
    RadioDataPublisher(wpi::nt::NetworkTableInstance ntInstance =
                           wpi::nt::NetworkTableInstance::GetDefault(),
                       std::string_view topicName = "/radio/data");

    void PublishJson(std::string_view jsonData);

   private:
    wpi::nt::NetworkTableInstance m_ntInstance;
    wpi::nt::StructPublisher<RadioStatus> m_publisher;
};