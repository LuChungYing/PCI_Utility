#ifndef _Pci_header_H_
#define _Pci_header_H_

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimpleTextOut.h>


typedef struct{
  UINT8 ConfigSpaceReg[256];
  UINT32 Bus;
  UINT32 Dev;
  UINT32 Func;
  UINT32 NumOfList;
}PCIdevInfo;

EFI_STATUS
GetKey (
  OUT  EFI_INPUT_KEY  *Key
);

BOOLEAN
ShowPressedKey (
  IN  EFI_INPUT_KEY  Key
);

EFI_STATUS
DoPciDeviceinfo (
);
UINT32
ShowScreen1 (
);
VOID
ShowScreen0 (
);

VOID
ModifyPCIconfigspace (
  VOID
);

UINT8
PutKey (
);

#endif

