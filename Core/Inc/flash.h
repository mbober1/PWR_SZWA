#include "stm32l476g_discovery_qspi.h"

static struct Data bestStruct;

/**
 * Data structure.
 *
 * @param rtcTime STM style time structure.
 * @param rtcData STM style data structure.
 */
struct Data {
	RTC_TimeTypeDef rtcTime;
	RTC_DateTypeDef rtcData;
	float meassure;
};

uint8_t setDataCount(uint16_t count) {
	uint8_t data[2] = { (uint8_t) (count >> 8), (uint8_t) count };
	if (BSP_QSPI_Erase_Block(0) != QSPI_OK)
		return QSPI_ERROR;
	if (BSP_QSPI_Write(data, 0, 2) != QSPI_OK)
		return QSPI_ERROR;
	return QSPI_OK;
}

uint16_t getDataCount() {
	uint8_t data[2];
	if (BSP_QSPI_Read(data, 0, 2) == QSPI_OK) {
		uint16_t count = (uint16_t) ((data[0] << 8) | data[1]);
		return count;
	} else
		return 0;
}

/**
 * Copy structure. From A to B.
 *
 * @param a Source of data.
 * @param b Destination of data.
 */
void copyStruct(const struct Data *a, struct Data *b) {
	b->meassure = a->meassure;
	b->rtcData = a->rtcData;
	b->rtcTime = a->rtcTime;
}



uint8_t eventType(float W) {
	if 		(W > 0.11 && W < 0.35) return 1;
	else if (W > 0.35 && W < 0.44) return 2;
	else if (W > 0.44 && W < 1.66) return 3;
	else if (W > 1.66 && W < 8.20) return 4;
	else if (W > 8.20 && W < 32.8) return 5;
	else if (W > 32.8 && W < 328) return 6;
	else return 0;
}

/**
 * Print data information to serial monitor.
 *
 * @param data Pointer to Data struct.
 */
void infoStruct(const struct Data *data) {
	printf("	Date: %02d.%02d.20%02d\n\r", data->rtcData.Date,
			data->rtcData.Month, data->rtcData.Year);
	printf("	Time: %02d:%02d:%02d:%03ld\n\r", data->rtcTime.Hours,
			data->rtcTime.Minutes, data->rtcTime.Seconds,
			data->rtcTime.SubSeconds);
	printf("	Meassure: %f\r\n", data->meassure);
	printf("	Zdarzenie klasy %d\r\n", eventType(data->meassure));
}

/**
 * Writes data to external memory
 *
 * @param dataSource Source of data.
 * @param size Size of data.
 * @param place Place in external memory.
 * @return QSPI Error code.
 */
uint8_t storeStruct(void *dataSource, size_t size, uint16_t place) {
	for (size_t i = 0; i < size; i++) {
		uint8_t data = ((uint8_t*) dataSource)[i];
		uint16_t address = (size * place) + i + N25Q128A_SUBSECTOR_SIZE;
		if (BSP_QSPI_Write(&data, address, 1) != QSPI_OK)
			return QSPI_ERROR;
	}
	return QSPI_OK;
}

/**
 * Read data from external memory.
 *
 * @param dataDestination Destination of data.
 * @param size Size of data.
 * @param place Place in external memory.
 * @return QSPI Error code.
 */
uint8_t loadStruct(void *dataDestination, size_t size, uint16_t place) {
	uint16_t dataCount = getDataCount();

	if (place > dataCount) {
		printf("Jest tylko %d elementow, nie %d", dataCount, place);
		return QSPI_ERROR;
	}

	for (size_t i = 0; i < size; i++) {
		uint8_t data;
		uint16_t address = (size * place) + i + N25Q128A_SUBSECTOR_SIZE;
		if (BSP_QSPI_Read(&data, address, 1) != QSPI_OK)
			return QSPI_ERROR;
		((char*) dataDestination)[i] = data;
	}
	return QSPI_OK;
}

/**
 * Return last saved data from external memory.
 *
 * @param data Pointer to Data struct.
 * @return QSPI Error code.
 */
void getLastStruct() {
	struct Data data;
	uint16_t dataCount = getDataCount();
	if(dataCount) {
		loadStruct(&data, sizeof(struct Data), dataCount - 1);
		infoStruct(&data);
	} else printf("Nie ma zadnych danych!\r\n");
}


/**
 * Writes data to external memory in the next free memory location.
 *
 * @param dataSource Source of data.
 * @return QSPI Error code.
 */
uint8_t storeNextStruct(void *dataSource) {
	if(((struct Data*)dataSource)->meassure > bestStruct.meassure) copyStruct(dataSource, &bestStruct);
	uint16_t dataCount = getDataCount() + 1;
	setDataCount(dataCount);
	return storeStruct(dataSource, sizeof(struct Data), dataCount - 1);
}


/**
 * Saves the measurement with the current timestamp in the next free memory location.
 *
 * @param data Measurement.
 * @return QSPI Error code.
 */
uint8_t nextMeasurement(float data) {
	struct Data tmp;
	HAL_RTC_GetTime(&hrtc, &tmp.rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &tmp.rtcData, RTC_FORMAT_BIN);
	tmp.meassure = data;
	return storeNextStruct(&tmp);
}


int memLeft() {
	return ((N25Q128A_FLASH_SIZE - N25Q128A_PAGE_SIZE) - (sizeof(struct Data) * getDataCount())) / sizeof(struct Data);
}


void clearMemory() {
	if (BSP_QSPI_Erase_Chip() != QSPI_OK)
		return QSPI_ERROR;
	setDataCount(0);
}


void listAllData() {
	struct Data data;
	uint16_t dataCount = getDataCount();
	uint8_t i;
	if(dataCount > 10)  i = 10;
	else i = dataCount;

	while (i) {
		loadStruct(&data, sizeof(struct Data), dataCount - 11 + i);
		printf("[%3d] %02d:%02d:%02d:%03ld	%02d.%02d.20%02d -> %f\n\r", dataCount - 10 + i,
				data.rtcTime.Hours, data.rtcTime.Minutes, data.rtcTime.Seconds,
				data.rtcTime.SubSeconds, data.rtcData.Date, data.rtcData.Month,
				data.rtcData.Year, data.meassure);
		if(i > 10) break;
		else i--;
	}
	printf("\r\n\n");
}
