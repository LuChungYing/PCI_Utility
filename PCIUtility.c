/** @file
//;******************************************************************************
//;* Copyright (c) 1983-2021, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;
//; Abstract:
//;  To list all pci device, but only for single host bus. And function to modify
//;  its configeration space.
//;------------------------------------------------------------------------------
//;
//; $Log: $
//;
//; Revision History:
//;*
//;******************************************************************************
  This sample application is to find all pci device and read or write the configration
  space.
  
  Copyright (c) 2006 - 2021, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/
#include "Pciheader.h"

PCIdevInfo            gPcidev[256*32*8];
UINT32                gNumPcidev = 0;
UINT32                gMODE = 0;
UINT32                gCurrentDev;
UINT32                gcursorrow=3;
UINT8                 gES1=0;

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  
  BOOLEAN                          Exit;
  EFI_STATUS                       Status;
  EFI_INPUT_KEY                    Key;
  Exit = FALSE;  
  
  DoPciDeviceinfo (ImageHandle, &SystemTable);
  
  ShowScreen0 ();
  
  Status = gST->ConOut->SetCursorPosition (
                gST->ConOut, 
                0, 
                gcursorrow
                );
  if (EFI_ERROR (Status)) {
      Print (L"SetCursor Error\n");
      return Status;
    }

  Status = gST->ConOut->EnableCursor(
                gST->ConOut,
                TRUE
                );
  if (EFI_ERROR (Status)) {
    Print (L"EnableCursor Error\n");
    return Status;
  }
  
  while (!Exit) {
    Status = GetKey (&Key);
    if (!EFI_ERROR (Status)) {
      Exit = ShowPressedKey (Key);
    }
  }
  
  gST->ConOut->ClearScreen (gST->ConOut);


  return EFI_SUCCESS;

}

/**
 Receive the key by console stdio.

 @param[out] Key          A pointer to a key by keyboard
 
 @retval EFI_SUCCESS            The operation completed successfully.
 @retval EFI_INVALID_PARAMETER  Time is NULL.
 @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
**/

EFI_STATUS
GetKey (
  OUT  EFI_INPUT_KEY  *Key
  )
{
  EFI_STATUS  Status;

  //
  // Wait for key pressed by using event.
  //
  gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, NULL);
  Status = gST->ConIn->ReadKeyStroke (
             gST->ConIn,
             Key
             );
  return Status;
}

/**
 Handle the key.

 @param[IN] Key          A pointer to a key by keyboard, to do something for the key.
 
 @retval EFI_SUCCESS            The operation completed successfully.
 @retval EFI_INVALID_PARAMETER  Time is NULL.
 @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
**/

BOOLEAN
ShowPressedKey (
  IN  EFI_INPUT_KEY  Key
  )
{
  UINT32 i;
	Print (L"\n");
  switch (Key.UnicodeChar) {
    
  case CHAR_CARRIAGE_RETURN:
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
    
    for (i=0; i < 65536; i++){
      if (gPcidev[i].NumOfList == gcursorrow-2){
        gCurrentDev = i;
        break;
      }
    }
    ShowScreen1 (i,gMODE);
    gES1 = 1;
    break;
    
  default:
    break;
  }

  switch (Key.ScanCode) {

  case SCAN_UP:
    //
    // Print light red color enter.
    //
    if (gES1 == 1){
      gST->ConOut->SetCursorPosition (gST->ConOut, 0, 21);
      break;
    }
	  if (gcursorrow>3){
	    gcursorrow--;
	  }
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, gcursorrow);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTRED);
    break;

  case SCAN_DOWN:
    //
    // Print light cyan color enter.
    //
    if (gES1 == 1){
      gST->ConOut->SetCursorPosition (gST->ConOut, 0, 21);
      break;
    }
    if (gcursorrow < (gNumPcidev + 2)){
	    gcursorrow++;
	  }
    gST->ConOut->SetCursorPosition (gST->ConOut, 0, gcursorrow);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTCYAN);
    
    break;
    
  case SCAN_F1:
    gES1 = 0;
    ShowScreen0 ();
    break;
    
  case SCAN_F2:
    if(gMODE < 2)
      gMODE++;
    else
      gMODE = 0;
    ShowScreen1 (gCurrentDev,gMODE);
    break;
    
  case SCAN_F3:
    ModifyPCIconfigspace(gCurrentDev,gMODE);
    ShowScreen1 (gCurrentDev,gMODE);
    break;
    
  case SCAN_ESC:
    Print (L"Esc\n");
    return TRUE;
    break;

  default:
    if (gES1 == 1){
      gST->ConOut->SetCursorPosition (gST->ConOut, 0, 21);
      break;
    }
    else
      gST->ConOut->SetCursorPosition (gST->ConOut, 0, gcursorrow);
    break;
  }
  gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY);

  return FALSE;
}

