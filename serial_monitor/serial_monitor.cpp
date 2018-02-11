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
#include <windows.h>
#include <stdio.h>

INT32 DeviceID;
char DefaultDeviceName[256];

bool DPC_Init();
bool DPC_GetRegRepeat( unsigned char Address, std::vector<unsigned char> &Data );
bool DPC_PutReg(unsigned char RegisterAddress, unsigned char RegisterData );
bool DPC_GetReg(unsigned char RegisterAddress, unsigned char* RegisterData );
void DumpToFile(char* file_name,std::vector<unsigned char> logicData);

HWND GetConsoleHwnd(void)
   {
       #define MY_BUFSIZE 1024 // Buffer size for console window titles.
       HWND hwndFound;         // This is what is returned to the caller.
       char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
                                           // WindowTitle.
       char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
                                           // WindowTitle.

       // Fetch current window title.

       GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

       // Format a "unique" NewWindowTitle.

       wsprintf(pszNewWindowTitle,"%d/%d",
                   GetTickCount(),
                   GetCurrentProcessId());

       // Change current window title.

       SetConsoleTitle(pszNewWindowTitle);

       // Ensure window title has been updated.

       Sleep(40);

       // Look for NewWindowTitle.

       hwndFound=FindWindow(NULL, pszNewWindowTitle);

       // Restore original window title.

       SetConsoleTitle(pszOldWindowTitle);

       return(hwndFound);
   }

int _tmain(int argc, _TCHAR* argv[])
{
  if(DPC_Init())
  {

    unsigned char Address = 0x00;
    const int capture_size = 128;
    //int NumBytes = capture_size*1024*1024;
    int NumBytes = 130;
    double mB = 1024.0*1024.0;

    std::vector<unsigned char> Data(NumBytes);
    unsigned char* DataPtr = &Data[0];
    std::vector<unsigned char> logicData;
    logicData.reserve(NumBytes);
    //DPC_PutReg(Address,0x0);
    //DPC_PutReg(0x1,0x20);
    //DPC_PutReg(0x2,0x18);
    clock_t init,final;

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
    ERC ErrorCode;
    HANDLE InterfaceHandle;
    bool Success = true;
    const size_t data_read_size = Data.size();
    TRID trans_id = 0;
    //HWND hwnd = GetConsoleHwnd();
    //DvmgStartConfigureDevices(hwnd,&ErrorCode);

    if ( !DpcOpenData(&InterfaceHandle, DefaultDeviceName,&ErrorCode, &trans_id) ) {
      printf("DpcOpenData error!\n");
      return 1;
    }
    
    

    init=clock();
    unsigned iter = 0;
    char rx_data[2];
    rx_data[1] = 0;
    unsigned rx_count = 0;
    unsigned last_loc = 0, last_loc_next = 0;
    std::vector<unsigned char> capture_data;
    const int capture_size_bytes = 1024*32;

    capture_data.reserve(capture_size_bytes);
    unsigned loc = 0;
    unsigned loc1 = 0;
    bool run_loop = true;
    //for(int i = 0; i < 1000000; ++i){
    while(run_loop){
      if ( !DpcGetRegRepeat(InterfaceHandle, Address, &Data[0], data_read_size, &ErrorCode,&trans_id) ) {
        printf("DpcGetRegRepeat error!\n");
        break;
      }
      
    
      last_loc_next = (last_loc - 2) % 128;

      for(unsigned b = 0; b < 128; b += 2)
      {
        loc = (last_loc + b) % 128;
        loc1 = (loc + 1) % 128;

        if(Data[loc] > 0)
        {
          last_loc_next = loc;
          capture_data.push_back(Data[loc1]); 
          rx_count++;

          if(rx_count >= capture_size_bytes)
            run_loop = false;
        }
      }

      last_loc = (last_loc_next + 2) % 128;
      iter++;
    }
    final=clock()-init;
    double total_time = (double)final / ((double)CLOCKS_PER_SEC);
    double bps = ((double)(iter * (unsigned)data_read_size)) / total_time;
    DpcCloseData(InterfaceHandle,&ErrorCode);
    unsigned char* p_data = &capture_data[0];
    FILE* dump_file = NULL;
    fopen_s(&dump_file,"pv2_gps.dat","wb");
    fwrite(p_data,1,capture_data.size(),dump_file);
    fclose(dump_file);
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