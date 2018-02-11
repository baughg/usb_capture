// USBDataCapture.cpp : Defines the entry point for the console application.
//
#define	_CRT_SECURE_NO_WARNINGS

/* ------------------------------------------------------------ */
/*					Include File Definitions					*/
/* ------------------------------------------------------------ */

#if defined(WIN32)

/* Include Windows specific headers here.
*/
#include <windows.h>

#else

/* Include Unix specific headers here.
*/

#endif

#include "stdafx.h"
#include "dpcutil.h"
#include <vector>
#include <time.h>

INT32 DeviceID;
char DefaultDeviceName[256];

bool DPC_Init();
bool DPC_GetRegRepeat( unsigned char Address, std::vector<unsigned char> &Data );
bool DPC_PutReg(unsigned char RegisterAddress, unsigned char RegisterData );
bool DPC_GetReg(unsigned char RegisterAddress, unsigned char* RegisterData );
void DumpToFile(char* file_name,std::vector<unsigned char> logicData);

int _tmain(int argc, _TCHAR* argv[])
{
  if(DPC_Init())
  {

    unsigned char Address = 0x00;
    const int capture_size = 128;
    //int NumBytes = capture_size*1024*1024;
    int NumBytes = 4;
    double mB = 1024.0*1024.0;

    std::vector<unsigned char> Data(NumBytes);
    unsigned char* DataPtr = &Data[0];
    std::vector<unsigned char> logicData;
    logicData.reserve(NumBytes);
    //DPC_PutReg(Address,0x0);
    //DPC_PutReg(0x1,0x20);
    //DPC_PutReg(0x2,0x18);
    clock_t init,final;
    init=clock();
    unsigned char sample = 0;
    unsigned char id = 0;

    //if(DPC_GetReg(2,&id))
    //{
    //  if(DPC_GetReg(0,&sample))
    //  {
    //  }
    //}
    //
    //return 0;

    if ( DPC_GetRegRepeat(Address, Data) )
    {
      final=clock()-init;
      std::vector<unsigned char> DataCopy(NumBytes);

      memcpy(&DataCopy[0],&Data[0],NumBytes);
      double total_time = (double)final / ((double)CLOCKS_PER_SEC);
      double mb = ((double)NumBytes / mB) / total_time;

      
      //StatusLine(false, "Register Repeat Read 0x{0:X2}: \r\n", Address);
      unsigned char seqMarker = Data[0];
      unsigned char _seqMarker = Data[0];
      unsigned char seqMarkerNext = seqMarker + 1;
      unsigned char _seqMarkerNext = seqMarker + 1;
      bool found_data = false;
      unsigned char dataByte = 0;
      unsigned char xorDataByte = 0;
      int startMarkerInx = 0;
      int startMarkerInxPrev = 0;
      int dist = 0;
      int distPrev = 0;
      bool bot_start_marker = false;
      int byteBuffer = 0;
      int byteBufferFound = 0;
      bool seqAndDataEqu = false;
      unsigned char lastDataByte = 0;

      unsigned char* newBytePtr = (unsigned char*)&byteBuffer;
      unsigned char* newByteFndPtr = (unsigned char*)&byteBufferFound;
      for ( int i=0; i<NumBytes; i++ )
      {
          *newBytePtr = Data[i];
          bool data_at_i = false;

          if(i >= 2)
          {
            _seqMarker = newBytePtr[2];
            _seqMarkerNext = newBytePtr[0];
            dataByte = newBytePtr[1];
            xorDataByte = dataByte ^ 0xAA;

            if(_seqMarker == seqMarker && 
              _seqMarkerNext == seqMarkerNext)
            {
              byteBufferFound = byteBuffer;
              data_at_i = true;
              found_data = true;
            }

            if(_seqMarker == seqMarker &&
              _seqMarkerNext == xorDataByte &&
              dataByte == seqMarker)
            {
              unsigned char seqMarkerNextNext = seqMarkerNext+1;
              int find_i = 0;
              bool foundSeqMarkerNext = false;
              bool foundMarker = false;
              int end_i = i+128;

              if(end_i > NumBytes)
                end_i = NumBytes;

              for(int _i = i+1; _i < end_i; _i++)
              {
                if(Data[_i] == seqMarkerNext)
                {
                  find_i = _i;
                  foundSeqMarkerNext = true;
                  foundMarker = true;
                  break;
                }
                else if(Data[_i] == seqMarkerNextNext)
                {
                  find_i = _i;
                  foundMarker = true;
                  break;
                }
              }
              
              seqAndDataEqu = true;

              if(foundMarker && foundSeqMarkerNext && find_i >= 2)
              {
                if(Data[find_i-2] == seqMarker)
                  seqAndDataEqu = false;
                else
                  byteBufferFound = byteBuffer;
              }
              else              
                byteBufferFound = byteBuffer;
            }
          }
          
          if(found_data && !data_at_i)
          {
            _seqMarker = newByteFndPtr[2];
            _seqMarkerNext = newByteFndPtr[0];
            dataByte = newByteFndPtr[1];
            seqMarker = _seqMarkerNext;
            seqMarkerNext++;
            found_data = false;
            logicData.push_back(dataByte);
            lastDataByte = dataByte;
          }

          if(seqAndDataEqu)
          {
            seqAndDataEqu = false;
             _seqMarker = newByteFndPtr[2];
            _seqMarkerNext = _seqMarker + 2;           
            dataByte = newByteFndPtr[1];
            seqMarker = _seqMarkerNext;
            seqMarkerNext += 2;
            *newBytePtr = seqMarker;
            
            logicData.push_back(dataByte);
            lastDataByte = dataByte;
          }

          size_t entry_count = logicData.size();

          if(entry_count >= 5847)
          {
              entry_count = 0;
          }
          byteBuffer <<= 8;
      } // decode

      //for ( int i=0; i<NumBytes; i++ )
      //{
      //  found_data = false;

      //  if(Data[i] == seqMarker)
      //    continue;
      //  else if(Data[i] == seqMarkerNext)
      //  {
      //    found_data = true;          
      //  }
      //  else if(Data[i] != seqMarker && Data[i] != seqMarkerNext)
      //  {
      //    found_data = true;
      //  }

      //  if(found_data)
      //  {
      //    dataByte = Data[i];
      //    seqMarker = seqMarkerNext;
      //    seqMarkerNext++;
      //    logicData.push_back(dataByte);
      //  }
      //  //printf("0x%02x,",(int)Data[i]);
      //  //printf("%d,",(int)Data[i]);
      //  //StatusLine(false, BitConverter.ToString(Data, i, Math.Min(16, NumBytes-i)));
      //} // decode
      DumpToFile("rawusb.dat",DataCopy);

      unsigned int _byteCount = (unsigned int)logicData.size();
      unsigned int byteCount = _byteCount;
      
      unsigned char* data_out_ptr = &logicData[0];
      FILE* capture_file = NULL;

      fopen_s(&capture_file,"capture.dat","wb");
      int remaining_bytes = (int)_byteCount;

      if(capture_file)
      {
        fwrite(&_byteCount,1,sizeof(_byteCount),capture_file);

        while(remaining_bytes > 0)
        {
          int write_len = 1024;

          if(write_len > remaining_bytes)
            write_len = remaining_bytes;

          fwrite(data_out_ptr,1,write_len,capture_file);
          data_out_ptr += write_len;
          remaining_bytes -= write_len;
        }

        fclose(capture_file);
      }

      if(byteCount > 1024)
        byteCount = 1024;

      for(unsigned int b = 0; b < byteCount; b++)
      {
        printf("0x%02x,",(int)logicData[b]);
      }

       printf("%u bytes captured.\n",_byteCount);
       printf("Data rate: %f MB/s\n",(float)mb);
    }
    else
    {
      //StatusLine(false, "Error accessing register 0x{0:X2}\r\n", Address);
    }
  }
  return 0;
}

