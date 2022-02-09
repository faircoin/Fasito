/*
 * Serial.h
 *
 *  Created on: 23.04.2017
 *      Author: thokon00
 */

#include <string.h>
#include <stdio.h>

#ifndef SERIAL_H_
#define SERIAL_H_

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0

class SerialEMU
{
  public:
    SerialEMU() : write_error(0) {}
    void begin(long x) {};
    int available() { return 1; };
    int read() { return getchar(); };
    size_t write(uint8_t n);
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *str)           { return write((const uint8_t *)str, strlen(str)); }
    size_t write(const char *buffer, size_t size)   { return write((const uint8_t *)buffer, size); }
    size_t print(char c)                { return write((uint8_t)c); }
    size_t print(const char s[])            { return write(s); }

    size_t print(uint8_t b)             { return printNumber(b, 10, 0); }
    size_t print(int n)             { return print((long)n); }
    size_t print(unsigned int n)            { return printNumber(n, 10, 0); }
    size_t print(long n);
    size_t print(unsigned long n)           { return printNumber(n, 10, 0); }

    size_t print(unsigned char n, int base)     { return printNumber(n, base, 0); }
    size_t print(int n, int base)           { return (base == 10) ? print(n) : printNumber(n, base, 0); }
    size_t print(unsigned int n, int base)      { return printNumber(n, base, 0); }
    size_t print(long n, int base)          { return (base == 10) ? print(n) : printNumber(n, base, 0); }
    size_t print(unsigned long n, int base)     { return printNumber(n, base, 0); }

    size_t print(double n, int digits = 2)      { return printFloat(n, digits); }
    size_t println(void);
    size_t println(char c)              { return print(c) + println(); }
    size_t println(const char s[])          { return print(s) + println(); }

    void flush() { fflush(stdout); }

    size_t println(uint8_t b)           { return print(b) + println(); }
    size_t println(int n)               { return print(n) + println(); }
    size_t println(unsigned int n)          { return print(n) + println(); }
    size_t println(long n)              { return print(n) + println(); }
    size_t println(unsigned long n)         { return print(n) + println(); }

    size_t println(unsigned char n, int base)   { return print(n, base) + println(); }
    size_t println(int n, int base)         { return print(n, base) + println(); }
    size_t println(unsigned int n, int base)    { return print(n, base) + println(); }
    size_t println(long n, int base)        { return print(n, base) + println(); }
    size_t println(unsigned long n, int base)   { return print(n, base) + println(); }

    size_t println(double n, int digits = 2)    { return print(n, digits) + println(); }
    int getWriteError() { return write_error; }
    void clearWriteError() { setWriteError(0); }
    int printf(const char *format, ...);
  protected:
    void setWriteError(int err = 1) { write_error = err; }
  private:
    char write_error;
    size_t printFloat(double n, uint8_t digits);
    size_t printNumber(unsigned long n, uint8_t base, uint8_t sign);
};

#endif /* SERIAL_H_ */
