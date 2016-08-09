#define lowByte(w) ((uint8) ((w) & 0xff))
#define highByte(w) ((uint8) ((w) >> 8))

struct dxl_commands
{
	int GOAL_POSITION;
	int PRESENT_POSITION;
};

union DXL_MSG
{
	int dxl_msg;
	byte bytes[4];
};

union DXL_MSG_RETURN
{
	short dxl_msg;
	byte bytes[2];
};
DXL_MSG_RETURN positionRequest;

int DXL_BUS_SERIAL = 3;
int DXL_MOTOR_AMOUNT = 1;

typedef struct dxl_commands Dxl_commands;
Dxl_commands DXL_COMMANDS;
Dynamixel Dxl(DXL_BUS_SERIAL);

bool transmissionIsActive = false;

bool debugMode;
byte isMoving = 0;
bool newDataAvailable = false;
bool positionReached = false;
int positionData[8];
int idData[8];

void setup() {
	// TODO: Ping dynamixels at first

	// Init user defines
	debugMode = false;

	if (debugMode) Serial1.begin(9600);

	// Init dxl commandos
	DXL_COMMANDS.GOAL_POSITION = 30;
	DXL_COMMANDS.PRESENT_POSITION = 36;

	// Init usb connection
	SerialUSB.attachInterrupt(usbInterrupt);

	// Init dxl motor
	Dxl.begin(3);
	for (int i = 0; i < DXL_MOTOR_AMOUNT; i++) Dxl.jointMode(i + 1);

	// Init communication data
	positionRequest.bytes[0] = 36;
	positionRequest.bytes[1] = 37;

	pinMode(BOARD_LED_PIN, OUTPUT);
}

//USB max packet data is maximum 64byte, so nCount can not exceeds 64 bytes
void usbInterrupt(byte* buffer, byte nCount) {
	byte controlData[64];

	// Read everything from stream
	for (unsigned int i = 0; i < nCount; i++) controlData[i] = buffer[i];

	// Extract start message from stream
	DXL_MSG msgStart;
	for (int i = 0; i < 4; i++) msgStart.bytes[i] = controlData[i];

	if (msgStart.dxl_msg == 9999) {

		newDataAvailable = true;
		transmissionIsActive = true;

		// Extract ids from stream
		DXL_MSG ids;
		for (int i = 0; i < 4; i++) ids.bytes[i] = controlData[i + 4];
		for (size_t i = 0; i < 8; i++) {
			if ((ids.dxl_msg & (1 << i)) > 0) idData[i] = 1;
			else idData[i] = 0;
		}

		// Extract speed from stream
		DXL_MSG speed;
		for (int i = 0; i < 4; i++) speed.bytes[i] = controlData[i + 8];

		// Extract positions from stream
		DXL_MSG position;
		for (int i = 0; i < 4; i++) position.bytes[i] = controlData[i + 12];
		positionData[0] = position.dxl_msg;

		int cntr = 0;
		while (true) {
			for (int i = 0; i < 4; i++) position.bytes[i] = controlData[i + 16 + (cntr * 4)];
			// Stop reading position when end of message is reached
			if (position.dxl_msg == 8888) break;
			else {
				positionData[cntr + 1] = position.dxl_msg;
				cntr++;
			}
		}

		// Sending data to dynamixels
		for (int i = 0; i < 8; i++) if (idData[i] == 1) {
			Dxl.setPosition(i + 1, positionData[i], speed.dxl_msg);
		}
		transmissionIsActive = false;
	}
}

void loop() {
	// Check if new position data is available
	if (newDataAvailable) {
		// Check if transmission is active
		if (!transmissionIsActive) {
			// Check if current position of all dynamixel is in soll position
			for (int i = 0; i < 8; i++) {
				if (idData[i] == 1) {
					int tempPos = Dxl.readWord(i + 1, DXL_COMMANDS.PRESENT_POSITION);
					if (!((tempPos >= (positionData[i] - 5)) && (tempPos <= (positionData[i] + 5)))) {
						positionReached = false;
						break;
					}
					else {
						positionReached = true;
					}
				}
			}
			if (positionReached)
			{
				SerialUSB.write(10);
				newDataAvailable = false;
				positionReached = false;
			}
		}
	}
}

