#include "ps4.h"

#include <esp_system.h>
#include "esp_log.h"

#include "ps4_int.h"

/********************************************************************************/
/*                              C O N S T A N T S */
/********************************************************************************/

static const uint8_t hid_cmd_payload_ps4_enable[] = {0x43, 0x02};

/********************************************************************************/
/*                         L O C A L    V A R I A B L E S */
/********************************************************************************/

static ps4_connection_callback_t ps4_connection_cb = NULL;
static ps4_connection_object_callback_t ps4_connection_object_cb = NULL;
static void* ps4_connection_object = NULL;

static ps4_event_callback_t ps4_event_cb = NULL;
static ps4_event_object_callback_t ps4_event_object_cb = NULL;
static void* ps4_event_object = NULL;

static bool is_active = false;		// NEW

/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S */
/********************************************************************************/

/*******************************************************************************
**
** Function         ps4Init
**
** Description      This initializes the bluetooth services to listen
**                  for an incoming PS4 controller connection.
**
**
** Returns          void
**
*******************************************************************************/
// REMOVE!
/* void ps4Init() {
  sppInit();
  gapInitServices();
} */

// NEW
/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S                        */
/********************************************************************************/

/*******************************************************************************
**
** Function         ps4Init
**
** Description      This initializes the bluetooth services to listen
**                  for an incoming PS3 controller connection.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Init()
{
    sppInit();
    l2cap_init_services();
}

/*******************************************************************************
**
** Function         ps3Deinit
**
** Description      This deinitializes the bluetooth services to stop
**                  listening for incoming connections.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Deinit()
{
    l2cap_deinit_services();
    spp_deinit();
}
// ---------------------------------------- //



/*******************************************************************************
**
** Function         ps4IsConnected
**
** Description      This returns whether a PS4 controller is connected, based
**                  on whether a successful handshake has taken place.
**
**
** Returns          bool
**
*******************************************************************************/
//bool ps4IsConnected() { return gapIsConnected(); }
bool ps4IsConnected() { return is_active; }

void ps4Disconnected() { is_active = false; }

/*******************************************************************************
**
** Function         ps4Enable
**
** Description      This triggers the PS4 controller to start continually
**                  sending its data.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Enable() {
	ESP_LOGE("PS4", "Enable");
  uint8_t length = sizeof(hid_cmd_payload_ps4_enable);
  hid_cmd_t hidCommand;

  //hidCommand.code = hid_cmd_code_set_report | hid_cmd_code_type_feature;
  //hidCommand.code = 0x43;
  //hidCommand.identifier = hid_cmd_identifier_ps4_enable;
  //hidCommand.identifier = 0x02;
  //memcpy(hidCommand.data, hid_cmd_payload_ps4_enable, length);

  //gapSendHid(&hidCommand, length);
  //l2cap_send_hid( &hidCommand, length );
  uint8_t buf[2];
                buf[0] = 0x43; // HID BT Get_report (0x40) | Report Type (Feature 0x03)
                buf[1] = 0x02; // Report ID
				l2cap_send_hid( buf, sizeof(buf));
  ps4SetLed(255, 0, 0);
}

/*******************************************************************************
**
** Function         ps4Cmd
**
** Description      Send a command to the PS4 controller.
**
**
** Returns          void
**
*******************************************************************************/
void ps4Cmd(ps4_cmd_t cmd) {
  hid_cmd_t hidCommand = {.data = {0x80, 0x00, 0xFF}};
  uint16_t length = sizeof(hidCommand.data);
  
  hidCommand.code = hid_cmd_code_set_report | hid_cmd_code_type_output;
  hidCommand.identifier = hid_cmd_identifier_ps4_control;
	//bufferData[0] = hid_cmd_code_set_report | hid_cmd_code_type_output;
  hidCommand.data[ps4_control_packet_index_small_rumble] = cmd.smallRumble;  // Small Rumble
  hidCommand.data[ps4_control_packet_index_large_rumble] = cmd.largeRumble;  // Big rumble

  hidCommand.data[ps4_control_packet_index_red] = cmd.r;    // Red
  hidCommand.data[ps4_control_packet_index_green] = cmd.g;  // Green
  hidCommand.data[ps4_control_packet_index_blue] = cmd.b;   // Blue

  // Time to flash bright (255 = 2.5 seconds)
  hidCommand.data[ps4_control_packet_index_flash_on_time] = cmd.flashOn;
  // Time to flash dark (255 = 2.5 seconds)
  hidCommand.data[ps4_control_packet_index_flash_off_time] = cmd.flashOff;

	/*uint8_t buf[79];
                memset(buf, 0, sizeof(buf));

                buf[0] = 0x52; // HID BT Set_report (0x50) | Report Type (Output 0x02)
                buf[1] = 0x11; // Report ID
                buf[2] = 0x80;
                buf[4]= 0xFF;

                buf[7] = cmd.smallRumble;  // Small Rumble
                buf[8] = cmd.largeRumble;  // Big rumble

                buf[9] = cmd.r; // Red
                buf[10] = cmd.g; // Green
                buf[11] = cmd.b; // Blue

                buf[12] = cmd.flashOn; // Time to flash bright (255 = 2.5 seconds)
                buf[13] = cmd.flashOff; // Time to flash dark (255 = 2.5 seconds)

*/
  //gapSendHid(&hidCommand, length);
  //ESP_LOGE("Data: ", "%02x", hidCommand.data[9]);
  //ESP_LOGE("Data: ",  "%08x", buf);
  //uint8_t length = sizeof(hidCommand.data);
  l2cap_send_hid( &hidCommand, length );
  //l2cap_send_hid( buf, sizeof(buf));
}

