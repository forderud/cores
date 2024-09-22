/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2019 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#if F_CPU >= 20000000

#define USB_DESC_LIST_DEFINE
#include "usb_desc.h"
#ifdef NUM_ENDPOINTS
#include "usb_names.h"
#include "imxrt.h"
#include "avr_functions.h"
#include "avr/pgmspace.h"

// At very slow CPU speeds, the OCRAM just isn't fast enough for
// USB to work reliably.  But the precious/limited DTCM is.  So
// as an ugly workaround, undefine DMAMEM so all buffers which
// would normally be allocated in OCRAM are placed in DTCM.
#if defined(F_CPU) && F_CPU < 30000000
#undef DMAMEM
#endif

// USB Descriptors are binary data which the USB host reads to
// automatically detect a USB device's capabilities.  The format
// and meaning of every field is documented in numerous USB
// standards.  When working with USB descriptors, despite the
// complexity of the standards and poor writing quality in many
// of those documents, remember descriptors are nothing more
// than constant binary data that tells the USB host what the
// device can do.  Computers will load drivers based on this data.
// Those drivers then communicate on the endpoints specified by
// the descriptors.

// To configure a new combination of interfaces or make minor
// changes to existing configuration (eg, change the name or ID
// numbers), usually you would edit "usb_desc.h".  This file
// is meant to be configured by the header, so generally it is
// only edited to add completely new USB interfaces or features.



// **************************************************************
//   USB Device
// **************************************************************

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)

#ifdef CDC_IAD_DESCRIPTOR
#ifndef DEVICE_CLASS
#define DEVICE_CLASS 0xEF
#endif
#ifndef DEVICE_SUBCLASS
#define DEVICE_SUBCLASS 0x02
#endif
#ifndef DEVICE_PROTOCOL
#define DEVICE_PROTOCOL 0x01
#endif
#endif


// USB Device Descriptor.  The USB host reads this first, to learn
// what type of device is connected.
static uint8_t device_descriptor[] = {
        18,                                     // bLength
        1,                                      // bDescriptorType
        0x00, 0x02,                             // bcdUSB
#ifdef DEVICE_CLASS
        DEVICE_CLASS,                           // bDeviceClass
#else
	0,
#endif
#ifdef DEVICE_SUBCLASS
        DEVICE_SUBCLASS,                        // bDeviceSubClass
#else
	0,
#endif
#ifdef DEVICE_PROTOCOL
        DEVICE_PROTOCOL,                        // bDeviceProtocol
#else
	0,
#endif
        EP0_SIZE,                               // bMaxPacketSize0
        LSB(VENDOR_ID), MSB(VENDOR_ID),         // idVendor
        LSB(PRODUCT_ID), MSB(PRODUCT_ID),       // idProduct
#ifdef BCD_DEVICE
	LSB(BCD_DEVICE), MSB(BCD_DEVICE),       // bcdDevice
#else
  // For USB types that don't explicitly define BCD_DEVICE,
  // use the minor version number to help teensy_ports
  // identify which Teensy model is used.
  #if defined(__IMXRT1062__) && defined(ARDUINO_TEENSY40)
        0x79, 0x02, // Teensy 4.0
  #elif defined(__IMXRT1062__) && defined(ARDUINO_TEENSY41)
        0x80, 0x02, // Teensy 4.1
  #elif defined(__IMXRT1062__) && defined(ARDUINO_TEENSY_MICROMOD)
        0x81, 0x02, // Teensy MicroMod
  #else
        0x00, 0x02,
  #endif
#endif
        1,                                      // iManufacturer
        2,                                      // iProduct
        3,                                      // iSerialNumber
        1                                       // bNumConfigurations
};

PROGMEM static const uint8_t qualifier_descriptor[] = {	// 9.6.2 Device_Qualifier, page 264
	10,					// bLength
	6,					// bDescriptorType
	0x00, 0x02,				// bcdUSB
#ifdef DEVICE_CLASS
        DEVICE_CLASS,                           // bDeviceClass
#else
	0,
#endif
#ifdef DEVICE_SUBCLASS
        DEVICE_SUBCLASS,                        // bDeviceSubClass
#else
	0,
#endif
#ifdef DEVICE_PROTOCOL
        DEVICE_PROTOCOL,                        // bDeviceProtocol
#else
	0,
#endif
        EP0_SIZE,                               // bMaxPacketSize0
        1,					// bNumConfigurations
        0                                       // bReserved
};

// These descriptors must NOT be "const", because the USB DMA
// has trouble accessing flash memory with enough bandwidth
// while the processor is executing from flash.



// **************************************************************
//   HID Report Descriptors
// **************************************************************

// Each HID interface needs a special report descriptor that tells
// the meaning and format of the data.

#ifdef EXPERIMENTAL_INTERFACE
// DOC: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-rdpeusb/c2f351f9-84d2-4a1b-9fe3-a6ca195f84d0
static uint8_t microsoft_os_string_desc[] = {
	18, 3,
	'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
	0xF8, 0  // GET_MS_DESCRIPTOR will use bRequest=0xF8
};
static uint8_t microsoft_os_compatible_id_desc[] = {
	40, 0, 0, 0, // total length, 16 header + 24 function * 1
	0, 1, 4, 0,  // version 1.00, wIndex=4 (Compat ID)
	1, 0, 0, 0, 0, 0, 0, 0, // 1 function
	EXPERIMENTAL_INTERFACE, 1,
	'W','I','N','U','S','B',0,0, // compatibleID
	0,0,0,0,0,0,0,0,             // subCompatibleID
	0,0,0,0,0,0
};
#endif

// **************************************************************
//   USB Descriptor Sizes
// **************************************************************

// pre-compute the size and position of everything in the config descriptor
//
#define CONFIG_HEADER_DESCRIPTOR_SIZE	9

#define CDC_IAD_DESCRIPTOR_POS		CONFIG_HEADER_DESCRIPTOR_SIZE
#ifdef  CDC_IAD_DESCRIPTOR
#define CDC_IAD_DESCRIPTOR_SIZE		8
#else
#define CDC_IAD_DESCRIPTOR_SIZE		0
#endif

#define CDC_DATA_INTERFACE_DESC_POS	CDC_IAD_DESCRIPTOR_POS+CDC_IAD_DESCRIPTOR_SIZE
#ifdef  CDC_DATA_INTERFACE
#define CDC_DATA_INTERFACE_DESC_SIZE	9+5+5+4+5+7+9+7+7
#else
#define CDC_DATA_INTERFACE_DESC_SIZE	0
#endif

#define CDC2_DATA_INTERFACE_DESC_POS    CDC_DATA_INTERFACE_DESC_POS+CDC_DATA_INTERFACE_DESC_SIZE
#ifdef  CDC2_DATA_INTERFACE
#define CDC2_DATA_INTERFACE_DESC_SIZE   8 + 9+5+5+4+5+7+9+7+7
#else
#define CDC2_DATA_INTERFACE_DESC_SIZE   0
#endif

