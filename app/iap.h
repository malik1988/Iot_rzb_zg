#if BOOT_LOADER

#ifndef IAP_H
#define IAP_H

uint32_t IAP_GetFile();
uint8_t IAP_Program_Check_Buf();
uint8_t IAP_Program_Check();
void IAP_Program_Erase();



#endif //EDNOF IAP_H

#endif //endof BOOT_LOADER