void ReadFromFile(char* file_name,std::vector<unsigned char> &logicData)
{
  unsigned int _byteCount = (unsigned int)logicData.size();
  unsigned int byteCount = _byteCount;

  unsigned char* data_out_ptr = &logicData[0];
  FILE* capture_file = NULL;

  fopen_s(&capture_file,file_name,"rb");
  int remaining_bytes = (int)_byteCount;

  if(capture_file)
  {
    fread(&_byteCount,1,sizeof(_byteCount),capture_file);

    while(remaining_bytes > 0)
    {
      int write_len = 1024;

      if(write_len > remaining_bytes)
        write_len = remaining_bytes;

      fread(data_out_ptr,1,write_len,capture_file);
      data_out_ptr += write_len;
      remaining_bytes -= write_len;
    }

    fclose(capture_file);
  }
}


void DumpToFile(char* file_name,std::vector<unsigned char> logicData)
{
  unsigned int _byteCount = (unsigned int)logicData.size();
  unsigned int byteCount = _byteCount;

  unsigned char* data_out_ptr = &logicData[0];
  FILE* capture_file = NULL;

  fopen_s(&capture_file,file_name,"wb");
  int remaining_bytes = (int)_byteCount;

  if(capture_file)
  {
    fwrite(&_byteCount,1,sizeof(_byteCount),capture_file);

    while(remaining_bytes > 0)
    {
      int write_len = 1024;

      if(write_len > remaining_bytes)
        write_len = remaining_bytes;

      fwrite(data_out_ptr,1,write_len,capture_file);
      data_out_ptr += write_len;
      remaining_bytes -= write_len;
    }

    fclose(capture_file);
  }
}

