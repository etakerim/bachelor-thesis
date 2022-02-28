#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <HardwareSerial.h>


#define LED             2

#define OPENLOG_VCC     25
#define OPENLOG_VCC1    26
#define OPENLOG_VCC2    27

#define OPENLOG_TX      13
#define OPENLOG_RX      5
#define OPEN_LOG_BAUDRATE   115200

#define TIMER_PRESCALER             80
#define SAMPLING_FREQ_HZ            500
#define SAMPLING_PERIOD_US          (1000000 / SAMPLING_FREQ_HZ)

#define IMU_DEN         18
#define IMU_SDO         23
#define IMU_SDA         19
#define IMU_SCL         21
#define IMU_CS_AG       22
#define IMU_CS_M        14
#define IMU_INT1_M      12
#define IMU_DRDY_M      4
#define IMU_INT2_AG     16
#define IMU_INT1_AG     17

#define SCK  IMU_SCL
#define MISO IMU_SDO
#define MOSI IMU_SDA
#define XGCS IMU_CS_AG
#define MCS  IMU_CS_M


HardwareSerial OpenLog(1);
Adafruit_LSM9DS1 imu = Adafruit_LSM9DS1(
    SCK, MISO, MOSI, XGCS, MCS
);
volatile bool sample = false;
hw_timer_t *timer = NULL;
portMUX_TYPE tm_lock = portMUX_INITIALIZER_UNLOCKED;
uint32_t t = 0;
uint32_t t_prev = 0;
char buffer[50];


void IRAM_ATTR tick()
{
   vTaskEnterCritical(&tm_lock);
   sample = true;
   vTaskExitCritical(&tm_lock);
}

void setup()
{
    // LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // OpenLog
    pinMode(OPENLOG_VCC, OUTPUT);
    pinMode(OPENLOG_VCC1, OUTPUT);
    pinMode(OPENLOG_VCC2, OUTPUT);
    gpio_set_drive_capability((gpio_num_t) OPENLOG_VCC, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability((gpio_num_t) OPENLOG_VCC1, GPIO_DRIVE_CAP_3);
    gpio_set_drive_capability((gpio_num_t) OPENLOG_VCC2, GPIO_DRIVE_CAP_3);
    REG_WRITE(GPIO_OUT_W1TC_REG, (7 << OPENLOG_VCC));

    // FTDI USB
    // Serial.begin(115200);

    // IMU -  Inertial measurement unit
    pinMode(IMU_DEN, INPUT);
    pinMode(IMU_SDO, INPUT);
    pinMode(IMU_SDA, INPUT);
    pinMode(IMU_SCL, INPUT);
    pinMode(IMU_CS_AG, INPUT);
    pinMode(IMU_CS_M, INPUT);
    pinMode(IMU_INT1_M, INPUT);
    pinMode(IMU_INT2_AG, INPUT);
    pinMode(IMU_INT2_AG, INPUT);
    pinMode(IMU_INT1_AG, INPUT);

    // OpenLog
    REG_WRITE(GPIO_OUT_W1TS_REG, (7 << OPENLOG_VCC));
    digitalWrite(OPENLOG_VCC, HIGH);
    delay(200);
    OpenLog.begin(OPEN_LOG_BAUDRATE, SERIAL_8N1, OPENLOG_RX, OPENLOG_TX);
    delay(10000);

    // IMU SPI Driver
    if (!imu.begin()) {
        Serial.println("Can't start IMU");
        while (1);
    }
    imu.setupAccel(imu.LSM9DS1_ACCELRANGE_2G); // 2G, 4G, 8G, 16G

    timer = timerBegin(0, TIMER_PRESCALER, true);
    timerAlarmWrite(timer, SAMPLING_PERIOD_US, true);
    timerAttachInterrupt(timer, &tick, true);        
    timerAlarmEnable(timer);

    digitalWrite(LED, HIGH);
    OpenLog.println("A_X[LSB] A_Y[LSB] A_Z[LSB]");
}

void loop()
{
    if (sample) {
        vTaskEnterCritical(&tm_lock);
        sample = false;
        vTaskExitCritical(&tm_lock);

        imu.readAccel();
        sprintf(
            buffer, "%d %d %d\n",
            imu.accelData.x, 
            imu.accelData.y, 
            imu.accelData.z
        );
        OpenLog.print(buffer);
        OpenLog.flush();
        t_prev = t;
    }
}

/*
2G 0.061  (2000 mg/(2^16 / 2))
4G 0.122
8G 0.244
16G 0.732

pinMode(BUTTON_PIN, INPUT_PULLUP);
attachInterrupt(BUTTON_PIN, isr, FALLING);  // LOW, HIGH, CHANGE, FALLING, RISSING
void IRAM_ATTR isr() { }
detachInterrupt(GPIOPin);

*/