#define CDC3_DATA_INTERFACE_DESC_POS    CDC2_DATA_INTERFACE_DESC_POS+CDC2_DATA_INTERFACE_DESC_SIZE
#ifdef  CDC3_DATA_INTERFACE
#define CDC3_DATA_INTERFACE_DESC_SIZE   8 + 9+5+5+4+5+7+9+7+7
#else
#define CDC3_DATA_INTERFACE_DESC_SIZE   0
#endif

#define MIDI_INTERFACE_DESC_POS		CDC3_DATA_INTERFACE_DESC_POS+CDC3_DATA_INTERFACE_DESC_SIZE
#ifdef  MIDI_INTERFACE
  #if !defined(MIDI_NUM_CABLES) || MIDI_NUM_CABLES < 1 || MIDI_NUM_CABLES > 16
  #error "MIDI_NUM_CABLES must be defined between 1 to 16"
  #endif
#define MIDI_INTERFACE_DESC_SIZE	9+7+((6+6+9+9)*MIDI_NUM_CABLES)+(9+4+MIDI_NUM_CABLES)*2
#else
#define MIDI_INTERFACE_DESC_SIZE	0
#endif

#define KEYBOARD_INTERFACE_DESC_POS	MIDI_INTERFACE_DESC_POS+MIDI_INTERFACE_DESC_SIZE
#define KEYBOARD_INTERFACE_DESC_SIZE	0

#define MOUSE_INTERFACE_DESC_POS	KEYBOARD_INTERFACE_DESC_POS+KEYBOARD_INTERFACE_DESC_SIZE
#define MOUSE_INTERFACE_DESC_SIZE	0

#define RAWHID_INTERFACE_DESC_POS	MOUSE_INTERFACE_DESC_POS+MOUSE_INTERFACE_DESC_SIZE
#define RAWHID_INTERFACE_DESC_SIZE	0

#define FLIGHTSIM_INTERFACE_DESC_POS	RAWHID_INTERFACE_DESC_POS+RAWHID_INTERFACE_DESC_SIZE
#define FLIGHTSIM_INTERFACE_DESC_SIZE	0

#define SEREMU_INTERFACE_DESC_POS	FLIGHTSIM_INTERFACE_DESC_POS+FLIGHTSIM_INTERFACE_DESC_SIZE
#define SEREMU_INTERFACE_DESC_SIZE	0

#define JOYSTICK_INTERFACE_DESC_POS	SEREMU_INTERFACE_DESC_POS+SEREMU_INTERFACE_DESC_SIZE
#define JOYSTICK_INTERFACE_DESC_SIZE	0

#define MTP_INTERFACE_DESC_POS		JOYSTICK_INTERFACE_DESC_POS+JOYSTICK_INTERFACE_DESC_SIZE
#ifdef  MTP_INTERFACE
#define MTP_INTERFACE_DESC_SIZE		9+7+7+7
#else
#define MTP_INTERFACE_DESC_SIZE	0
#endif

#define KEYMEDIA_INTERFACE_DESC_POS	MTP_INTERFACE_DESC_POS+MTP_INTERFACE_DESC_SIZE
#define KEYMEDIA_INTERFACE_DESC_SIZE	0

#define AUDIO_INTERFACE_DESC_POS	KEYMEDIA_INTERFACE_DESC_POS+KEYMEDIA_INTERFACE_DESC_SIZE
#ifdef  AUDIO_INTERFACE
#define AUDIO_INTERFACE_DESC_SIZE	8 + 9+10+12+9+12+10+9 + 9+9+7+11+9+7 + 9+9+7+11+9+7+9
#else
#define AUDIO_INTERFACE_DESC_SIZE	0
#endif

#define MULTITOUCH_INTERFACE_DESC_POS	AUDIO_INTERFACE_DESC_POS+AUDIO_INTERFACE_DESC_SIZE
#define MULTITOUCH_INTERFACE_DESC_SIZE	0

#define EXPERIMENTAL_INTERFACE_DESC_POS	MULTITOUCH_INTERFACE_DESC_POS+MULTITOUCH_INTERFACE_DESC_SIZE
#ifdef  EXPERIMENTAL_INTERFACE
#define EXPERIMENTAL_INTERFACE_DESC_SIZE 9+7+7
#define EXPERIMENTAL_HID_DESC_OFFSET	MULTITOUCH_INTERFACE_DESC_POS+9
#else
#define EXPERIMENTAL_INTERFACE_DESC_SIZE 0
#endif

#define CONFIG_DESC_SIZE		EXPERIMENTAL_INTERFACE_DESC_POS+EXPERIMENTAL_INTERFACE_DESC_SIZE



// **************************************************************
//   USB Configuration
// **************************************************************

// USB Configuration Descriptor.  This huge descriptor tells all
// of the devices capabilities.

