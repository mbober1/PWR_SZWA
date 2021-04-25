#include "stm32l476g_discovery_qspi.h"


static uint16_t dataCount;


/**
 * Data structure.
 *
 * @param rtcTime STM style time structure.
 * @param rtcData STM style data structure.
 */
struct Data {
	RTC_TimeTypeDef rtcTime;
	RTC_DateTypeDef rtcData;
	uint16_t meassure;
};


/**
 * Print data information to serial monitor.
 *
 * @param data Pointer to Data struct.
 */
void dataInfo(const struct Data *data) {
	printf("Date: %02d.%02d.20%02d\n\r", data->rtcData.Date, data->rtcData.Month, data->rtcData.Year);
	printf("Time: %02d:%02d:%02d:%03ld\n\r", data->rtcTime.Hours, data->rtcTime.Minutes, data->rtcTime.Seconds, data->rtcTime.SubSeconds);
	printf("Meassure: %d\r\n\n", data->meassure);
}


/**
 * Writes data to external memory
 *
 * @param dataSource Source of data.
 * @param size Size of data.
 * @param place Place in external memory.
 * @return QSPI Error code.
 */
uint8_t storeStruct(void *dataSource, size_t size, uint16_t place)
{
  for(size_t i = 0; i < size; i++) {
    uint8_t data = ((uint8_t *)dataSource)[i];
    uint16_t address = (size*place) + i;
    if(BSP_QSPI_Write(&data, address, 1) != QSPI_OK) return QSPI_ERROR;
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
uint8_t loadStruct(void *dataDestination, size_t size, uint16_t place)
{
	if(place > dataCount) {
		printf("There is only %d elements, not %d", dataCount, place);
		return QSPI_ERROR;
	}

  for(size_t i = 0; i < size; i++) {
      uint8_t data = 0;
      uint16_t address = (size*place) + i;
      if(BSP_QSPI_Read(&data, address, 1) != QSPI_OK) return QSPI_ERROR;
      ((char *)dataDestination)[i] = data;
  }
    return QSPI_OK;
}


/**
 * Return last saved data from external memory.
 *
 * @param data Pointer to Data struct.
 * @return QSPI Error code.
 */
uint8_t getLastStruct(struct Data *tmp) {
	return loadStruct(tmp, sizeof(struct Data), dataCount - 1);
}


/**
 * Writes data to external memory in the next free memory location.
 *
 * @param dataSource Source of data.
 * @return QSPI Error code.
 */
uint8_t storeNextStruct(void *dataSource) {
	return storeStruct(dataSource, sizeof(struct Data), dataCount++);
}


/**
 * Saves the measurement with the current timestamp in the next free memory location.
 *
 * @param data Measurement.
 * @return QSPI Error code.
 */
uint8_t nextMeasurement(uint16_t data) {
	struct Data tmp;
	HAL_RTC_GetTime(&hrtc, &tmp.rtcTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &tmp.rtcData, RTC_FORMAT_BIN);
	tmp.meassure = data;
	return storeNextStruct(&tmp);
}
