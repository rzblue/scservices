#include "RadioDataPublisher.hpp"

#include <cstdio>
#include <wpi/util/json.hpp>

// JSON serialization/deserialization for NetworkStatus
void to_json(wpi::util::json& j, const NetworkStatus& ns) {
    j = wpi::util::json{{"ssid", ns.ssid},
                        {"hashedWpaKey", ns.hashedWpaKey},
                        {"wpaKeySalt", ns.wpaKeySalt},
                        {"isLinked", ns.isLinked},
                        {"macAddress", ns.macAddress},
                        {"signalDbm", ns.signalDbm},
                        {"noiseDbm", ns.noiseDbm},
                        {"signalNoiseRatio", ns.signalNoiseRatio},
                        {"rxRateMbps", ns.rxRateMbps},
                        {"rxPackets", ns.rxPackets},
                        {"rxBytes", ns.rxBytes},
                        {"txRateMbps", ns.txRateMbps},
                        {"txPackets", ns.txPackets},
                        {"txBytes", ns.txBytes},
                        {"bandwidthUsedMbps", ns.bandwidthUsedMbps},
                        {"connectionQuality", ns.connectionQuality}};
}

void from_json(const wpi::util::json& j, NetworkStatus& ns) {
    ns.ssid = j.value("ssid", "");
    ns.hashedWpaKey = j.value("hashedWpaKey", "");
    ns.wpaKeySalt = j.value("wpaKeySalt", "");
    ns.isLinked = j.value("isLinked", false);
    ns.macAddress = j.value("macAddress", "");
    ns.signalDbm = j.value("signalDbm", int64_t{0});
    ns.noiseDbm = j.value("noiseDbm", int64_t{0});
    ns.signalNoiseRatio = j.value("signalNoiseRatio", int64_t{0});
    ns.rxRateMbps = j.value("rxRateMbps", 0.0);
    ns.rxPackets = j.value("rxPackets", int64_t{0});
    ns.rxBytes = j.value("rxBytes", int64_t{0});
    ns.txRateMbps = j.value("txRateMbps", 0.0);
    ns.txPackets = j.value("txPackets", int64_t{0});
    ns.txBytes = j.value("txBytes", int64_t{0});
    ns.bandwidthUsedMbps = j.value("bandwidthUsedMbps", 0.0);
    ns.connectionQuality = j.value("connectionQuality", "");
}

// JSON serialization/deserialization for RadioStatus
void to_json(wpi::util::json& j, const RadioStatus& rs) {
    j = wpi::util::json{{"mode", rs.mode},
                        {"channel", rs.channel},
                        {"teamNumber", rs.teamNumber},
                        {"ssidSuffix", rs.ssidSuffix},
                        {"networkStatus24", rs.networkStatus24},
                        {"networkStatus6", rs.networkStatus6},
                        {"status", rs.status},
                        {"version", rs.version},
                        {"valid", rs.valid}};
}

void from_json(const wpi::util::json& j, RadioStatus& rs) {
    rs.mode = j.value("mode", "");
    rs.channel = j.value("channel", "");
    rs.teamNumber = j.value("teamNumber", int64_t{0});
    rs.ssidSuffix = j.value("ssidSuffix", "");
    rs.networkStatus24 = j.value("networkStatus24", NetworkStatus{});
    rs.networkStatus6 = j.value("networkStatus6", NetworkStatus{});
    rs.status = j.value("status", "");
    rs.version = j.value("version", "");
    rs.valid = true;
}

RadioDataPublisher::RadioDataPublisher(wpi::nt::NetworkTableInstance ntInst,
                                       std::string_view topicName)
    : m_ntInstance(ntInst),
      m_publisher(
          m_ntInstance.GetStructTopic<RadioStatus>(topicName).Publish()) {}

void RadioDataPublisher::PublishJson(std::string_view jsonData) {
    printf("Publishing RadioStatus JSON data\n");
    try {
        wpi::util::json parsed = wpi::util::json::parse(jsonData);

        if (!parsed.is_object()) {
            m_publisher.Set({});
            return;
        }

        RadioStatus status = parsed.get<RadioStatus>();
        m_publisher.Set(status);
        printf("published?");
    } catch (const wpi::util::json::exception& e) {
        printf("Failed to parse RadioStatus JSON: %s\n", e.what());
        m_publisher.Set({});
    }
    std::fflush(stdout);
}

