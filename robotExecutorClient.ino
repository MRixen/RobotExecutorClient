struct dxl_commands
{
	int GOAL_POSITION;
};

union DXL_IDs
{
	short dxl_ids;
	byte bytes[2];
};

union DXL_MSG_START
{
	short dxl_msg_start;
	byte bytes[4];
};

union DXL_POSITION_1
{
	short dxl_position;
	byte bytes[2];
};

union DXL_POSITION_2
{
	short dxl_position;
	byte bytes[2];
};

int dxl_speed;

byte controlData[35];

int DXL_BUS_SERIAL = 3;
int DXL_MOTOR_AMOUNT = 1;

typedef struct dxl_commands Dxl_commands;
Dxl_commands DXL_COMMANDS;
Dynamixel Dxl(DXL_BUS_SERIAL);


bool debugMode;


void setup() {

	// Init user defines
	debugMode = true;

	if (debugMode) Serial1.begin(115200);

	// Init dxl commandos
	DXL_COMMANDS.GOAL_POSITION = 30;

	// Init usb connection
	SerialUSB.attachInterrupt(usbInterrupt);

	// Init dxl motor
	Dxl.begin(3);
	for (int i = 0; i < DXL_MOTOR_AMOUNT; i++) Dxl.jointMode(i + 1);
}

//USB max packet data is maximum 64byte, so nCount can not exceeds 64 bytes
void usbInterrupt(byte* buffer, byte nCount) {
	int positionData[16];
	int idData[16];

	if (debugMode) toggleLED();

	// Read everything from stream
	for (unsigned int i = 0; i < nCount; i++) {
		if (debugMode) Serial1.write(buffer[i]);
		controlData[i] = buffer[i];
	}

	// Extract start message from stream
	DXL_MSG_START msgStart;
	msgStart.bytes[0] = controlData[0];
	msgStart.bytes[1] = controlData[1];
	msgStart.bytes[2] = controlData[2];
	msgStart.bytes[3] = controlData[3];

	if (debugMode) Serial1.write(buffer[i]);
	// Extract ids from stream
	//DXL_IDs ids;
	//ids.bytes[0] = controlData[2];
	//ids.bytes[1] = controlData[3];

	// TODO Print the ids to see what is inside

	//for (size_t i = 0; i < 16; i++) {

		//idData[i] = ids.dxl_ids & (1 << i);
		//Serial1.println(idData[i]);
		//Serial1.println((1 << i));
	//}

	// Extract speed
	//dxl_speed = controlData[4];

	// Extract positions
	/*DXL_POSITION_1 position_1;
	position_1.bytes[0] = controlData[5];
	position_1.bytes[1] = controlData[6];

	DXL_POSITION_2 position_2;
	position_2.bytes[0] = controlData[7];
	position_2.bytes[1] = controlData[8];

	positionData[0] = position_1.dxl_position;
	positionData[1] = position_2.dxl_position;*/

	// TODO: Add SPEED data
	// TODO: Print the last data
	for (size_t i = 0; i < 2; i++) {
		if (idData[i] != 0) Dxl.writeWord(idData[i], DXL_COMMANDS.GOAL_POSITION, positionData[i]);
		//if (debugMode) Serial1.write(idData[i]);

		//if (debugMode) Serial1.write(positionData[i]);
	}
}

void loop() {


}