/**
 Print Screen1, print its config space.

 @param[IN] index         Index for which device want to print its config space.
 @param[IN] mode          For Byte, Word, Dword.
 
 @retval EFI_SUCCESS            The operation completed successfully.
 @retval EFI_INVALID_PARAMETER  Time is NULL.
 @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
**/

UINT32
ShowScreen1(
  IN UINT32 index,
  IN UINT32 mode
  )
{
  UINT32 i;
  
  gST->ConOut->ClearScreen (gST->ConOut);
  
  if (mode == 0){
    Print (L"   ");
    for (i=0;i<16;i++){
      Print (L"%2X ",i);
    }
      for (i=0; i<256; i++){
            if (i%16 == 0)
              Print (L"\n%X0 ",i/16);
            Print (L"%2X",gPcidev[index].ConfigSpaceReg[i]);
              Print(L" ");
      }
  } else if (mode == 1){
    Print (L"   ");
    for(i=0;i<16;i++){
      if (i % 2 == 1)
        Print (L"%2X ",i-1);
      else
        Print (L"%2X",i+1);
    }
          for (i=0; i<256; i++){
              if (i % 16 == 0)
                Print (L"\n%X0 ",i/16);
              if (i % 2 == 1)
                Print (L"%2X",gPcidev[index].ConfigSpaceReg[i-1]);
              else
                Print (L"%2X",gPcidev[index].ConfigSpaceReg[i+1]);
              if (i%2 == 1)
              Print(L" ");
          }
  } else {
    Print(L"   ");
    for(i=0;i<16;i++){
      if (i % 4 == 1)
        Print(L"%2X",i+1);
      else if (i % 4 == 2)
        Print(L"%2X",i-1);
      else if (i % 4 == 3)
        Print(L"%2X ",i-3);
      else
        Print(L"%2X",i+3);
    }
          for (i=0; i<256; i++){
              if (i % 16 == 0)
                Print(L"\n%X0 ",i/16);
              if (i % 4 == 1)
                Print(L"%2X",gPcidev[index].ConfigSpaceReg[i+1]);
              else if (i % 4 == 2)
                Print(L"%2X",gPcidev[index].ConfigSpaceReg[i-1]);
              else if (i % 4 == 3)
                Print(L"%2X",gPcidev[index].ConfigSpaceReg[i-3]);
              else
                Print(L"%2X",gPcidev[index].ConfigSpaceReg[i+3]);
              if (i%4 == 3)
                Print(L" ");
          }
  }
  Print (L"\n");
  Print (L"Press [F1] to List All PCI Devices.\n");
  
  Print (L"Press [F2] to Switch Operation Mode Between Byte/Word/Dword.\n");
  
  Print (L"Press [F3] to Change Current Display PCI Device Register Value..\n");
  
  Print (L"Press [Esc] to Quit.\n");
  gST->ConOut->SetCursorPosition (gST->ConOut, 0, 21);
  return 0;
}

/**
 Print Screen0, list all pci devices.
 
 @retval EFI_SUCCESS            The operation completed successfully.
 @retval EFI_INVALID_PARAMETER  Time is NULL.
 @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
**/

VOID
ShowScreen0(){
    UINT32 x;
    
    gST->ConOut->ClearScreen (gST->ConOut);
    
    Print (L"<PCI utility ChungYng >\n");
    Print (L"PCI Device List: Screen0\n");
    Print (L"ListNo.   Bus   Dev   Func\n");
    
    //make Pci device list
    for (x=0; x<65536; x++ ){
      if (gPcidev[x].NumOfList > 0){
        Print (L"%d         %d    %d    %d\n",
          gPcidev[x].NumOfList,
          gPcidev[x].Bus,gPcidev[x].Dev,
          gPcidev[x].Func
          );
        }
    }
  
    Print (L"Press Arrow Key to Select PCI Device.\n");
    
    Print (L"Press [Enter] to Show Selected PCI Device.\n");
    
    Print (L"Press [Esc] to Quit.\n");

}

