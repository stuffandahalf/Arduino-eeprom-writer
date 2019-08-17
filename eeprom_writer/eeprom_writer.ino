// SERIAL = 0xC000
// EEPROM is hardwired for 0xE000 = 0x0000

#define LED     13

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
        T data = 0;
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

enum class Protocol : uint8_t {
    FAIL,
    END,
    READ,
    WRITE,
    DUMP
};

uint8_t buffer[BUFFER_SIZE] = { 0 };

Bus<uint8_t> *dataBus;
Bus<uint16_t> *addressBus;

inline void blink() {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
}

inline void write(uint16_t address, uint8_t data) {
    digitalWrite(OE, DISABLE);
    addressBus->set(address);
    digitalWrite(CE, ENABLE);
    digitalWrite(WE, ENABLE);
    dataBus->set(data);
    delayMicroseconds(2);
    digitalWrite(WE, DISABLE);
    digitalWrite(CE, DISABLE);
    delayMicroseconds(2);
    //dataBus->set_mode(INPUT_PULLUP);
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
    //delay(10);
}

void writeBuffer(uint16_t address, bool locked) {
    digitalWrite(OE, DISABLE);
    if (locked) {
        write(0x5555, 0xAA);
        write(0x2AAA, 0x55);
        write(0x5555, 0xA0);
        //delayMicroseconds(2);
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
        dataBus->set(buffer[i]);
        digitalWrite(WE, ENABLE);
        delayMicroseconds(2);
        digitalWrite(WE, DISABLE);
        digitalWrite(CE, DISABLE);
        //delayMicroseconds(2);
        //dataBus->set_mode(INPUT_PULLUP);
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

inline void erase() {
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0x80);
    write(0x5555, 0xAA);
    write(0x2AAA, 0x55);
    write(0x5555, 0x10);
    
    delay(50);
}

template <typename T>
inline T receiveData() {
    T data = 0;
    for (int i = 0; i < sizeof(T); i++) {
        if (i) {
            data <<= 8;
        }
        while (!Serial.available());
        data |= (uint8_t)Serial.read();
    }
    
    return data;
}

void setup() {
    Serial.begin(115200);
    
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    
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
    //delay(2000);
    
    Serial.println("Ready");
}

void loop() {
    uint16_t address = 0;
    uint8_t data = 0;
    
    if (Serial.available()) {
        Protocol command = (Protocol)Serial.read();
        
        switch (command) {
        case Protocol::READ:
            address = receiveData<uint16_t>();
            data = read(address);
            Serial.print((char)data);
            Serial.print((char)Protocol::END);
            
            break;
        case Protocol::WRITE:
            address = receiveData<uint16_t>();
            for (int i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = receiveData<uint8_t>();
            }
            writeBuffer(address, false);
            break;
        default:
            Serial.print("Received ");
            Serial.println((int)command);
            break;
        }
    }
}