PROGMEM const uint8_t usb_config_descriptor_480[CONFIG_DESC_SIZE] = {
        // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
        9,                                      // bLength;
        2,                                      // bDescriptorType;
        LSB(CONFIG_DESC_SIZE),                 // wTotalLength
        MSB(CONFIG_DESC_SIZE),
        NUM_INTERFACE,                          // bNumInterfaces
        1,                                      // bConfigurationValue
        0,                                      // iConfiguration
        0xC0,                                   // bmAttributes
        50,                                     // bMaxPower

#ifdef CDC_IAD_DESCRIPTOR
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC_STATUS_INTERFACE,                   // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
#endif

#ifdef CDC_DATA_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_STATUS_INTERFACE,			// bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC_STATUS_INTERFACE,                   // bMasterInterface
        CDC_DATA_INTERFACE,                     // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_ACM_ENDPOINT | 0x80,                // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        LSB(CDC_ACM_SIZE),MSB(CDC_ACM_SIZE),    // wMaxPacketSize
        5,                                      // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_DATA_INTERFACE,                     // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_RX_SIZE_480),MSB(CDC_RX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_TX_SIZE_480),MSB(CDC_TX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
#endif // CDC_DATA_INTERFACE

#ifdef CDC2_DATA_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC2_STATUS_INTERFACE,                  // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC2_STATUS_INTERFACE,                  // bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC2_STATUS_INTERFACE,                  // bMasterInterface
        CDC2_DATA_INTERFACE,                    // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_ACM_ENDPOINT | 0x80,               // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        5,                                      // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC2_DATA_INTERFACE,                    // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_RX_SIZE_480),MSB(CDC_RX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_TX_SIZE_480),MSB(CDC_TX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
#endif // CDC2_DATA_INTERFACE

#ifdef CDC3_DATA_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC3_STATUS_INTERFACE,                  // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC3_STATUS_INTERFACE,                  // bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC3_STATUS_INTERFACE,                  // bMasterInterface
        CDC3_DATA_INTERFACE,                    // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_ACM_ENDPOINT | 0x80,               // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        5,                                      // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC3_DATA_INTERFACE,                    // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_RX_SIZE_480),MSB(CDC_RX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_TX_SIZE_480),MSB(CDC_TX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
#endif // CDC3_DATA_INTERFACE

#ifdef MIDI_INTERFACE
	// configuration for 480 Mbit/sec speed
        // Standard MS Interface Descriptor,
        9,                                      // bLength
        4,                                      // bDescriptorType
        MIDI_INTERFACE,                         // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x01,                                   // bInterfaceClass (0x01 = Audio)
        0x03,                                   // bInterfaceSubClass (0x03 = MIDI)
        0x00,                                   // bInterfaceProtocol (unused for MIDI)
        0,                                      // iInterface
        // MIDI MS Interface Header, USB MIDI 6.1.2.1, page 21, Table 6-2
        7,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x01,                                   // bDescriptorSubtype = MS_HEADER
        0x00, 0x01,                             // bcdMSC = revision 01.00
	LSB(7+(6+6+9+9)*MIDI_NUM_CABLES),       // wTotalLength
	MSB(7+(6+6+9+9)*MIDI_NUM_CABLES),
        // MIDI IN Jack Descriptor, B.4.3, Table B-7 (embedded), page 40
        6,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x02,                                   // bDescriptorSubtype = MIDI_IN_JACK
        0x01,                                   // bJackType = EMBEDDED
        1,                                      // bJackID, ID = 1
        0,                                      // iJack
        // MIDI IN Jack Descriptor, B.4.3, Table B-8 (external), page 40
        6,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x02,                                   // bDescriptorSubtype = MIDI_IN_JACK
        0x02,                                   // bJackType = EXTERNAL
        2,                                      // bJackID, ID = 2
        0,                                      // iJack
        // MIDI OUT Jack Descriptor, B.4.4, Table B-9, page 41
        9,
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x03,                                   // bDescriptorSubtype = MIDI_OUT_JACK
        0x01,                                   // bJackType = EMBEDDED
        3,                                      // bJackID, ID = 3
        1,                                      // bNrInputPins = 1 pin
        2,                                      // BaSourceID(1) = 2
        1,                                      // BaSourcePin(1) = first pin
        0,                                      // iJack
        // MIDI OUT Jack Descriptor, B.4.4, Table B-10, page 41
        9,
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x03,                                   // bDescriptorSubtype = MIDI_OUT_JACK
        0x02,                                   // bJackType = EXTERNAL
        4,                                      // bJackID, ID = 4
        1,                                      // bNrInputPins = 1 pin
        1,                                      // BaSourceID(1) = 1
        1,                                      // BaSourcePin(1) = first pin
        0,                                      // iJack
  #if MIDI_NUM_CABLES >= 2
	#define MIDI_INTERFACE_JACK_PAIR(a, b, c, d) \
		6, 0x24, 0x02, 0x01, (a), 0, \
		6, 0x24, 0x02, 0x02, (b), 0, \
		9, 0x24, 0x03, 0x01, (c), 1, (b), 1, 0, \
		9, 0x24, 0x03, 0x02, (d), 1, (a), 1, 0,
	MIDI_INTERFACE_JACK_PAIR(5, 6, 7, 8)
  #endif
  #if MIDI_NUM_CABLES >= 3
	MIDI_INTERFACE_JACK_PAIR(9, 10, 11, 12)
  #endif
  #if MIDI_NUM_CABLES >= 4
	MIDI_INTERFACE_JACK_PAIR(13, 14, 15, 16)
  #endif
  #if MIDI_NUM_CABLES >= 5
	MIDI_INTERFACE_JACK_PAIR(17, 18, 19, 20)
  #endif
  #if MIDI_NUM_CABLES >= 6
	MIDI_INTERFACE_JACK_PAIR(21, 22, 23, 24)
  #endif
  #if MIDI_NUM_CABLES >= 7
	MIDI_INTERFACE_JACK_PAIR(25, 26, 27, 28)
  #endif
  #if MIDI_NUM_CABLES >= 8
	MIDI_INTERFACE_JACK_PAIR(29, 30, 31, 32)
  #endif
  #if MIDI_NUM_CABLES >= 9
	MIDI_INTERFACE_JACK_PAIR(33, 34, 35, 36)
  #endif
  #if MIDI_NUM_CABLES >= 10
	MIDI_INTERFACE_JACK_PAIR(37, 38, 39, 40)
  #endif
  #if MIDI_NUM_CABLES >= 11
	MIDI_INTERFACE_JACK_PAIR(41, 42, 43, 44)
  #endif
  #if MIDI_NUM_CABLES >= 12
	MIDI_INTERFACE_JACK_PAIR(45, 46, 47, 48)
  #endif
  #if MIDI_NUM_CABLES >= 13
	MIDI_INTERFACE_JACK_PAIR(49, 50, 51, 52)
  #endif
  #if MIDI_NUM_CABLES >= 14
	MIDI_INTERFACE_JACK_PAIR(53, 54, 55, 56)
  #endif
  #if MIDI_NUM_CABLES >= 15
	MIDI_INTERFACE_JACK_PAIR(57, 58, 59, 60)
  #endif
  #if MIDI_NUM_CABLES >= 16
	MIDI_INTERFACE_JACK_PAIR(61, 62, 63, 64)
  #endif
        // Standard Bulk OUT Endpoint Descriptor, B.5.1, Table B-11, pae 42
        9,                                      // bLength
        5,                                      // bDescriptorType = ENDPOINT
        MIDI_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MIDI_RX_SIZE_480),MSB(MIDI_RX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
        0,                                      // bRefresh
        0,                                      // bSynchAddress
        // Class-specific MS Bulk OUT Endpoint Descriptor, B.5.2, Table B-12, page 42
        4+MIDI_NUM_CABLES,                      // bLength
        0x25,                                   // bDescriptorSubtype = CS_ENDPOINT
        0x01,                                   // bJackType = MS_GENERAL
        MIDI_NUM_CABLES,                        // bNumEmbMIDIJack = number of jacks
        1,                                      // BaAssocJackID(1) = jack ID #1
  #if MIDI_NUM_CABLES >= 2
        5,
  #endif
  #if MIDI_NUM_CABLES >= 3
        9,
  #endif
  #if MIDI_NUM_CABLES >= 4
        13,
  #endif
  #if MIDI_NUM_CABLES >= 5
        17,
  #endif
  #if MIDI_NUM_CABLES >= 6
        21,
  #endif
  #if MIDI_NUM_CABLES >= 7
        25,
  #endif
  #if MIDI_NUM_CABLES >= 8
        29,
  #endif
  #if MIDI_NUM_CABLES >= 9
        33,
  #endif
  #if MIDI_NUM_CABLES >= 10
        37,
  #endif
  #if MIDI_NUM_CABLES >= 11
        41,
  #endif
  #if MIDI_NUM_CABLES >= 12
        45,
  #endif
  #if MIDI_NUM_CABLES >= 13
        49,
  #endif
  #if MIDI_NUM_CABLES >= 14
        53,
  #endif
  #if MIDI_NUM_CABLES >= 15
        57,
  #endif
  #if MIDI_NUM_CABLES >= 16
        61,
  #endif
        // Standard Bulk IN Endpoint Descriptor, B.5.1, Table B-11, pae 42
        9,                                      // bLength
        5,                                      // bDescriptorType = ENDPOINT
        MIDI_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MIDI_TX_SIZE_480),MSB(MIDI_TX_SIZE_480),// wMaxPacketSize
        0,                                      // bInterval
        0,                                      // bRefresh
        0,                                      // bSynchAddress
        // Class-specific MS Bulk IN Endpoint Descriptor, B.5.2, Table B-12, page 42
        4+MIDI_NUM_CABLES,                      // bLength
        0x25,                                   // bDescriptorSubtype = CS_ENDPOINT
        0x01,                                   // bJackType = MS_GENERAL
        MIDI_NUM_CABLES,                        // bNumEmbMIDIJack = number of jacks
        3,                                      // BaAssocJackID(1) = jack ID #3
  #if MIDI_NUM_CABLES >= 2
        7,
  #endif
  #if MIDI_NUM_CABLES >= 3
        11,
  #endif
  #if MIDI_NUM_CABLES >= 4
        15,
  #endif
  #if MIDI_NUM_CABLES >= 5
        19,
  #endif
  #if MIDI_NUM_CABLES >= 6
        23,
  #endif
  #if MIDI_NUM_CABLES >= 7
        27,
  #endif
  #if MIDI_NUM_CABLES >= 8
        31,
  #endif
  #if MIDI_NUM_CABLES >= 9
        35,
  #endif
  #if MIDI_NUM_CABLES >= 10
        39,
  #endif
  #if MIDI_NUM_CABLES >= 11
        43,
  #endif
  #if MIDI_NUM_CABLES >= 12
        47,
  #endif
  #if MIDI_NUM_CABLES >= 13
        51,
  #endif
  #if MIDI_NUM_CABLES >= 14
        55,
  #endif
  #if MIDI_NUM_CABLES >= 15
        59,
  #endif
  #if MIDI_NUM_CABLES >= 16
        63,
  #endif
