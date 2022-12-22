/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "class/hid/hid.h"
#include "class/usbtmc/usbtmc.h"
#include "class/usbtmc/usbtmc_device.h"

#include "pico/unique_id.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

#if defined(CFG_TUD_HID)
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
  (void) itf;
  return desc_hid_report;
}

#define EPNUM_HID   0x03

#endif /* CFG_TUD_HID */

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#if defined(CFG_TUD_USBTMC)

#  define TUD_USBTMC_DESC_MAIN(_itfnum,_bNumEndpoints) \
     TUD_USBTMC_IF_DESCRIPTOR(_itfnum, _bNumEndpoints,  /*_stridx = */ 4u, TUD_USBTMC_PROTOCOL_USB488), \
     TUD_USBTMC_BULK_DESCRIPTORS(/* OUT = */0x01, /* IN = */ 0x81, /* packet size = */USBTMCD_MAX_PACKET_SIZE)

#if CFG_TUD_USBTMC_ENABLE_INT_EP
// USBTMC Interrupt xfer always has length of 2, but we use epMaxSize=8 for
//  compatibility with mcus that only allow 8, 16, 32 or 64 for FS endpoints
#  define TUD_USBTMC_DESC(_itfnum) \
     TUD_USBTMC_DESC_MAIN(_itfnum, /* _epCount = */ 3), \
     TUD_USBTMC_INT_DESCRIPTOR(/* INT ep # */ 0x82, /* epMaxSize = */ 8, /* bInterval = */16u )
#  define TUD_USBTMC_DESC_LEN (TUD_USBTMC_IF_DESCRIPTOR_LEN + TUD_USBTMC_BULK_DESCRIPTORS_LEN + TUD_USBTMC_INT_DESCRIPTOR_LEN)

#else

#  define TUD_USBTMC_DESC(_itfnum) \
     TUD_USBTMC_DESC_MAIN(_itfnum, /* _epCount = */ 2u)
#  define TUD_USBTMC_DESC_LEN (TUD_USBTMC_IF_DESCRIPTOR_LEN + TUD_USBTMC_BULK_DESCRIPTORS_LEN)

#endif /* CFG_TUD_USBTMC_ENABLE_INT_EP */

#else
#  define USBTMC_DESC_LEN (0)
#endif /* CFG_TUD_USBTMC */

enum
{
  ITF_NUM_USBTMC,
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};


#if defined(CFG_TUD_HID) 
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_USBTMC_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#else
#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_USBTMC_DESC_LEN)
#endif /* CFG_TUD_HID */

uint8_t const desc_configuration[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
  TUD_USBTMC_DESC(ITF_NUM_USBTMC),
#if defined(CFG_TUD_HID) 
  // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
  TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 2, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, 0x80 | EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10)
#endif /* CFG_TUD_HID */
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations
  return desc_configuration;
}

static char usbd_serial_str[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "TinyUSB",                     // 1: Manufacturer
  "TinyUSB Device",              // 2: Product
  usbd_serial_str,                      // 3: Serials, should use chip ID
  "TinyUSB USBTMC",              // 4: USBTMC
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  size_t chr_count;

  // Assign the SN using the unique flash id
  if (!usbd_serial_str[0]) {
    pico_get_unique_board_id_string(usbd_serial_str, sizeof(usbd_serial_str));
  }

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }
  else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if ( chr_count > 31 ) {
      chr_count = 31;
    }

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((((uint16_t)TUSB_DESC_STRING) << 8 ) | (2u*chr_count + 2u));

  return _desc_str;
}