/*******************************************************************************
**
** Function         ps4SetLedOnly
**
** Description      Sets the LEDs on the PS4 controller.
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetLed(uint8_t r, uint8_t g, uint8_t b) {
  ps4_cmd_t cmd = {0};

  cmd.r = r;
  cmd.g = g;
  cmd.b = b;

  ps4Cmd(cmd);
}

/*******************************************************************************
**
** Function         ps4SetOutput
**
** Description      Sets feedback on the PS4 controller.
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetOutput(ps4_cmd_t prevCommand) { ps4Cmd(prevCommand); }

/*******************************************************************************
**
** Function         ps4SetConnectionCallback
**
** Description      Registers a callback for receiving PS4 controller
**                  connection notifications
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetConnectionCallback(ps4_connection_callback_t cb) {
  ps4_connection_cb = cb;
}

/*******************************************************************************
**
** Function         ps4SetConnectionObjectCallback
**
** Description      Registers a callback for receiving PS4 controller
**                  connection notifications
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetConnectionObjectCallback(void* object, ps4_connection_object_callback_t cb) {
  ps4_connection_object_cb = cb;
  ps4_connection_object = object;
}

/*******************************************************************************
**
** Function         ps4SetEventCallback
**
** Description      Registers a callback for receiving PS4 controller events
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetEventCallback(ps4_event_callback_t cb) { ps4_event_cb = cb; }

/*******************************************************************************
**
** Function         ps4SetEventObjectCallback
**
** Description      Registers a callback for receiving PS4 controller events
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetEventObjectCallback(void* object, ps4_event_object_callback_t cb) {
  ps4_event_object_cb = cb;
  ps4_event_object = object;
}

/*******************************************************************************
**
** Function         ps4SetBluetoothMacAddress
**
** Description      Writes a Registers a callback for receiving PS4 controller
*events
**
**
** Returns          void
**
*******************************************************************************/
void ps4SetBluetoothMacAddress(const uint8_t* mac) {
  // The bluetooth MAC address is derived from the base MAC address
  // https://docs.espressif.com/projects/esp-idf/en/stable/api-reference/system/system.html#mac-address
  ESP_LOGE("PS4", "MAC: %02X", mac);
  uint8_t baseMac[6];
  memcpy(baseMac, mac, 6);
  baseMac[5] -= 2;
  ESP_LOGE("PS4", "%02X", baseMac);
  esp_base_mac_addr_set(baseMac);
}

/********************************************************************************/
/*                      L O C A L    F U N C T I O N S */
/********************************************************************************/

void ps4ConnectEvent(uint8_t isConnected) {
  if (isConnected) {
    ps4Enable();
	ESP_LOGE("Connected", "TRUE!");
  } else {
	  is_active = false;
  }
}

void ps4PacketEvent(ps4_t ps4, ps4_event_t event, uint8_t* packet) {
	//ESP_LOGE("Packet: ", "YES!");
	if (is_active) {
		//ESP_LOGE("IS ACTIVE?", " YES!");
		if (ps4_event_cb != NULL) {
			ps4_event_cb(ps4, event, packet);
		}

		if (ps4_event_object_cb != NULL && ps4_event_object != NULL) {
			ps4_event_object_cb(ps4_event_object, ps4, event);
		}
	} else {
		is_active = true;
		
		if (ps4_connection_cb != NULL) {
			ps4_connection_cb(is_active);
		}

		if (ps4_connection_object_cb != NULL && ps4_connection_object != NULL) {
			ps4_connection_object_cb(ps4_connection_object, is_active);
		}
	}
}