/**
 Find Pci device and put the config space in array.

 @param[IN] ImageHandle         
 @param[IN] mode         
 
 @retval EFI_SUCCESS            The operation completed successfully.
 @retval EFI_INVALID_PARAMETER  Time is NULL.
 @retval EFI_DEVICE_ERROR       The time could not be retrieved due to hardware error.
**/

EFI_STATUS
DoPciDeviceinfo (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                       Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRBinterface;

  UINT32 find;
  UINT8 buffer;
  UINT32 index;
  UINT8 Bus = 0;
  UINT8 Dev = 0;
  UINT8 Func = 0;
  UINTN i;
  
  Status = gBS->LocateProtocol (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  &PciRBinterface
                  );
  
  if (EFI_ERROR (Status)) {
    Print(L"LocateProtocol Error\n");
    return Status;
  }
  
  for (index=0; index < 65536 ; index++){//Bus Dev func reg
    if ((index !=0 ) && (index % 256 == 0 )){
      Bus++;
      Dev = 0;
      Func = 0;
    }
    else if ((index != 0) && (index % 8 == 0)){
      Dev++;
      Func = 0;
    }
    else if (index !=0 ){
      Func++;
    }
    //Print(L"Bus:%d Dev%d Func:%d \n",Bus,Dev,Func);
    Status = PciRBinterface->Pci.Read (
                      PciRBinterface, 
                      EfiPciWidthUint32,
                      EFI_PCI_ADDRESS(Bus,Dev,Func,0),
                      1, 
                      &find
                      ); 
    if (EFI_ERROR (Status)) {
      Print(L"Pci.Read Error\n");
      return Status;
    }
    if (find != 0xffffffff){
      
      for (i=0; i<256; i++){
        Status = PciRBinterface->Pci.Read (
                          PciRBinterface,
                          EfiPciWidthUint8,
                          EFI_PCI_ADDRESS(Bus,Dev,Func,i),
                          1, 
                          &buffer
                          );
        gPcidev[index].ConfigSpaceReg[i]=buffer;
      }
      
      if (EFI_ERROR (Status)) {
        Print(L"Pci.Read Error\n");
        return Status;
      }
      
      gPcidev[index].Bus = Bus;
      gPcidev[index].Dev = Dev;
      gPcidev[index].Func = Func;
      gNumPcidev++;
      gPcidev[index].NumOfList= gNumPcidev;
    }
     
  }
  

  return Status;
}
VOID
ModifyPCIconfigspace(
  IN UINT32 index,
  IN UINT32 mode
  )
{
  EFI_STATUS                       Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRBinterface;
  EFI_INPUT_KEY                    Key;
  
  UINT64 data = 0xff;
  UINT32 t;
  UINT64 offset = 0;
  UINTN i;
  UINT32 n[3] = {2,4,8};
  
  
  Print(L"Please enter offset : ");
  for (i=0; i<2; i++){
    Status = GetKey (&Key);
    if (PutKey(Key, &offset,0) == 1)
      i--;
  }
  if (mode == 1){
    if (offset % 2)
      Print(L"\nDepond on Operation Mode, Correct Offset to %2Xh.\n",(offset/2)*2);
  }
  if (mode == 2){
    if (offset % 4)
      Print(L"\nDepond on Operation Mode, Correct Offset to %2Xh.\n",(offset/4)*4);
  }
  Print(L"\nPlease enter data : ");
  
  if (mode == 0){
    for (i=0;i<n[0];i++){
      Status = GetKey (&Key);
      if(PutKey(Key, 0,&data) == 1)
        i--;
      }
    }
  else if (mode == 1){
    for (i=0;i<n[1];i++){
      Status = GetKey (&Key);
      if(PutKey(Key, 0,&data) == 1)
        i--;
      }
    }
  else {
    for (i=0;i<n[2];i++){
      Status = GetKey (&Key);
      if(PutKey(Key, 0,&data) == 1)
        i--;
      }
  }
  Print(L"\n%d %d\n",offset, data);
  //write pci and read pci
  Status = gBS->LocateProtocol (
              &gEfiPciRootBridgeIoProtocolGuid, 
              NULL,
              &PciRBinterface
              );
  if (EFI_ERROR (Status)) {
    Print(L"LocateProtocol Error\n");
    return;
  }
  if (mode == 0){
    Status = PciRBinterface->Pci.Write (
              PciRBinterface, 
              EfiPciWidthUint8,
              EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,offset),
              1,
              &data
              );
     if (EFI_ERROR (Status)) {
       Print(L"Pci.Write Error\n");
       return ;
     }
    Status = PciRBinterface->Pci.Read (
               PciRBinterface,
               EfiPciWidthUint8,
               EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,offset),
               1, 
               &t
               );
    
     if (EFI_ERROR (Status)) {
       Print(L"Pci.Read Error\n");
       return ;
     }
  gPcidev[index].ConfigSpaceReg[offset] = (UINT8)t;
  Print(L"data = %X read = %X\n",data,t);
    
  } else if (mode == 1){
    PciRBinterface->Pci.Write(
                      PciRBinterface, 
                      EfiPciWidthUint16,
                      EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,(offset/2)*2),
                      1,
                      &data
                      );
    
      if (EFI_ERROR (Status)) {
        Print(L"Pci.Write Error\n");
        return ;
      }
    for (i=0; i<2; i++){
      PciRBinterface->Pci.Read (
                        PciRBinterface,
                        EfiPciWidthUint8,
                        EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,(offset/2)*2)+i,
                        1, 
                        &t
                        );
      
      if (EFI_ERROR (Status)) {
        Print(L"Pci.Write Error\n");
        return ;
      }
      gPcidev[index].ConfigSpaceReg[(offset/2) * 2 + i] =(UINT8) t;
    }
  } else {
    PciRBinterface->Pci.Write (
                      PciRBinterface, 
                      EfiPciWidthUint32,
                      EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,(offset/4)*4),
                      1, 
                      &data
                      );    
    if (EFI_ERROR (Status)) {
     Print(L"Pci.Write Error\n");
     return ;
    }
    
    for (i=0; i<4; i++){
      PciRBinterface->Pci.Read (
                        PciRBinterface, 
                        EfiPciWidthUint8,
                        EFI_PCI_ADDRESS(gPcidev[index].Bus,gPcidev[index].Dev,gPcidev[index].Func,(offset/4)*4)+i,
                        1, 
                        &t
                        );
      
    if (EFI_ERROR (Status)) {
      Print(L"Pci.Read Error\n");
      return ;
    }
      gPcidev[index].ConfigSpaceReg[(offset/4) * 4 + i] = (UINT8) t;
    }
  }
}