namespace {
// NetworkStatus field offsets and sizes
constexpr size_t kSsidOff = 0, kSsidSize = 32;
constexpr size_t kHashedWpaKeyOff = kSsidOff + kSsidSize,
                 kHashedWpaKeySize = 64;
constexpr size_t kWpaKeySaltOff = kHashedWpaKeyOff + kHashedWpaKeySize,
                 kWpaKeySaltSize = 16;
constexpr size_t kIsLinkedOff = kWpaKeySaltOff + kWpaKeySaltSize;
constexpr size_t kMacAddressOff =
                     kIsLinkedOff + wpi::util::Struct<bool>::GetSize(),
                 kMacAddressSize = 17;
constexpr size_t kSignalDbmOff = kMacAddressOff + kMacAddressSize;
constexpr size_t kNoiseDbmOff =
    kSignalDbmOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kSignalNoiseRatioOff =
    kNoiseDbmOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kRxRateMbpsOff =
    kSignalNoiseRatioOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kRxPacketsOff =
    kRxRateMbpsOff + wpi::util::Struct<double>::GetSize();
constexpr size_t kRxBytesOff =
    kRxPacketsOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kTxRateMbpsOff =
    kRxBytesOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kTxPacketsOff =
    kTxRateMbpsOff + wpi::util::Struct<double>::GetSize();
constexpr size_t kTxBytesOff =
    kTxPacketsOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kBandwidthUsedMbpsOff =
    kTxBytesOff + wpi::util::Struct<int64_t>::GetSize();
constexpr size_t kConnectionQualityOff = kBandwidthUsedMbpsOff +
                                         wpi::util::Struct<double>::GetSize(),
                 kConnectionQualitySize = 16;

// RadioStatus field offsets and sizes
constexpr size_t kModeOff = 0, kModeSize = 20;
constexpr size_t kChannelOff = kModeOff + kModeSize, kChannelSize = 5;
constexpr size_t kTeamNumberOff = kChannelOff + kChannelSize;
constexpr size_t kSsidSuffixOff =
                     kTeamNumberOff + wpi::util::Struct<int64_t>::GetSize(),
                 kSsidSuffixSize = 16;
constexpr size_t kNetworkStatus24Off = kSsidSuffixOff + kSsidSuffixSize;
constexpr size_t kNetworkStatus6Off =
    kNetworkStatus24Off + wpi::util::Struct<NetworkStatus>::GetSize();
constexpr size_t kStatusOff = kNetworkStatus6Off +
                              wpi::util::Struct<NetworkStatus>::GetSize(),
                 kStatusSize = 16;
constexpr size_t kVersionOff = kStatusOff + kStatusSize, kVersionSize = 32;

// Helper to unpack a null-terminated string from a fixed-size char array
inline std::string UnpackString(const uint8_t* ptr, size_t maxSize) {
    size_t len =
        std::min(maxSize, std::strlen(reinterpret_cast<const char*>(ptr)));
    return std::string(reinterpret_cast<const char*>(ptr), len);
}

// Helper to pack a string into a fixed-size char array with zero-fill
inline void PackString(uint8_t* ptr, size_t size, const std::string& value) {
    std::memset(ptr, 0, size);
    std::memcpy(ptr, value.c_str(), std::min(size, value.size()));
}
}  // namespace

NetworkStatus wpi::util::Struct<NetworkStatus>::Unpack(
    std::span<const uint8_t> data) {
    NetworkStatus result;
    result.ssid = UnpackString(data.data() + kSsidOff, kSsidSize);
    result.hashedWpaKey =
        UnpackString(data.data() + kHashedWpaKeyOff, kHashedWpaKeySize);
    result.wpaKeySalt =
        UnpackString(data.data() + kWpaKeySaltOff, kWpaKeySaltSize);
    result.isLinked = wpi::util::UnpackStruct<bool, kIsLinkedOff>(data);
    result.macAddress =
        UnpackString(data.data() + kMacAddressOff, kMacAddressSize);
    result.signalDbm = wpi::util::UnpackStruct<int64_t, kSignalDbmOff>(data);
    result.noiseDbm = wpi::util::UnpackStruct<int64_t, kNoiseDbmOff>(data);
    result.signalNoiseRatio =
        wpi::util::UnpackStruct<int64_t, kSignalNoiseRatioOff>(data);
    result.rxRateMbps = wpi::util::UnpackStruct<double, kRxRateMbpsOff>(data);
    result.rxPackets = wpi::util::UnpackStruct<int64_t, kRxPacketsOff>(data);
    result.rxBytes = wpi::util::UnpackStruct<int64_t, kRxBytesOff>(data);
    result.txRateMbps = wpi::util::UnpackStruct<double, kTxRateMbpsOff>(data);
    result.txPackets = wpi::util::UnpackStruct<int64_t, kTxPacketsOff>(data);
    result.txBytes = wpi::util::UnpackStruct<int64_t, kTxBytesOff>(data);
    result.bandwidthUsedMbps =
        wpi::util::UnpackStruct<double, kBandwidthUsedMbpsOff>(data);
    result.connectionQuality = UnpackString(data.data() + kConnectionQualityOff,
                                            kConnectionQualitySize);
    return result;
}

