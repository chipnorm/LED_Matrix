// FILE: ChipNorm_LED_Matrix.h
// VERSION: 1.0.0
//
// ChipNorm invests time and resources providing this open code,
// please support ChipNorm by purchasing products from ChipNorm.
//
// Written by Enrique Fernandez for ChipNorm by FMH.
//
// Copyright   Copyright (c) 2025 Enrique Fernandez - ChipNorm by FMH
// See the LICENSE file for details.

#ifndef CHIPNORM_LED_MATRIX_H
#define CHIPNORM_LED_MATRIX_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <SPI.h>  
#include <array>
#include <utility>  

constexpr size_t MATRIX_COLUMNS = 4;
constexpr size_t MATRIX_ROWS = 1;

using byte = uint8_t;

class ByteBlock {
private:
    byte rows[8];
public:
    ByteBlock() { for (byte& r : rows) r = 0; }  
    
    ByteBlock(std::initializer_list<byte> list) {
        size_t i = 0;
        for (byte val : list) {
            if (i < 8) rows[i++] = val;
        }
        for (; i < 8; ++i) rows[i] = 0;
    }
    byte& operator[](size_t index) { return rows[index]; }
    const byte& operator[](size_t index) const { return rows[index]; }
};


class MAX72XX_Helper {
private:
    MAX72XX_Helper() {}  
public:
    static const unsigned int OP_NOOP = 0;
    static const unsigned int OP_DIGIT0 = 1;
    static const unsigned int OP_INTENSITY = 10;
    static const unsigned int OP_SHUTDOWN = 12;

    static ByteBlock getCharBlock(char c) noexcept;  
};


class ChipNorm_LED_Matrix {
protected:
    ByteBlock LedStates[MATRIX_COLUMNS * MATRIX_ROWS];
    byte spidata[MATRIX_COLUMNS * MATRIX_ROWS * 2];
    
    unsigned int SPI_MOSI;
    unsigned int SPI_CLK;
    unsigned int SPI_CS;
    unsigned int IntensityLevel = 7;
    unsigned long spiTransferSpeed = 10000000;

    void transferData(byte opcode, byte data);
    void transferDataAll(const byte *data, unsigned int len);
    void spiTransfer(unsigned int segment, byte opcode, byte data);
    void initSPI();
    void initConf();
    void resetBuffers();

public:
    ChipNorm_LED_Matrix(unsigned int mosiPin, unsigned int clkPin, unsigned int csPin);

    void setBrightness(unsigned int level);
    void clear();
    void update();
    void displayOnSegment(unsigned int segmentindex, ByteBlock data);

    void write(char c);
    void write_manuell(unsigned int segmentindex, unsigned int digit, byte value);
    
    void moveLeft();
    void insertNextCharBit(char c, int bitIndex);

    void scroll_text(const char* scrollText, int segmentCount, int repetitions, int scrollDelay = 50);  
};


inline ChipNorm_LED_Matrix::ChipNorm_LED_Matrix(unsigned int mosiPin, unsigned int clkPin, unsigned int csPin)  
    : SPI_MOSI(mosiPin), SPI_CLK(clkPin), SPI_CS(csPin) {
    resetBuffers();
    initConf();  
}

inline void ChipNorm_LED_Matrix::transferData(byte opcode, byte data) {
    digitalWrite(SPI_CS, LOW);
    SPI.beginTransaction(SPISettings(spiTransferSpeed, MSBFIRST, SPI_MODE0));
    SPI.transfer(opcode);
    SPI.transfer(data);
    SPI.endTransaction();
    digitalWrite(SPI_CS, HIGH);
}

inline void ChipNorm_LED_Matrix::transferDataAll(const byte *data, unsigned int len) {
    digitalWrite(SPI_CS, LOW);
    SPI.beginTransaction(SPISettings(spiTransferSpeed, MSBFIRST, SPI_MODE0));
    for(unsigned int i = 0; i < len; ++i) {
        SPI.transfer(data[i]);
    }
    SPI.endTransaction();
    digitalWrite(SPI_CS, HIGH);
}

inline void ChipNorm_LED_Matrix::spiTransfer(unsigned int segment, byte opcode, byte data) {
    const size_t targetIndex = (MATRIX_COLUMNS - 1 - segment) * 2;
    spidata[targetIndex] = opcode;
    spidata[targetIndex + 1] = data;
    transferDataAll(spidata, MATRIX_COLUMNS * 2);  
}

inline void ChipNorm_LED_Matrix::resetBuffers() {
    for (size_t i = 0; i < MATRIX_COLUMNS * MATRIX_ROWS; ++i) {
        LedStates[i] = ByteBlock();
    }
    for (size_t i = 0; i < MATRIX_COLUMNS * MATRIX_ROWS * 2; ++i) {
        spidata[i] = 0;
    }
}

inline void ChipNorm_LED_Matrix::initSPI() {
    pinMode(SPI_CS, OUTPUT);
    digitalWrite(SPI_CS, HIGH);
    
    SPI.begin(SPI_CLK, -1, SPI_MOSI, -1);  
}