#endif // MIDI_INTERFACE

#ifdef MTP_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        MTP_INTERFACE,                          // bInterfaceNumber
        0,                                      // bAlternateSetting
        3,                                      // bNumEndpoints
        0x06,                                   // bInterfaceClass (0x06 = still image)
        0x01,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        4,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MTP_TX_SIZE_480),MSB(MTP_TX_SIZE_480), // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MTP_RX_SIZE_480),MSB(MTP_RX_SIZE_480), // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_EVENT_ENDPOINT | 0x80,              // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        MTP_EVENT_SIZE, 0,                      // wMaxPacketSize
        MTP_EVENT_INTERVAL_480,                 // bInterval
#endif // MTP_INTERFACE

#ifdef AUDIO_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        AUDIO_INTERFACE,                        // bFirstInterface
        3,                                      // bInterfaceCount
        0x01,                                   // bFunctionClass
        0x01,                                   // bFunctionSubClass
        0x00,                                   // bFunctionProtocol
        0,                                      // iFunction
	// Standard AudioControl (AC) Interface Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-1, page 36
	9,					// bLength
	4,					// bDescriptorType, 4 = INTERFACE
	AUDIO_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	1,					// bInterfaceSubclass, 1 = AUDIO_CONTROL
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-specific AC Interface Header Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-2, page 37-38
	10,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	0x01,					// bDescriptorSubtype, 1 = HEADER
	0x00, 0x01,				// bcdADC (version 1.0)
	LSB(62), MSB(62),			// wTotalLength
	2,					// bInCollection
	AUDIO_INTERFACE+1,			// baInterfaceNr(1) - Transmit to PC
	AUDIO_INTERFACE+2,			// baInterfaceNr(2) - Receive from PC
	// Input Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-3, page 39
	12,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	0x02,					// bDescriptorSubType, 2 = INPUT_TERMINAL
	1,					// bTerminalID
	//0x01, 0x02,				// wTerminalType, 0x0201 = MICROPHONE
	//0x03, 0x06,				// wTerminalType, 0x0603 = Line Connector
	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	0,					// bAssocTerminal, 0 = unidirectional
	2,					// bNrChannels
	0x03, 0x00,				// wChannelConfig, 0x0003 = Left & Right Front
	0,					// iChannelNames
	0, 					// iTerminal
	// Output Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-4, page 40
	9,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	3,					// bDescriptorSubtype, 3 = OUTPUT_TERMINAL
	2,					// bTerminalID
	0x01, 0x01,				// wTerminalType, 0x0101 = USB_STREAMING
	0,					// bAssocTerminal, 0 = unidirectional
	1,					// bCSourceID, connected to input terminal, ID=1
	0,					// iTerminal
	// Input Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-3, page 39
	12,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	2,					// bDescriptorSubType, 2 = INPUT_TERMINAL
	3,					// bTerminalID
	0x01, 0x01,				// wTerminalType, 0x0101 = USB_STREAMING
	0,					// bAssocTerminal, 0 = unidirectional
	2,					// bNrChannels
	0x03, 0x00,				// wChannelConfig, 0x0003 = Left & Right Front
	0,					// iChannelNames
	0, 					// iTerminal
	// Volume feature descriptor
	10,					// bLength
	0x24, 				// bDescriptorType = CS_INTERFACE
	0x06, 				// bDescriptorSubType = FEATURE_UNIT
	0x31, 				// bUnitID
	0x03, 				// bSourceID (Input Terminal)
	0x01, 				// bControlSize (each channel is 1 byte, 3 channels)
	0x01, 				// bmaControls(0) Master: Mute
	0x02, 				// bmaControls(1) Left: Volume
	0x02, 				// bmaControls(2) Right: Volume
	0x00,				// iFeature
	// Output Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-4, page 40
	9,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	3,					// bDescriptorSubtype, 3 = OUTPUT_TERMINAL
	4,					// bTerminalID
	//0x02, 0x03,				// wTerminalType, 0x0302 = Headphones
	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	0,					// bAssocTerminal, 0 = unidirectional
	0x31,				// bCSourceID, connected to feature, ID=31
	0,					// iTerminal
	// Standard AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.1, Table 4-18, page 59
	// Alternate 0: default setting, disabled zero bandwidth
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+1,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Alternate 1: streaming data
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+1,			// bInterfaceNumber
	1,					// bAlternateSetting
	1,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-Specific AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.2, Table 4-19, page 60
	7, 					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	1,					// bDescriptorSubtype, 1 = AS_GENERAL
	2,					// bTerminalLink: Terminal ID = 2
	3,					// bDelay (approx 3ms delay, audio lib updates)
	0x01, 0x00,				// wFormatTag, 0x0001 = PCM
	// Type I Format Descriptor
	// USB DCD for Audio Data Formats 1.0, Section 2.2.5, Table 2-1, page 10
	11,					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	2,					// bDescriptorSubtype = FORMAT_TYPE
	1,					// bFormatType = FORMAT_TYPE_I
	2,					// bNrChannels = 2
	2,					// bSubFrameSize = 2 byte
	16,					// bBitResolution = 16 bits
	1,					// bSamFreqType = 1 frequency
	LSB(44100), MSB(44100), 0,		// tSamFreq
	// Standard AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.1, Table 4-20, page 61-62
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_TX_ENDPOINT | 0x80,		// bEndpointAddress
	0x09, 					// bmAttributes = isochronous, adaptive
	LSB(AUDIO_TX_SIZE), MSB(AUDIO_TX_SIZE),	// wMaxPacketSize
	4,			 		// bInterval, 4 = every 8 micro-frames
	0,					// bRefresh
	0,					// bSynchAddress
	// Class-Specific AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.2, Table 4-21, page 62-63
	7,  					// bLength
	0x25,  					// bDescriptorType, 0x25 = CS_ENDPOINT
	1,  					// bDescriptorSubtype, 1 = EP_GENERAL
	0x00,  					// bmAttributes
	0,  					// bLockDelayUnits, 1 = ms
	0x00, 0x00,  				// wLockDelay
	// Standard AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.1, Table 4-18, page 59
	// Alternate 0: default setting, disabled zero bandwidth
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+2,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Alternate 1: streaming data
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+2,			// bInterfaceNumber
	1,					// bAlternateSetting
	2,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-Specific AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.2, Table 4-19, page 60
	7, 					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	1,					// bDescriptorSubtype, 1 = AS_GENERAL
	3,					// bTerminalLink: Terminal ID = 3
	3,					// bDelay (approx 3ms delay, audio lib updates)
	0x01, 0x00,				// wFormatTag, 0x0001 = PCM
	// Type I Format Descriptor
	// USB DCD for Audio Data Formats 1.0, Section 2.2.5, Table 2-1, page 10
	11,					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	2,					// bDescriptorSubtype = FORMAT_TYPE
	1,					// bFormatType = FORMAT_TYPE_I
	2,					// bNrChannels = 2
	2,					// bSubFrameSize = 2 byte
	16,					// bBitResolution = 16 bits
	1,					// bSamFreqType = 1 frequency
	LSB(44100), MSB(44100), 0,		// tSamFreq
	// Standard AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.1, Table 4-20, page 61-62
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_RX_ENDPOINT,			// bEndpointAddress
	0x05, 					// bmAttributes = isochronous, asynchronous
	LSB(AUDIO_RX_SIZE), MSB(AUDIO_RX_SIZE),	// wMaxPacketSize
	4,			 		// bInterval, 4 = every 8 micro-frames
	0,					// bRefresh
	AUDIO_SYNC_ENDPOINT | 0x80,		// bSynchAddress
	// Class-Specific AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.2, Table 4-21, page 62-63
	7,  					// bLength
	0x25,  					// bDescriptorType, 0x25 = CS_ENDPOINT
	1,  					// bDescriptorSubtype, 1 = EP_GENERAL
	0x00,  					// bmAttributes
	0,  					// bLockDelayUnits, 1 = ms
	0x00, 0x00,  				// wLockDelay
	// Standard AS Isochronous Audio Synch Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.2.1, Table 4-22, page 63-64
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_SYNC_ENDPOINT | 0x80,		// bEndpointAddress
	0x11, 					// bmAttributes = isochronous, feedback
	4, 0,					// wMaxPacketSize, 4 bytes
	4,			 		// bInterval, 4 = 4 = every 8 micro-frames
	7,					// bRefresh,
	0,					// bSynchAddress
