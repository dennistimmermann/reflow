// USB descriptors: composite device with one CDC ACM port and one DFU
// runtime interface. CDC = serial console; DFU runtime lets `dfu-util -e`
// (or any -D flash) switch us into the STM32G0 ROM bootloader without a
// 1200-baud baud-rate-as-signal hack.

#include <string.h>
#include "tusb.h"
#include "class/dfu/dfu_rt_device.h"

// Vendor/product IDs: keep ST's CDC pair so macOS/Windows don't bind a
// fresh driver cache entry on the first boot after migration.
#define USB_VID 0x0483
#define USB_PID 0x5740
#define USB_BCD 0x0210  // 2.1 so a BOS descriptor (WebUSB) can be added later

//--------------------------------------------------------------------+
// Device descriptor
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = USB_BCD,

    // IAD advertises the composite (CDC wants it).
    .bDeviceClass    = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor        = USB_VID,
    .idProduct       = USB_PID,
    .bcdDevice       = 0x0100,

    .iManufacturer   = 0x01,
    .iProduct        = 0x02,
    .iSerialNumber   = 0x03,

    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// Configuration descriptor
//--------------------------------------------------------------------+
enum {
    ITF_NUM_CDC = 0,       // CDC control
    ITF_NUM_CDC_DATA,      // CDC data
    ITF_NUM_DFU_RT,        // DFU runtime
    ITF_NUM_TOTAL,
};

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_DFU_RT_DESC_LEN)

// DFU runtime attribute bits (bmAttributes):
//   0x08  willDetach            — device performs the detach without host-side reconnect logic
//   0x04  manifestationTolerant — device tolerates further requests during manifestation
//   0x01  canDownload           — host → device
//   0x02  canUpload             — not supported
#define DFU_RT_ATTRS (0x08 | 0x04 | 0x01)

uint8_t const desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attributes, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // CDC: control itf, string index 4, notif EP + size, data EP out/in + size
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

    // DFU runtime: itf, string index 5, attributes, detach timeout (ms), max transfer size
    TUD_DFU_RT_DESCRIPTOR(ITF_NUM_DFU_RT, 5, DFU_RT_ATTRS, 1000, 1024),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String descriptors
//--------------------------------------------------------------------+
enum {
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_CDC,
    STRID_DFU,
};

static char const *const string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // 0: English (0x0409)
    "reflow-oven",               // 1: Manufacturer
    "Reflow Oven",               // 2: Product
    NULL,                        // 3: Serial (filled from MCU UID at runtime)
    "Reflow CDC",                // 4: CDC
    "Reflow DFU",                // 5: DFU runtime
};

static uint16_t _desc_str[32 + 1];

// Build a 12-hex-char serial from the STM32 96-bit UID (three 32-bit words at
// UID_BASE). Truncating to 12 hex = 48 bits is plenty to disambiguate
// individual boards; keeps the string short for host log readability.
static size_t fill_serial(uint16_t *out, size_t max_chars) {
    static const uint32_t UID_BASE = 0x1FFF7590u;  // STM32G0 unique device ID
    const uint32_t w0 = *(volatile uint32_t *)(UID_BASE + 0);
    const uint32_t w1 = *(volatile uint32_t *)(UID_BASE + 4);

    static const char hex[] = "0123456789ABCDEF";
    char tmp[12];
    for (int i = 0; i < 8; ++i) tmp[i]     = hex[(w0 >> ((7 - i) * 4)) & 0xF];
    for (int i = 0; i < 4; ++i) tmp[8 + i] = hex[(w1 >> ((3 - i) * 4)) & 0xF];

    size_t n = 12;
    if (n > max_chars) n = max_chars;
    for (size_t i = 0; i < n; ++i) out[i] = (uint16_t)tmp[i];
    return n;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    size_t chr_count = 0;
    size_t const max_chars = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;

    if (index == STRID_LANGID) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else if (index == STRID_SERIAL) {
        chr_count = fill_serial(_desc_str + 1, max_chars);
    } else if (index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
        const char *s = string_desc_arr[index];
        chr_count = strlen(s);
        if (chr_count > max_chars) chr_count = max_chars;
        for (size_t i = 0; i < chr_count; ++i) _desc_str[1 + i] = (uint16_t)s[i];
    } else {
        return NULL;
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}