bool DPC_Init()
{
#ifdef REPLAY
  return true;
#else
  ERC ErrorCode;


  if ( !DpcInit(&ErrorCode) )
  {
    printf("DPCInit FAILED!!!  Error Code = %d\n",(int)ErrorCode);
    //StatusLine(true, "DPCInit FAILED!!!  Error Code = {0}", ErrorCode);
    return false;
  }

  DeviceID = DvmgGetDefaultDev(&ErrorCode);
  if ( DeviceID == -1 )
  {
    //StatusLine(true, "No default device");
    printf("No default device\n");
    return false;
  }
  else
  {
    DvmgGetDevName(DeviceID, DefaultDeviceName,&ErrorCode);
    printf("Device ID = %d\nDevice name = %s\n",(int)DeviceID,DefaultDeviceName);
    //StatusLine(false, "Device ID = {0}", DeviceID);
    //StatusLine(true, "Device name = {0}", DefaultDeviceName);
    return true;
  }
#endif
}

bool DPC_GetRegRepeat( unsigned char Address, std::vector<unsigned char> &Data )
{
#ifdef REPLAY
  ReadFromFile("rawusb.dat",Data);
  return true;
#else
  if ( Data.size() == 0 )
    return false;

  ERC ErrorCode;
  HANDLE InterfaceHandle;
  bool Success = true;

  if ( !DpcOpenData(&InterfaceHandle, DefaultDeviceName,&ErrorCode, NULL) )
    return false;

  if ( !DpcGetRegRepeat(InterfaceHandle, Address, &Data[0], Data.size(), &ErrorCode,NULL) )
    Success = false;

  DpcCloseData(InterfaceHandle,&ErrorCode);
  return Success;
#endif
}

bool DPC_PutReg(unsigned char RegisterAddress, unsigned char RegisterData )
{
  ERC ErrorCode;
  HANDLE InterfaceHandle;
  bool Success = true;

  if ( !DpcOpenData(&InterfaceHandle, DefaultDeviceName,&ErrorCode, NULL) )
    return false;

  if ( !DpcPutReg(InterfaceHandle, RegisterAddress, RegisterData,&ErrorCode, NULL) )
    Success = false;

  DpcCloseData(InterfaceHandle, &ErrorCode);
  return Success;
}

bool DPC_GetReg(unsigned char RegisterAddress, unsigned char* RegisterData )
{
  ERC ErrorCode;
  HANDLE InterfaceHandle;
  bool Success = true;

  if ( !DpcOpenData(&InterfaceHandle, DefaultDeviceName,&ErrorCode, NULL) )
    return false;

  if ( !DpcGetReg(InterfaceHandle, RegisterAddress, RegisterData,&ErrorCode, NULL) )
    Success = false;

  DpcCloseData(InterfaceHandle, &ErrorCode);
  return Success;
}