#endif

#ifdef EXPERIMENTAL_INTERFACE
	// configuration for 480 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        EXPERIMENTAL_INTERFACE,                 // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0xFF,                                   // bInterfaceClass (0xFF = Vendor)
        0x6A,                                   // bInterfaceSubClass
        0xC7,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        1 | 0x80,                               // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(512), MSB(512),                     // wMaxPacketSize
        1,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        1,                                      // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(512), MSB(512),                     // wMaxPacketSize
        1,                                      // bInterval
#endif // EXPERIMENTAL_INTERFACE
};


PROGMEM const uint8_t usb_config_descriptor_12[CONFIG_DESC_SIZE] = {
        // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
        9,                                      // bLength;
        2,                                      // bDescriptorType;
        LSB(CONFIG_DESC_SIZE),                 // wTotalLength
        MSB(CONFIG_DESC_SIZE),
        NUM_INTERFACE,                          // bNumInterfaces
        1,                                      // bConfigurationValue
        0,                                      // iConfiguration
        0xC0,                                   // bmAttributes
        50,                                     // bMaxPower

#ifdef CDC_IAD_DESCRIPTOR
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC_STATUS_INTERFACE,                   // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
#endif

#ifdef CDC_DATA_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_STATUS_INTERFACE,			// bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC_STATUS_INTERFACE,                   // bMasterInterface
        CDC_DATA_INTERFACE,                     // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_ACM_ENDPOINT | 0x80,                // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        16,                                     // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_DATA_INTERFACE,                     // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_RX_SIZE_12),MSB(CDC_RX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(CDC_TX_SIZE_12),MSB(CDC_TX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
#endif // CDC_DATA_INTERFACE

#ifdef CDC2_DATA_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC2_STATUS_INTERFACE,                  // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC2_STATUS_INTERFACE,                  // bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC2_STATUS_INTERFACE,                  // bMasterInterface
        CDC2_DATA_INTERFACE,                    // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_ACM_ENDPOINT | 0x80,               // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        64,                                     // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC2_DATA_INTERFACE,                    // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_RX_SIZE_12, 0,                      // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC2_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_TX_SIZE_12, 0,                      // wMaxPacketSize
        0,                                      // bInterval
#endif // CDC2_DATA_INTERFACE

#ifdef CDC3_DATA_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC3_STATUS_INTERFACE,                  // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        0,                                      // iFunction
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC3_STATUS_INTERFACE,                  // bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC3_STATUS_INTERFACE,                  // bMasterInterface
        CDC3_DATA_INTERFACE,                    // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_ACM_ENDPOINT | 0x80,               // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        64,                                     // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC3_DATA_INTERFACE,                    // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_RX_SIZE_12, 0,                      // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC3_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_TX_SIZE_12, 0,                      // wMaxPacketSize
        0,                                      // bInterval
#endif // CDC3_DATA_INTERFACE

#ifdef MIDI_INTERFACE
	// configuration for 12 Mbit/sec speed
        // Standard MS Interface Descriptor,
        9,                                      // bLength
        4,                                      // bDescriptorType
        MIDI_INTERFACE,                         // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x01,                                   // bInterfaceClass (0x01 = Audio)
        0x03,                                   // bInterfaceSubClass (0x03 = MIDI)
        0x00,                                   // bInterfaceProtocol (unused for MIDI)
        0,                                      // iInterface
        // MIDI MS Interface Header, USB MIDI 6.1.2.1, page 21, Table 6-2
        7,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x01,                                   // bDescriptorSubtype = MS_HEADER
        0x00, 0x01,                             // bcdMSC = revision 01.00
	LSB(7+(6+6+9+9)*MIDI_NUM_CABLES),       // wTotalLength
	MSB(7+(6+6+9+9)*MIDI_NUM_CABLES),
        // MIDI IN Jack Descriptor, B.4.3, Table B-7 (embedded), page 40
        6,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x02,                                   // bDescriptorSubtype = MIDI_IN_JACK
        0x01,                                   // bJackType = EMBEDDED
        1,                                      // bJackID, ID = 1
        0,                                      // iJack
        // MIDI IN Jack Descriptor, B.4.3, Table B-8 (external), page 40
        6,                                      // bLength
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x02,                                   // bDescriptorSubtype = MIDI_IN_JACK
        0x02,                                   // bJackType = EXTERNAL
        2,                                      // bJackID, ID = 2
        0,                                      // iJack
        // MIDI OUT Jack Descriptor, B.4.4, Table B-9, page 41
        9,
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x03,                                   // bDescriptorSubtype = MIDI_OUT_JACK
        0x01,                                   // bJackType = EMBEDDED
        3,                                      // bJackID, ID = 3
        1,                                      // bNrInputPins = 1 pin
        2,                                      // BaSourceID(1) = 2
        1,                                      // BaSourcePin(1) = first pin
        0,                                      // iJack
        // MIDI OUT Jack Descriptor, B.4.4, Table B-10, page 41
        9,
        0x24,                                   // bDescriptorType = CS_INTERFACE
        0x03,                                   // bDescriptorSubtype = MIDI_OUT_JACK
        0x02,                                   // bJackType = EXTERNAL
        4,                                      // bJackID, ID = 4
        1,                                      // bNrInputPins = 1 pin
        1,                                      // BaSourceID(1) = 1
        1,                                      // BaSourcePin(1) = first pin
        0,                                      // iJack
  #if MIDI_NUM_CABLES >= 2
	#define MIDI_INTERFACE_JACK_PAIR(a, b, c, d) \
		6, 0x24, 0x02, 0x01, (a), 0, \
		6, 0x24, 0x02, 0x02, (b), 0, \
		9, 0x24, 0x03, 0x01, (c), 1, (b), 1, 0, \
		9, 0x24, 0x03, 0x02, (d), 1, (a), 1, 0,
	MIDI_INTERFACE_JACK_PAIR(5, 6, 7, 8)
  #endif
  #if MIDI_NUM_CABLES >= 3
	MIDI_INTERFACE_JACK_PAIR(9, 10, 11, 12)
  #endif
  #if MIDI_NUM_CABLES >= 4
	MIDI_INTERFACE_JACK_PAIR(13, 14, 15, 16)
  #endif
  #if MIDI_NUM_CABLES >= 5
	MIDI_INTERFACE_JACK_PAIR(17, 18, 19, 20)
  #endif
  #if MIDI_NUM_CABLES >= 6
	MIDI_INTERFACE_JACK_PAIR(21, 22, 23, 24)
  #endif
  #if MIDI_NUM_CABLES >= 7
	MIDI_INTERFACE_JACK_PAIR(25, 26, 27, 28)
  #endif
  #if MIDI_NUM_CABLES >= 8
	MIDI_INTERFACE_JACK_PAIR(29, 30, 31, 32)
  #endif
  #if MIDI_NUM_CABLES >= 9
	MIDI_INTERFACE_JACK_PAIR(33, 34, 35, 36)
  #endif
  #if MIDI_NUM_CABLES >= 10
	MIDI_INTERFACE_JACK_PAIR(37, 38, 39, 40)
  #endif
  #if MIDI_NUM_CABLES >= 11
	MIDI_INTERFACE_JACK_PAIR(41, 42, 43, 44)
  #endif
  #if MIDI_NUM_CABLES >= 12
	MIDI_INTERFACE_JACK_PAIR(45, 46, 47, 48)
  #endif
  #if MIDI_NUM_CABLES >= 13
	MIDI_INTERFACE_JACK_PAIR(49, 50, 51, 52)
  #endif
  #if MIDI_NUM_CABLES >= 14
	MIDI_INTERFACE_JACK_PAIR(53, 54, 55, 56)
  #endif
  #if MIDI_NUM_CABLES >= 15
	MIDI_INTERFACE_JACK_PAIR(57, 58, 59, 60)
  #endif
  #if MIDI_NUM_CABLES >= 16
	MIDI_INTERFACE_JACK_PAIR(61, 62, 63, 64)
  #endif
        // Standard Bulk OUT Endpoint Descriptor, B.5.1, Table B-11, pae 42
        9,                                      // bLength
        5,                                      // bDescriptorType = ENDPOINT
        MIDI_RX_ENDPOINT,                       // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MIDI_RX_SIZE_12),MSB(MIDI_RX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
        0,                                      // bRefresh
        0,                                      // bSynchAddress
        // Class-specific MS Bulk OUT Endpoint Descriptor, B.5.2, Table B-12, page 42
        4+MIDI_NUM_CABLES,                      // bLength
        0x25,                                   // bDescriptorSubtype = CS_ENDPOINT
        0x01,                                   // bJackType = MS_GENERAL
        MIDI_NUM_CABLES,                        // bNumEmbMIDIJack = number of jacks
        1,                                      // BaAssocJackID(1) = jack ID #1
  #if MIDI_NUM_CABLES >= 2
        5,
  #endif
  #if MIDI_NUM_CABLES >= 3
        9,
  #endif
  #if MIDI_NUM_CABLES >= 4
        13,
  #endif
  #if MIDI_NUM_CABLES >= 5
        17,
  #endif
  #if MIDI_NUM_CABLES >= 6
        21,
  #endif
  #if MIDI_NUM_CABLES >= 7
        25,
  #endif
  #if MIDI_NUM_CABLES >= 8
        29,
  #endif
  #if MIDI_NUM_CABLES >= 9
        33,
  #endif
  #if MIDI_NUM_CABLES >= 10
        37,
  #endif
  #if MIDI_NUM_CABLES >= 11
        41,
  #endif
  #if MIDI_NUM_CABLES >= 12
        45,
  #endif
  #if MIDI_NUM_CABLES >= 13
        49,
  #endif
  #if MIDI_NUM_CABLES >= 14
        53,
  #endif
  #if MIDI_NUM_CABLES >= 15
        57,
  #endif
  #if MIDI_NUM_CABLES >= 16
        61,
  #endif
        // Standard Bulk IN Endpoint Descriptor, B.5.1, Table B-11, pae 42
        9,                                      // bLength
        5,                                      // bDescriptorType = ENDPOINT
        MIDI_TX_ENDPOINT | 0x80,                // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MIDI_TX_SIZE_12),MSB(MIDI_TX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
        0,                                      // bRefresh
        0,                                      // bSynchAddress
        // Class-specific MS Bulk IN Endpoint Descriptor, B.5.2, Table B-12, page 42
        4+MIDI_NUM_CABLES,                      // bLength
        0x25,                                   // bDescriptorSubtype = CS_ENDPOINT
        0x01,                                   // bJackType = MS_GENERAL
        MIDI_NUM_CABLES,                        // bNumEmbMIDIJack = number of jacks
        3,                                      // BaAssocJackID(1) = jack ID #3
  #if MIDI_NUM_CABLES >= 2
        7,
  #endif
  #if MIDI_NUM_CABLES >= 3
        11,
  #endif
  #if MIDI_NUM_CABLES >= 4
        15,
  #endif
  #if MIDI_NUM_CABLES >= 5
        19,
  #endif
  #if MIDI_NUM_CABLES >= 6
        23,
  #endif
  #if MIDI_NUM_CABLES >= 7
        27,
  #endif
  #if MIDI_NUM_CABLES >= 8
        31,
  #endif
  #if MIDI_NUM_CABLES >= 9
        35,
  #endif
  #if MIDI_NUM_CABLES >= 10
        39,
  #endif
  #if MIDI_NUM_CABLES >= 11
        43,
  #endif
  #if MIDI_NUM_CABLES >= 12
        47,
  #endif
  #if MIDI_NUM_CABLES >= 13
        51,
  #endif
  #if MIDI_NUM_CABLES >= 14
        55,
  #endif
  #if MIDI_NUM_CABLES >= 15
        59,
  #endif
  #if MIDI_NUM_CABLES >= 16
        63,
  #endif
#endif // MIDI_INTERFACE

#ifdef MTP_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        MTP_INTERFACE,                          // bInterfaceNumber
        0,                                      // bAlternateSetting
        3,                                      // bNumEndpoints
        0x06,                                   // bInterfaceClass (0x06 = still image)
        0x01,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MTP_TX_SIZE_12),MSB(MTP_TX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(MTP_RX_SIZE_12),MSB(MTP_RX_SIZE_12),// wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        MTP_EVENT_ENDPOINT | 0x80,              // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        MTP_EVENT_SIZE, 0,                      // wMaxPacketSize
        MTP_EVENT_INTERVAL_12,                  // bInterval
#endif // MTP_INTERFACE

#ifdef AUDIO_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        AUDIO_INTERFACE,                        // bFirstInterface
        3,                                      // bInterfaceCount
        0x01,                                   // bFunctionClass
        0x01,                                   // bFunctionSubClass
        0x00,                                   // bFunctionProtocol
        0,                                      // iFunction
	// Standard AudioControl (AC) Interface Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-1, page 36
	9,					// bLength
	4,					// bDescriptorType, 4 = INTERFACE
	AUDIO_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	1,					// bInterfaceSubclass, 1 = AUDIO_CONTROL
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-specific AC Interface Header Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-2, page 37-38
	10,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	0x01,					// bDescriptorSubtype, 1 = HEADER
	0x00, 0x01,				// bcdADC (version 1.0)
	LSB(62), MSB(62),			// wTotalLength
	2,					// bInCollection
	AUDIO_INTERFACE+1,			// baInterfaceNr(1) - Transmit to PC
	AUDIO_INTERFACE+2,			// baInterfaceNr(2) - Receive from PC
	// Input Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-3, page 39
	12,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	0x02,					// bDescriptorSubType, 2 = INPUT_TERMINAL
	1,					// bTerminalID
	//0x01, 0x02,				// wTerminalType, 0x0201 = MICROPHONE
	//0x03, 0x06,				// wTerminalType, 0x0603 = Line Connector
	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	0,					// bAssocTerminal, 0 = unidirectional
	2,					// bNrChannels
	0x03, 0x00,				// wChannelConfig, 0x0003 = Left & Right Front
	0,					// iChannelNames
	0, 					// iTerminal
	// Output Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-4, page 40
	9,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	3,					// bDescriptorSubtype, 3 = OUTPUT_TERMINAL
	2,					// bTerminalID
	0x01, 0x01,				// wTerminalType, 0x0101 = USB_STREAMING
	0,					// bAssocTerminal, 0 = unidirectional
	1,					// bCSourceID, connected to input terminal, ID=1
	0,					// iTerminal
	// Input Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-3, page 39
	12,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	2,					// bDescriptorSubType, 2 = INPUT_TERMINAL
	3,					// bTerminalID
	0x01, 0x01,				// wTerminalType, 0x0101 = USB_STREAMING
	0,					// bAssocTerminal, 0 = unidirectional
	2,					// bNrChannels
	0x03, 0x00,				// wChannelConfig, 0x0003 = Left & Right Front
	0,					// iChannelNames
	0, 					// iTerminal
	// Volume feature descriptor
	10,					// bLength
	0x24, 				// bDescriptorType = CS_INTERFACE
	0x06, 				// bDescriptorSubType = FEATURE_UNIT
	0x31, 				// bUnitID
	0x03, 				// bSourceID (Input Terminal)
	0x01, 				// bControlSize (each channel is 1 byte, 3 channels)
	0x01, 				// bmaControls(0) Master: Mute
	0x02, 				// bmaControls(1) Left: Volume
	0x02, 				// bmaControls(2) Right: Volume
	0x00,				// iFeature
	// Output Terminal Descriptor
	// USB DCD for Audio Devices 1.0, Table 4-4, page 40
	9,					// bLength
	0x24,					// bDescriptorType, 0x24 = CS_INTERFACE
	3,					// bDescriptorSubtype, 3 = OUTPUT_TERMINAL
	4,					// bTerminalID
	//0x02, 0x03,				// wTerminalType, 0x0302 = Headphones
	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	0,					// bAssocTerminal, 0 = unidirectional
	0x31,				// bCSourceID, connected to feature, ID=31
	0,					// iTerminal
	// Standard AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.1, Table 4-18, page 59
	// Alternate 0: default setting, disabled zero bandwidth
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+1,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Alternate 1: streaming data
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+1,			// bInterfaceNumber
	1,					// bAlternateSetting
	1,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-Specific AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.2, Table 4-19, page 60
	7, 					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	1,					// bDescriptorSubtype, 1 = AS_GENERAL
	2,					// bTerminalLink: Terminal ID = 2
	3,					// bDelay (approx 3ms delay, audio lib updates)
	0x01, 0x00,				// wFormatTag, 0x0001 = PCM
	// Type I Format Descriptor
	// USB DCD for Audio Data Formats 1.0, Section 2.2.5, Table 2-1, page 10
	11,					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	2,					// bDescriptorSubtype = FORMAT_TYPE
	1,					// bFormatType = FORMAT_TYPE_I
	2,					// bNrChannels = 2
	2,					// bSubFrameSize = 2 byte
	16,					// bBitResolution = 16 bits
	1,					// bSamFreqType = 1 frequency
	LSB(44100), MSB(44100), 0,		// tSamFreq
	// Standard AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.1, Table 4-20, page 61-62
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_TX_ENDPOINT | 0x80,		// bEndpointAddress
	0x09, 					// bmAttributes = isochronous, adaptive
	LSB(AUDIO_TX_SIZE), MSB(AUDIO_TX_SIZE),	// wMaxPacketSize
	1,			 		// bInterval, 1 = every frame
	0,					// bRefresh
	0,					// bSynchAddress
	// Class-Specific AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.2, Table 4-21, page 62-63
	7,  					// bLength
	0x25,  					// bDescriptorType, 0x25 = CS_ENDPOINT
	1,  					// bDescriptorSubtype, 1 = EP_GENERAL
	0x00,  					// bmAttributes
	0,  					// bLockDelayUnits, 1 = ms
	0x00, 0x00,  				// wLockDelay
	// Standard AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.1, Table 4-18, page 59
	// Alternate 0: default setting, disabled zero bandwidth
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+2,			// bInterfaceNumber
	0,					// bAlternateSetting
	0,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Alternate 1: streaming data
	9,					// bLenght
	4,					// bDescriptorType = INTERFACE
	AUDIO_INTERFACE+2,			// bInterfaceNumber
	1,					// bAlternateSetting
	2,					// bNumEndpoints
	1,					// bInterfaceClass, 1 = AUDIO
	2,					// bInterfaceSubclass, 2 = AUDIO_STREAMING
	0,					// bInterfaceProtocol
	0,					// iInterface
	// Class-Specific AS Interface Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.5.2, Table 4-19, page 60
	7, 					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	1,					// bDescriptorSubtype, 1 = AS_GENERAL
	3,					// bTerminalLink: Terminal ID = 3
	3,					// bDelay (approx 3ms delay, audio lib updates)
	0x01, 0x00,				// wFormatTag, 0x0001 = PCM
	// Type I Format Descriptor
	// USB DCD for Audio Data Formats 1.0, Section 2.2.5, Table 2-1, page 10
	11,					// bLength
	0x24,					// bDescriptorType = CS_INTERFACE
	2,					// bDescriptorSubtype = FORMAT_TYPE
	1,					// bFormatType = FORMAT_TYPE_I
	2,					// bNrChannels = 2
	2,					// bSubFrameSize = 2 byte
	16,					// bBitResolution = 16 bits
	1,					// bSamFreqType = 1 frequency
	LSB(44100), MSB(44100), 0,		// tSamFreq
	// Standard AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.1, Table 4-20, page 61-62
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_RX_ENDPOINT,			// bEndpointAddress
	0x05, 					// bmAttributes = isochronous, asynchronous
	LSB(AUDIO_RX_SIZE), MSB(AUDIO_RX_SIZE),	// wMaxPacketSize
	1,			 		// bInterval, 1 = every frame
	0,					// bRefresh
	AUDIO_SYNC_ENDPOINT | 0x80,		// bSynchAddress
	// Class-Specific AS Isochronous Audio Data Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.1.2, Table 4-21, page 62-63
	7,  					// bLength
	0x25,  					// bDescriptorType, 0x25 = CS_ENDPOINT
	1,  					// bDescriptorSubtype, 1 = EP_GENERAL
	0x00,  					// bmAttributes
	0,  					// bLockDelayUnits, 1 = ms
	0x00, 0x00,  				// wLockDelay
	// Standard AS Isochronous Audio Synch Endpoint Descriptor
	// USB DCD for Audio Devices 1.0, Section 4.6.2.1, Table 4-22, page 63-64
	9, 					// bLength
	5, 					// bDescriptorType, 5 = ENDPOINT_DESCRIPTOR
	AUDIO_SYNC_ENDPOINT | 0x80,		// bEndpointAddress
	0x11, 					// bmAttributes = isochronous, feedback
	3, 0,					// wMaxPacketSize, 3 bytes
	1,			 		// bInterval, 1 = every frame
	5,					// bRefresh, 5 = 32ms
	0,					// bSynchAddress
#endif

#ifdef EXPERIMENTAL_INTERFACE
	// configuration for 12 Mbit/sec speed
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        EXPERIMENTAL_INTERFACE,                 // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0xFF,                                   // bInterfaceClass (0xFF = Vendor)
        0x6A,                                   // bInterfaceSubClass
        0xFF,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        1 | 0x80,                               // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(64), MSB(64),                       // wMaxPacketSize
        1,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        1,                                      // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        LSB(64), MSB(64),                       // wMaxPacketSize
        1,                                      // bInterval
#endif // EXPERIMENTAL_INTERFACE
};


