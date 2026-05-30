// BLE HID implementation — see ble_hid.h.
//
// Uses esp-nimble-cpp's NimBLEHIDDevice wrapper. The wrapper publishes
// HID Service (0x1812), Device Information Service (0x180A), and Battery
// Service (0x180F); we just hand it our report map and HID info.
//
// Pairing: just-works (no PIN), bonded, secure connections. After first
// successful pair the host is remembered and can reconnect silently.

#include "ble_hid.h"

#include "hid_report_map.h"

#include "esp_log.h"

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEHIDDevice.h>
#include <NimBLEAdvertising.h>

#include <M5Unified.h>

#include <atomic>
#include <cstring>

namespace pocket {
namespace ble {

namespace {

constexpr const char* TAG = "BLE";

std::atomic<ConnState> s_state{ConnState::kInactive};

NimBLEServer*         s_server      = nullptr;
NimBLEHIDDevice*      s_hid         = nullptr;
NimBLECharacteristic* s_kbd_input   = nullptr;
NimBLECharacteristic* s_consumer_in = nullptr;
bool                  s_inited      = false;

class ServerCb : public NimBLEServerCallbacks {
public:
    void onConnect(NimBLEServer* /*server*/, NimBLEConnInfo& info) override
    {
        ESP_LOGI(TAG, "connected handle=%u", info.getConnHandle());
        s_state.store(ConnState::kConnected, std::memory_order_relaxed);
    }

    void onDisconnect(NimBLEServer* /*server*/, NimBLEConnInfo& /*info*/,
                      int reason) override
    {
        ESP_LOGI(TAG, "disconnected reason=0x%02x; re-advertising", reason);
        s_state.store(ConnState::kAdvertising, std::memory_order_relaxed);
        // Resume advertising so the central (or any new pair candidate)
        // can find us again without us having to deinit/reinit the stack.
        NimBLEDevice::getAdvertising()->start();
    }
};

ServerCb s_server_cb;

}  // namespace

bool init(const char* device_name)
{
    if (s_inited) return true;

    NimBLEDevice::init(device_name);
    NimBLEDevice::setPower(3);  // +3 dBm — modest, the stick lives in a pocket
    // bonding=true, mitm=false (no PIN — single-button device can't type one),
    // sc=true (Secure Connections / LESC).
    NimBLEDevice::setSecurityAuth(true, false, true);

    s_server = NimBLEDevice::createServer();
    // deleteCallbacks=false: s_server_cb is a static singleton, not heap.
    // The wrapper's default is true → ~NimBLEServer() would `delete` our
    // static and crash inside heap_caps_free. Confirmed on hardware.
    s_server->setCallbacks(&s_server_cb, /*deleteCallbacks=*/false);

    s_hid = new NimBLEHIDDevice(s_server);
    s_kbd_input   = s_hid->getInputReport(kKeyboardReportId);
    s_consumer_in = s_hid->getInputReport(kConsumerReportId);

    s_hid->setManufacturer("Pocket Oracle");
    // Vendor source: USB-IF (0x02). VID/PID are placeholders (no IF allocation).
    s_hid->setPnp(0x02, 0xE502, 0xA111, 0x0100);
    // Country=0 (not localized), flags=0x01 (remote wake).
    s_hid->setHidInfo(0x00, 0x01);
    s_hid->setReportMap(const_cast<uint8_t*>(kReportMap), kReportMapSize);

    const int bat = M5.Power.getBatteryLevel();
    s_hid->setBatteryLevel(bat > 0 ? static_cast<uint8_t>(bat) : 100);

    NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
    adv->setAppearance(HID_KEYBOARD);
    adv->addServiceUUID(s_hid->getHidService()->getUUID());
    adv->enableScanResponse(true);
    if (!adv->start()) {
        ESP_LOGE(TAG, "advertising start failed");
        return false;
    }

    s_state.store(ConnState::kAdvertising, std::memory_order_relaxed);
    s_inited = true;
    ESP_LOGI(TAG, "advertising as '%s'", device_name);
    return true;
}

void deinit()
{
    if (!s_inited) return;
    NimBLEDevice::deinit(true);
    s_server      = nullptr;
    s_hid         = nullptr;
    s_kbd_input   = nullptr;
    s_consumer_in = nullptr;
    s_inited      = false;
    s_state.store(ConnState::kInactive, std::memory_order_relaxed);
    ESP_LOGI(TAG, "deinitialized");
}

ConnState state()
{
    return s_state.load(std::memory_order_relaxed);
}

bool is_connected()
{
    return state() == ConnState::kConnected;
}

void send_keyboard(uint8_t keycode, uint8_t modifiers)
{
    if (!is_connected() || !s_kbd_input) return;

    uint8_t report[kKeyboardReportSize] = {0};
    report[0] = modifiers;
    report[2] = keycode;
    s_kbd_input->notify(report, kKeyboardReportSize);

    // Release: zero report.
    uint8_t release[kKeyboardReportSize] = {0};
    s_kbd_input->notify(release, kKeyboardReportSize);
}

void send_consumer(uint16_t usage)
{
    if (!is_connected() || !s_consumer_in) return;

    uint8_t report[kConsumerReportSize] = {
        static_cast<uint8_t>(usage & 0xFF),
        static_cast<uint8_t>((usage >> 8) & 0xFF),
    };
    s_consumer_in->notify(report, kConsumerReportSize);

    uint8_t release[kConsumerReportSize] = {0, 0};
    s_consumer_in->notify(release, kConsumerReportSize);
}

}  // namespace ble
}  // namespace pocket