void wpi::util::Struct<NetworkStatus>::Pack(std::span<uint8_t> data,
                                            const NetworkStatus& value) {
    PackString(data.data() + kSsidOff, kSsidSize, value.ssid);
    PackString(data.data() + kHashedWpaKeyOff, kHashedWpaKeySize,
               value.hashedWpaKey);
    PackString(data.data() + kWpaKeySaltOff, kWpaKeySaltSize, value.wpaKeySalt);
    wpi::util::PackStruct<kIsLinkedOff>(data, value.isLinked);
    PackString(data.data() + kMacAddressOff, kMacAddressSize, value.macAddress);
    wpi::util::PackStruct<kSignalDbmOff>(data, value.signalDbm);
    wpi::util::PackStruct<kNoiseDbmOff>(data, value.noiseDbm);
    wpi::util::PackStruct<kSignalNoiseRatioOff>(data, value.signalNoiseRatio);
    wpi::util::PackStruct<kRxRateMbpsOff>(data, value.rxRateMbps);
    wpi::util::PackStruct<kRxPacketsOff>(data, value.rxPackets);
    wpi::util::PackStruct<kRxBytesOff>(data, value.rxBytes);
    wpi::util::PackStruct<kTxRateMbpsOff>(data, value.txRateMbps);
    wpi::util::PackStruct<kTxPacketsOff>(data, value.txPackets);
    wpi::util::PackStruct<kTxBytesOff>(data, value.txBytes);
    wpi::util::PackStruct<kBandwidthUsedMbpsOff>(data, value.bandwidthUsedMbps);
    PackString(data.data() + kConnectionQualityOff, kConnectionQualitySize,
               value.connectionQuality);
}

RadioStatus wpi::util::Struct<RadioStatus>::Unpack(
    std::span<const uint8_t> data) {
    RadioStatus result;
    result.mode = UnpackString(data.data() + kModeOff, kModeSize);
    result.channel = UnpackString(data.data() + kChannelOff, kChannelSize);
    result.teamNumber = wpi::util::UnpackStruct<int64_t, kTeamNumberOff>(data);
    result.ssidSuffix =
        UnpackString(data.data() + kSsidSuffixOff, kSsidSuffixSize);
    result.networkStatus24 =
        wpi::util::UnpackStruct<NetworkStatus, kNetworkStatus24Off>(data);
    result.networkStatus6 =
        wpi::util::UnpackStruct<NetworkStatus, kNetworkStatus6Off>(data);
    result.status = UnpackString(data.data() + kStatusOff, kStatusSize);
    result.version = UnpackString(data.data() + kVersionOff, kVersionSize);
    result.valid = true;
    return result;
}

void wpi::util::Struct<RadioStatus>::Pack(std::span<uint8_t> data,
                                          const RadioStatus& value) {
    PackString(data.data() + kModeOff, kModeSize, value.mode);
    PackString(data.data() + kChannelOff, kChannelSize, value.channel);
    wpi::util::PackStruct<kTeamNumberOff>(data, value.teamNumber);
    PackString(data.data() + kSsidSuffixOff, kSsidSuffixSize, value.ssidSuffix);
    wpi::util::PackStruct<kNetworkStatus24Off>(data, value.networkStatus24);
    wpi::util::PackStruct<kNetworkStatus6Off>(data, value.networkStatus6);
    PackString(data.data() + kStatusOff, kStatusSize, value.status);
    PackString(data.data() + kVersionOff, kVersionSize, value.version);
}