__attribute__ ((section(".dmabuffers"), aligned(32)))
uint8_t usb_descriptor_buffer[CONFIG_DESC_SIZE];





// **************************************************************
//   String Descriptors
// **************************************************************

// The descriptors above can provide human readable strings,
// referenced by index numbers.  These descriptors are the
// actual string data

/* defined in usb_names.h
struct usb_string_descriptor_struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wString[];
};
*/

extern struct usb_string_descriptor_struct usb_string_manufacturer_name
        __attribute__ ((weak, alias("usb_string_manufacturer_name_default")));
extern struct usb_string_descriptor_struct usb_string_product_name
        __attribute__ ((weak, alias("usb_string_product_name_default")));
extern struct usb_string_descriptor_struct usb_string_serial_number
        __attribute__ ((weak, alias("usb_string_serial_number_default")));

PROGMEM const struct usb_string_descriptor_struct string0 = {
        4,
        3,
        {0x0409}
};

PROGMEM const struct usb_string_descriptor_struct usb_string_manufacturer_name_default = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME
};
PROGMEM const struct usb_string_descriptor_struct usb_string_product_name_default = {
	2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME
};
struct usb_string_descriptor_struct usb_string_serial_number_default = {
        12,
        3,
        {0,0,0,0,0,0,0,0,0,0}
};
#ifdef MTP_INTERFACE
PROGMEM const struct usb_string_descriptor_struct usb_string_mtp = {
	2 + 3 * 2,
	3,
	{'M','T','P'}
};
#endif

