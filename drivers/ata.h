#pragma once
#include <stdint.h>

void read_sectors_ATA_PIO(void* target_address, uint32_t LBA, uint8_t sector_count);
