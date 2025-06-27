/*
* @Author: karlosiric
* @Date:   2025-06-27 09:59:48
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 13:57:10
*/

#include "../include/weather.h"

int main(void) {

    printf("Testing Weather API fetching mechanicsm ... \n");
    char *json_response = NULL;
    const char *test_url = "https://api.open-meteo.com/v1/forecast?latitude=45.8150&longitude=15.9819&current_weather=true&hourly=temperature_2m,relativehumidity_2m,windspeed_10m";
    e_weather_error result = fetch_weather_data(test_url, &json_response);

    if (result != E_WEATHER_SUCCESS) {
        fprintf(stderr, "Error fetching weather data: %d\n", result);
        return (-1);
    }

    







    return (0);
}
