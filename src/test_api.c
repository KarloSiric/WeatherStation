/*
* @Author: karlosiric
* @Date:   2025-06-27 09:59:48
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 10:09:09
*/

#include "../include/weather.h"

int main(void) {

    printf("Testing Weather API fetching mechanicsm ... \n");
    char *response = NULL;
    const char *test_url = "https://api.open-meteo.com/v1/forecast?latitude=45.8150&longitude=15.9819&current_weather=true&hourly=temperature_2m,relativehumidity_2m,windspeed_10m";
    e_weather_error result = fetch_weather_data(test_url, &response);

    if (result != E_WEATHER_SUCCESS) {
        fprintf(stderr, "Error fetching weather data: %d\n", result);
        return (-1);
    }

    if (response == NULL) {
        fprintf(stderr, "No data has been received from the API.\n");
        free(response);
        return (-1);
    }

    printf("Received a response from the API successfully.\n");
    printf("Response: %s\n", response);
    free(response);
    return (0);
}