UINT8
PutKey(
  IN EFI_INPUT_KEY   Key,
  OUT UINT64 *offset,
  OUT UINT64 *data
  )
{
  UINT64 *buff;
  
  if (offset != NULL)
    buff = offset;
  else
    buff = data;
  
  switch (Key.UnicodeChar) {
    
  case L'0':
    *buff = *buff * 16 + 0;
    Print (L"0");
    break;
    
  case L'1':
    *buff = *buff * 16 + 1;
    Print (L"1");
    break;
    
  case L'2':
    *buff = *buff * 16 + 2;
    Print (L"2");
    break;
    
  case L'3':
    *buff = *buff * 16 + 3;
    Print (L"3");
    break;
    
  case L'4':
    *buff = *buff * 16 + 4;
    Print (L"4");
    break;
    
  case L'5':
    *buff = *buff * 16 + 5;
    Print (L"5");
    break;
    
  case L'6':
    *buff = *buff * 16 + 6;
    Print (L"6");
    break;
    
  case L'7':
    *buff = *buff * 16 + 7;
    Print (L"7");
    break;
    
  case L'8':
    *buff = *buff * 16 + 8;
    Print (L"8");
    break;
    
  case L'9':
    *buff = *buff * 16 + 9;
    Print (L"9");
    break;
    
  case L'A':
  case L'a':
    *buff = *buff * 16 + 10;
    Print (L"A");
    break;
    
  case L'B':
  case L'b':
    *buff = *buff * 16 + 11;
    Print (L"B");
    break;
    
  case L'C':
  case L'c':
    *buff = *buff * 16 + 12;
    Print (L"C");
    break;
    
  case L'D':
  case L'd':
    *buff = *buff * 16 + 13;
    Print (L"D");
    break;
    
  case L'E':
  case L'e':
    *buff = *buff * 16 + 14;
    Print (L"E");
    break;
    
  case L'F':
  case L'f':
    *buff = *buff * 16 + 15;
    Print (L"F");
    break;
    
  default:
    return 1;
  }
  return 0;
}
