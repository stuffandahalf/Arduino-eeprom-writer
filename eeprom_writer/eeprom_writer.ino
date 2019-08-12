// SERIAL = 0xC000
// EEPROM is hardwired for 0xE000 = 0x0000

#define ADDR0   53
#define ADDR1   51
#define ADDR2   49
#define ADDR3   47
#define ADDR4   45
#define ADDR5   43
#define ADDR6   41
#define ADDR7   39
#define ADDR8   37
#define ADDR9   35
#define ADDR10  33
#define ADDR11  31
#define ADDR12  29
#define ADDR13  27
#define ADDR14  25

#define IO0     52
#define IO1     50
#define IO2     48
#define IO3     46
#define IO4     44
#define IO5     42
#define IO6     40
#define IO7     38

#define CE      24
#define WE      26
#define OE      22

#define ENABLE  LOW
#define DISABLE HIGH

#define BUFFER_SIZE 64

template <typename T>
class Bus {
private:
    int pinCount;
    uint8_t *pins;
    
public:
    Bus(uint8_t *pins) : Bus(0, pins) { }

    Bus(unsigned int pinCount, uint8_t *pins) {
        if (pinCount > 0 && pinCount < (sizeof(T) * 8)) {
            this->pinCount = pinCount;
        }
        else {
            this->pinCount = sizeof(T) * 8;
        }
        
        this->pins = new uint8_t[this->pinCount];
        for (int i = 0; i < this->pinCount; i++) {
            this->pins[i] = pins[i];
        }
        this->set_mode(INPUT_PULLUP);
    }
    
    ~Bus() {
        delete[] this->pins;
    }
    
    void set_mode(byte mode) {
        for (int i = 0; i < this->pinCount; i++) {
            pinMode(this->pins[i], mode);
        }
    }
    
    void set(T data) {
        for (int i = 0; i < this->pinCount; i++) {
            pinMode(this->pins[i], OUTPUT);
            digitalWrite(this->pins[i], data & 1);
            data >>= 1;
        }
    }
    
    T get() {
        T data;
        for (int i = this->pinCount - 1; i >= 0; i--) {
            pinMode(this->pins[i], INPUT);
            data |= digitalRead(this->pins[i]);
            if (i) {
                data <<= 1;
            }
        }
        return data;
    }
};

uint8_t buffer[BUFFER_SIZE] = { 0 };

Bus<uint8_t> *dataBus;
Bus<uint16_t> *addressBus;

void write(uint16_t address, uint8_t data) {
    digitalWrite(OE, DISABLE);
    addressBus->set(address);
    digitalWrite(CE, ENABLE);
    digitalWrite(WE, ENABLE);
    dataBus->set(data);
    delayMicroseconds(2);
    digitalWrite(WE, DISABLE);
    digitalWrite(CE, DISABLE);
    dataBus->set_mode(INPUT_PULLUP);
}

uint8_t read(uint16_t address) {
    digitalWrite(WE, DISABLE);
    addressBus->set(address);
    digitalWrite(CE, ENABLE);
    digitalWrite(OE, ENABLE);
    uint8_t data = dataBus->get();
    //delay(5000);
    digitalWrite(OE, DISABLE);
    digitalWrite(CE, DISABLE);
    dataBus->set_mode(INPUT_PULLUP);
    return data;
}

inline void unlock() {
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0x80);
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0x20);
    //delayMicroseconds(10);
    //delay(10);
    delay(10);
}

void writeBuffer(uint16_t address, bool locked) {
    digitalWrite(OE, DISABLE);
    if (locked) {
        write(0x5555, 0xAA);
        write(0x2AAA, 0x55);
        write(0x5555, 0xA0);
        delayMicroseconds(2);
    }
    /*else {
        write(0x5555, 0xAA);
        write(0x2AAA, 0x55);
        write(0x5555, 0x80);
        write(0x5555, 0xAA);
        write(0x2AAA, 0x55);
        write(0x5555, 0x20);
    }*/
    //delayMicroseconds(2);
    
    for (int i = 0; i < BUFFER_SIZE; i++) {
        addressBus->set(address + i);
        digitalWrite(CE, ENABLE);
        digitalWrite(WE, ENABLE);
        dataBus->set(buffer[i]);
        delayMicroseconds(2);
        digitalWrite(WE, DISABLE);
        digitalWrite(CE, DISABLE);
        //delayMicroseconds(2);
        dataBus->set_mode(INPUT_PULLUP);
    }
    delay(10);
}

inline void enterProductID() {
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0x90);
    delay(10);
}

inline void exitProductID() {
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0xF0);
    delay(10);
}

void setup() {
    Serial.begin(115200);
    
    pinMode(CE, OUTPUT);
    digitalWrite(CE, DISABLE);
    
    pinMode(WE, OUTPUT);
    digitalWrite(WE, DISABLE);
    
    pinMode(OE, OUTPUT);
    digitalWrite(OE, DISABLE);
    
    uint8_t *pins = new uint8_t[8] { IO0, IO1, IO2, IO3, IO4, IO5, IO6, IO7 };
    dataBus = new Bus<uint8_t>(pins);
    delete pins;
    
    pins = new uint8_t[15] {
        ADDR0, ADDR1, ADDR2, ADDR3,
        ADDR4, ADDR5, ADDR6, ADDR7, 
        ADDR8, ADDR9, ADDR10, ADDR11,
        ADDR12, ADDR13, ADDR14 };
    addressBus = new Bus<uint16_t>(15, pins);
    delete pins;
    
    dataBus->set_mode(INPUT_PULLUP);
    
    buffer[0] = 0x55;
    Serial.println((int)buffer[0]);
    
    //unlock();
    writeBuffer(0x40, true);
    writeBuffer(0x40, true);
    //unlock();
    //writeBuffer(0, true);
    
    
    Serial.print("[0x");
    Serial.print(0, HEX);
    Serial.print("] = 0x");
    Serial.println((int)read(0), HEX);
    
    Serial.print("[0x");
    Serial.print(0x40, HEX);
    Serial.print("] = 0x");
    Serial.println((int)read(0x40), HEX);
    
    /*for (uint16_t i = 0; i < 0x8000; i++) {
        Serial.print("[0x");
        Serial.print(i, HEX);
        Serial.print("] = 0x");
        Serial.println((int)read(i), HEX);
    }*/
    
    /*unlockRom();
    for (int i = 0; i < BUFFER_SIZE; i++) {
        write(i, 0x55);
    }
    delay(1000);*/
    //write(0, 0x55);
    //exitProductID();
    //buffer[0] = 0x55;
    //writeBuffer(0, true);
    
    //Serial.println((int)read(0), HEX);
    /*delay(1000);
    Serial.println((int)read(0), HEX);
    Serial.println((int)read(1), HEX);
    Serial.println((int)read(2), HEX);*/
    
    //unlockRom();
    
    /*buffer[0] = 23;
    buffer[1] = 43;
    buffer[2] = 128;
    buffer[3] = 255;
    buffer[4] = 255;
    buffer[5] = 65;
    
    /*writeBuffer(0);
    
    for (int i = 0; i < 6; i++) {
        Serial.println((int)read(i));
    }*/
    /*write(345, 255);
    delay(1000);
    Serial.println((int)read(345));
    //addressBus->set(0x5555);
    //dataBus->set(0x55);*/
}

void loop() {
    
}
