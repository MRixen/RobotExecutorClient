#define lowByte(w) ((uint8) ((w) & 0xff))
#define highByte(w) ((uint8) ((w) >> 8))

struct dxl_commands
{
	int GOAL_POSITION;
	int PRESENT_POSITION;
};

union DXL_MSG
{
	short dxl_msg;
	byte bytes[2];
};

union DXL_MSG_RETURN
{
	short dxl_msg;
	byte bytes[2];
};
DXL_MSG_RETURN positionRequest;

int DXL_BUS_SERIAL = 3;
int MAX_DXL_MOTOR_AMOUNT = 8;

typedef struct dxl_commands Dxl_commands;
Dxl_commands DXL_COMMANDS;
Dynamixel Dxl(DXL_BUS_SERIAL);

bool transmissionIsActive = false;

bool debugMode;
byte isMoving = 0;
bool newDataAvailable = false;
bool positionReached = false;
int positionData[8];
int speedData[8];
int idData[8];
int MSG_BLOCK_SIZE = 2;
int dynamixelAmount;
int SMOOTH_ZONE = 3; // Zone to set position reached bit

bool sendSomething = false;
short dataArray[] = { 300, 0 };
int counter = 0;

void setup() {
	// TODO: Ping dynamixels at first

	// Init user defines
	debugMode = false;

	if (debugMode) Serial1.begin(115200);

	// Init dxl commandos
	DXL_COMMANDS.GOAL_POSITION = 30;
	DXL_COMMANDS.PRESENT_POSITION = 36;

	// Init usb connection
	SerialUSB.attachInterrupt(usbInterrupt);

	// Init dxl motor
	Dxl.begin(3);
	for (int i = 0; i < MAX_DXL_MOTOR_AMOUNT; i++) Dxl.jointMode(i + 1);

	// Init communication data
	positionRequest.bytes[0] = 36;
	positionRequest.bytes[1] = 37;

	pinMode(BOARD_LED_PIN, OUTPUT);
	digitalWrite(BOARD_LED_PIN, HIGH);

	for (size_t i = 0; i < 7; i++)
	{
		Dxl.setPosition(i+1, dataArray[1], 300);
	}
}

//USB max packet data is maximum 64byte, so nCount can not exceeds 64 bytes
void usbInterrupt(byte* buffer, byte nCount) {
	byte controlData[64];

	// Read everything from stream
	for (unsigned int i = 0; i < nCount; i++) controlData[i] = buffer[i];

	// Extract start message from stream
	DXL_MSG msgStart;
	for (int i = 0; i < MSG_BLOCK_SIZE; i++) msgStart.bytes[i] = controlData[i];

	if (msgStart.dxl_msg == 9999) {
	//if (true) {

		newDataAvailable = true;
		transmissionIsActive = true;

		// Extract ids from stream
		DXL_MSG ids;
		dynamixelAmount = 0;
		for (int i = 0; i < MSG_BLOCK_SIZE; i++) ids.bytes[i] = controlData[i + MSG_BLOCK_SIZE];
		for (size_t i = 0; i < 8; i++) {
			if ((ids.dxl_msg & (1 << i)) > 0) {
				idData[i] = 1;
				dynamixelAmount++;
			}
			else idData[i] = 0;
		}

		//for (size_t i = 0; i < dynamixelAmount; i++)
		//{
		//	digitalWrite(BOARD_LED_PIN, LOW);
		//	delay(250);
		//	digitalWrite(BOARD_LED_PIN, HIGH);
		//	delay(250);
		//}

		// Extract speed from stream
		DXL_MSG speed;
		for (int j = 0; j < dynamixelAmount; j++) {
			for (int i = 0; i < MSG_BLOCK_SIZE; i++) speed.bytes[i] = controlData[i + MSG_BLOCK_SIZE * 2 + (j * MSG_BLOCK_SIZE)];
			speedData[j] = speed.dxl_msg;
		}


		// Extract positions from stream
		DXL_MSG position;
		for (int j = 0; j < dynamixelAmount; j++) {
			for (int i = 0; i < MSG_BLOCK_SIZE; i++) position.bytes[i] = controlData[i + MSG_BLOCK_SIZE * 2 + (j + dynamixelAmount * MSG_BLOCK_SIZE)];
			positionData[j] = position.dxl_msg;
		}

		// Sending data to dynamixels
		for (int i = 0; i < dynamixelAmount; i++) Dxl.setPosition(i + 1, positionData[0], speedData[0]);
		//Dxl.setPosition(1, positionData[0], 50);

		transmissionIsActive = false;
	}
}

void loop() {
	int istPos;
	// Check if new position data is available
	if (newDataAvailable) {
		// Check if transmission is active
		if (!transmissionIsActive) {
			// Check if current position of all dynamixel is in soll position
			for (int i = 0; i < 1; i++) {
				istPos = Dxl.readWord(1, DXL_COMMANDS.PRESENT_POSITION);
				if (!((istPos >= (positionData[0] - SMOOTH_ZONE)) & (istPos <= (positionData[i] + SMOOTH_ZONE)))) {
					//SerialUSB.println(istPos);
					positionReached = false;
					break;
				}
				else {
					positionReached = true;
				}
			}
			if (positionReached)
			{
				positionReached = false;
				newDataAvailable = false;
				SerialUSB.write(1);
				//SerialUSB.println(1111);
				//counter++;
				//if (counter>1)  counter = 0;
			}
		}
	}
}

