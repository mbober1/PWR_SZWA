/* Defines declarations for external flash on board STM32F7-Disco */
#define N25Q128A_FLASH_SIZE                  0x1000000 /* 128 MBits => 16MBytes => 16,777,216 bytes */
#define N25Q128A_SECTOR_SIZE                 0x10000   /* 256 sectors of 64KBytes 		*/
#define N25Q128A_SUBSECTOR_SIZE              0x1000    /* 4096 subsectors of 4kBytes	*/
#define N25Q128A_PAGE_SIZE                   0x100     /* 65536 pages of 256 bytes		*/
#define N25Q128A_SUBSECTOR_CNT               4096      /* 4096 subsectors				*/
#define N25Q128A_SECTOR_CNT               	 256       /* 256 sectors					*/

#define N25Q128A_DUMMY_CYCLES_READ_QUAD      10

#define N25Q128A_BULK_ERASE_MAX_TIME         250000
#define N25Q128A_SECTOR_ERASE_MAX_TIME       3000
#define N25Q128A_SUBSECTOR_ERASE_MAX_TIME    800

/* Flag Status Register */
#define N25Q128A_FSR_PRERR                   ((uint8_t)0x02)    /* Protection error */
#define N25Q128A_FSR_PGSUS                   ((uint8_t)0x04)    /* Program operation suspended */
#define N25Q128A_FSR_VPPERR                  ((uint8_t)0x08)    /* Invalid voltage during program or erase */
#define N25Q128A_FSR_PGERR                   ((uint8_t)0x10)    /* Program error */
#define N25Q128A_FSR_ERERR                   ((uint8_t)0x20)    /* Erase error */
#define N25Q128A_FSR_ERSUS                   ((uint8_t)0x40)    /* Erase operation suspended */
#define N25Q128A_FSR_READY                   ((uint8_t)0x80)    /* Ready or command in progress */


/* Registers */
#define N25Q128A_SR_WIP                      ((uint8_t)0x01) 	/* Write in progress */
#define N25Q128A_VCR_NB_DUMMY                ((uint8_t)0xF0) 	/* Number of dummy clock cycles */
#define N25Q128A_SR_WREN                     ((uint8_t)0x02)	/* Write enable latch */

/* Register Operations */
#define READ_FLAG_STATUS_REG_CMD             0x70
#define WRITE_STATUS_REG_CMD                 0x01

/* Commands */
/* Reset operations */
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Operations */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A

/* Read Operations */
#define READ_CMD                             0x03
#define FAST_READ_CMD                        0x0B
#define DUAL_OUT_FAST_READ_CMD               0x3B
#define DUAL_INOUT_FAST_READ_CMD             0xBB
#define QUAD_OUT_FAST_READ_CMD               0x6B
#define QUAD_INOUT_FAST_READ_CMD             0xEB

#define READ_VOL_CFG_REG_CMD                 0x85
#define WRITE_VOL_CFG_REG_CMD                0x81
#define READ_STATUS_REG_CMD                  0x05
#define WRITE_STATUS_REG_CMD                 0x01
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04
/* Erase Operations */
#define SUBSECTOR_ERASE_CMD                  0x20
#define SECTOR_ERASE_CMD                     0xD8
#define BULK_ERASE_CMD                       0xC7

#define EXT_QUAD_IN_FAST_PROG_CMD            0x12


uint8_t flashReadID() {
	uint8_t buffer;

	QSPI_CommandTypeDef command;
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	command.Instruction       = READ_ID_CMD;
	command.AddressMode       = QSPI_ADDRESS_1_LINE;
	command.AddressSize       = QSPI_ADDRESS_24_BITS;
	command.Address           = 0x000000;
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	command.DataMode          = QSPI_DATA_1_LINE;
	command.DummyCycles       = 0;
	command.NbData            = 2;
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if(HAL_QSPI_Command(&hqspi, &command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 0;
	if(HAL_QSPI_Receive(&hqspi, &buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 0;

	return buffer;
}


uint8_t flashReadData(uint16_t address, uint16_t size, uint8_t* buffer) {
	QSPI_CommandTypeDef command;
	command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	command.Instruction       = READ_CMD;
	command.AddressMode       = QSPI_ADDRESS_4_LINES;
	command.AddressSize       = QSPI_ADDRESS_24_BITS;
	command.Address           = address;
	command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	command.DataMode          = QSPI_DATA_4_LINES;
	command.DummyCycles       = N25Q128A_DUMMY_CYCLES_READ_QUAD;
	command.NbData            = size;
	command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if(HAL_QSPI_Command(&hqspi, &command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 0;
	if(HAL_QSPI_Receive(&hqspi, buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) return 0;

	return 1;
}
