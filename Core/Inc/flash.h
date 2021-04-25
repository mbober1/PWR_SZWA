#include "stm32l476g_discovery_qspi.h"


static uint16_t dataCount;


// struktura z danymi
struct Data {
	RTC_TimeTypeDef rtcTime;
	RTC_DateTypeDef rtcData;
	int meassure;
};

// wypisz dane
void dataInfo(const struct Data *data) {
	printf("Date: %02d.%02d.20%02d\n\r", data->rtcData.Date, data->rtcData.Month, data->rtcData.Year);
	printf("Time: %02d:%02d:%02d:%03ld\n\r", data->rtcTime.Hours, data->rtcTime.Minutes, data->rtcTime.Seconds, data->rtcTime.SubSeconds);
	printf("Meassure: %d\r\n\n", data->meassure);
}

// zapis do pamieci
uint8_t storeStruct(void *data_source, size_t size, uint16_t place)
{
//  printf("Store start | Place %d |", place);

  for(size_t i = 0; i < size; i++) {
    uint8_t data = ((uint8_t *)data_source)[i];
    uint16_t address = (size*place) + i;
    if(BSP_QSPI_Write(&data, address, 1) != QSPI_OK) return QSPI_ERROR;
//    printf(" [%d]", address);
  }
//  printf(" | Store finnished\r\n");
  return QSPI_OK;
}

uint8_t storeNextStruct(void *data_source) {
	return storeStruct(data_source, sizeof(struct Data), dataCount++);
}


// odczyt w pamieci
uint8_t loadStruct(void *data_dest, size_t size, uint16_t place)
{
	if(place > dataCount) {
		printf("There is only %d elements, not %d", dataCount, place);
		return QSPI_ERROR;
	}


//	printf("Load start | Place %d |", place);

    for(size_t i = 0; i < size; i++)
    {
        uint8_t data = 0;
        uint16_t address = (size*place) + i;
        if(BSP_QSPI_Read(&data, address, 1) != QSPI_OK) return QSPI_ERROR;
        ((char *)data_dest)[i] = data;
//        printf(" [%d]", address);
    }
//    printf(" | Load finnished\r\n");
    return QSPI_OK;
}