void usb_init_serialnumber(void)
{
	char buf[11];
	uint32_t i, num;

	num = HW_OCOTP_MAC0 & 0xFFFFFF;
	// add extra zero to work around OS-X CDC-ACM driver bug
	if (num < 10000000) num = num * 10;
	ultoa(num, buf, 10);
	for (i=0; i<10; i++) {
		char c = buf[i];
		if (!c) break;
		usb_string_serial_number_default.wString[i] = c;
	}
	usb_string_serial_number_default.bLength = i * 2 + 2;
}


// **************************************************************
//   Descriptors List
// **************************************************************

// This table provides access to all the descriptor data above.

const usb_descriptor_list_t usb_descriptor_list[] = {
	//wValue, wIndex, address,          length
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0600, 0x0000, qualifier_descriptor, sizeof(qualifier_descriptor)},
	{0x0200, 0x0000, usb_config_descriptor_480, CONFIG_DESC_SIZE},
	{0x0700, 0x0000, usb_config_descriptor_12, CONFIG_DESC_SIZE},
#ifdef MTP_INTERFACE
	{0x0304, 0x0409, (const uint8_t *)&usb_string_mtp, 0},
#endif
#ifdef EXPERIMENTAL_INTERFACE
	{0x03EE, 0x0000, microsoft_os_string_desc, sizeof(microsoft_os_string_desc)},
	{0x0000, 0xEE04, microsoft_os_compatible_id_desc, sizeof(microsoft_os_compatible_id_desc)},
#endif
        {0x0300, 0x0000, (const uint8_t *)&string0, 0},
        {0x0301, 0x0409, (const uint8_t *)&usb_string_manufacturer_name, 0},
        {0x0302, 0x0409, (const uint8_t *)&usb_string_product_name, 0},
        {0x0303, 0x0409, (const uint8_t *)&usb_string_serial_number, 0},
	{0, 0, NULL, 0}
};





#endif // NUM_ENDPOINTS
//#endif // F_CPU >= 20 MHz
