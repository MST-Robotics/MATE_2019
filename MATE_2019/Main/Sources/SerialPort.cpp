#include "../Headers/SerialPort.h"

SerialPort::SerialPort()
{
  this->connected = false;
  this->handler = INVALID_HANDLE_VALUE;
}

SerialPort::~SerialPort() { closeSerialPort(); }

void SerialPort::openSerialPort(const char *portName, const int baudRate)
{
  this->connected = false;

  this->handler =
      CreateFileA(static_cast<LPCSTR>(portName), GENERIC_READ | GENERIC_WRITE,
                  0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (this->handler != INVALID_HANDLE_VALUE)
  {
    DCB dcbSerialParameters = {0};

    if (!GetCommState(this->handler, &dcbSerialParameters))
    {
      printf("failed to get current serial parameters");
    }
    else
    {
      dcbSerialParameters.BaudRate = baudRate;
      dcbSerialParameters.ByteSize = 8;
      dcbSerialParameters.StopBits = ONESTOPBIT;
      dcbSerialParameters.Parity = NOPARITY;
      dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

      if (!SetCommState(handler, &dcbSerialParameters))
      {
        printf("ALERT: could not set Serial port parameters\n");
      }
      else
      {
        this->connected = true;
        PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
      }
    }
  }
}

void SerialPort::closeSerialPort() 
{
  this->connected = false;
  CloseHandle(this->handler);
}

int SerialPort::readSerialPort(char *buffer, unsigned int buf_size)
{
  DWORD bytesRead;
  unsigned int toRead = 0;

  ClearCommError(this->handler, &this->errors, &this->status);

  if (this->status.cbInQue > 0)
  {
    if (this->status.cbInQue > buf_size)
    {
      toRead = buf_size;
    }
    else
    {
      toRead = this->status.cbInQue;
    }
  }

  if (ReadFile(this->handler, buffer, toRead, &bytesRead, NULL))
  {
    return bytesRead;
  }

  return 0;
}

bool SerialPort::writeSerialPort(char *buffer, unsigned int buf_size)
{
  DWORD bytesSend;

  if (!WriteFile(this->handler, (void *)buffer, buf_size, &bytesSend, 0))
  {
    ClearCommError(this->handler, &this->errors, &this->status);
    return false;
  }
  else
  {
    return true;
  }
}

bool SerialPort::isConnected() { return this->connected; }