inline void ChipNorm_LED_Matrix::initConf() {
    initSPI();
    
    for(unsigned int i = 0; i < MATRIX_COLUMNS; i++) {
        spiTransfer(i, 11, 7); 
        spiTransfer(i, 9, 0);   
        spiTransfer(i, 12, 1); 
        spiTransfer(i, 15, 0); 
        
        spiTransfer(i, MAX72XX_Helper::OP_INTENSITY, IntensityLevel);  
    }
    clear();
}

inline void ChipNorm_LED_Matrix::setBrightness(unsigned int level) {
    IntensityLevel = level > 15 ? 15 : level;
    for(unsigned int i = 0; i < MATRIX_COLUMNS; i++) {
        spiTransfer(i, MAX72XX_Helper::OP_INTENSITY, IntensityLevel);
    }
}

inline void ChipNorm_LED_Matrix::clear() {
    for (size_t i = 0; i < MATRIX_COLUMNS; i++) {
        LedStates[i] = ByteBlock();  
        for (unsigned int digit = 0; digit < 8; ++digit) {
            spiTransfer(i, MAX72XX_Helper::OP_DIGIT0 + digit, 0x00);  
        }
    }
}

inline void ChipNorm_LED_Matrix::update() {
    for (size_t segmentindex = 0; segmentindex < MATRIX_COLUMNS; ++segmentindex) {
        const ByteBlock& data = LedStates[segmentindex];
        for (unsigned int digit = 0; digit < 8; ++digit) {
            spiTransfer(segmentindex, MAX72XX_Helper::OP_DIGIT0 + digit, data[digit]);
        }
    }
}

inline void ChipNorm_LED_Matrix::displayOnSegment(unsigned int segmentindex, ByteBlock data) {
    if (segmentindex >= MATRIX_COLUMNS) return;
    LedStates[segmentindex] = data;  
    update();
}

inline void ChipNorm_LED_Matrix::write(char c) {
    ByteBlock char_data = MAX72XX_Helper::getCharBlock(c);
    LedStates[0] = char_data;  
    update();
}

inline void ChipNorm_LED_Matrix::write_manuell(unsigned int segmentindex, unsigned int digit, byte value) {
    if (segmentindex >= MATRIX_COLUMNS || digit >= 8) return;
    LedStates[segmentindex][digit] = value;
    spiTransfer(segmentindex, MAX72XX_Helper::OP_DIGIT0 + digit, value);
}

inline void ChipNorm_LED_Matrix::moveLeft() {
    for (int row = 0; row < 8; ++row) {
        byte incomingBit = 0x00;  
        
        for (int segment = MATRIX_COLUMNS - 1; segment >= 0; --segment) {
            
            byte& currentRowByte = LedStates[segment][row];
            
            byte bitShiftedOut = currentRowByte & 0b10000000;
            
            currentRowByte <<= 1;
            
            if (incomingBit) {
                currentRowByte |= 0b00000001;  
            } else {
                currentRowByte &= ~0b00000001;  
            }
            
            incomingBit = bitShiftedOut;
        }
    }
}

inline void ChipNorm_LED_Matrix::insertNextCharBit(char c, int bitIndex) {
    
    ByteBlock charBlock = MAX72XX_Helper::getCharBlock(c);
    
    for (int digit = 0; digit < 8; ++digit) {
        byte rowData = charBlock[digit];
        
        byte newBit = (rowData >> (8 - 1 - bitIndex)) & 0x01;  

        byte& rightmostRow = LedStates[MATRIX_COLUMNS - 1][digit];
        
        if (newBit) {
            rightmostRow |= 0b00000001;  
        } else {
            rightmostRow &= ~0b00000001;  
        }
    }
}

inline void ChipNorm_LED_Matrix::scroll_text(const char* scrollText, int segmentCount, int repetitions, int scrollDelay) {
    
    if (segmentCount > MATRIX_COLUMNS) {
        segmentCount = MATRIX_COLUMNS;
    }

    const int CHAR_WIDTH = 8;
    size_t textLength = strlen(scrollText);
    
    int pixelsPerRepetition = textLength * CHAR_WIDTH;
    int totalScrollPixels = pixelsPerRepetition * repetitions;
    
    int pixelCounter = 0;  
    size_t textIndex = 0;

    clear();
    update();

    for (int i = 0; i < totalScrollPixels; ++i) {
        
        moveLeft();
        
        char currentChar = scrollText[textIndex];
        insertNextCharBit(currentChar, pixelCounter);
        
        update();
        delay(scrollDelay);  

        pixelCounter++;
        if (pixelCounter >= CHAR_WIDTH) {
            pixelCounter = 0;
            textIndex++;
            if (textIndex >= textLength) {
                textIndex = 0;  
            }
        }
    }
}

#endif // CHIPNORM_LED_MATRIX_H