/*
* @Author: karlosiric
* @Date:   2025-06-27 09:59:48
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-07-14 11:26:38
*/

#include "../include/weather.h"

int test_api(void) {

    printf("Testing Weather API fetching mechanicsm ... \n");
    char *json_response = NULL;
    const char *test_url = "https://api.open-meteo.com/v1/forecast?latitude=45.8150&longitude=15.9819&current_weather=true&hourly=temperature_2m,relativehumidity_2m,windspeed_10m";

    printf("1. Fetching weather data ... \n");
    e_weather_error result = fetch_weather_data(test_url, &json_response);
    if (result != E_WEATHER_SUCCESS) {
        fprintf(stderr, "Error fetching weather data: %d\n", result);
        return (-1);
    }
    printf("\nSuccessfully fetched the data from the URL %s\n", test_url);
    printf("----------------------------------\n");
    printf("Fetched %zu bytes of data.\n", strlen(json_response));
    printf("\nFetched data: %s\n", json_response);
    printf("----------------------------------\n");

    printf("2. Parsing the fetched data ... \n");
    s_weather weather_data;
    e_weather_error parse_result = parse_weather_json(json_response, &weather_data);

    if (parse_result != E_WEATHER_SUCCESS) {
        fprintf(stderr, "Error parsing weather data: %d\n", parse_result);
        free(json_response);
        return (-2);
    }

    printf("Successfully parsed the weather data from the JSON response.\n");
    printf("----------------------------------\n");
    printf("   Location: %.2f, %.2f\n", weather_data.latitude, weather_data.longitude);
    printf("   Elevation: %.1fm\n", weather_data.elevation);
    printf("   Timezone: %s\n", weather_data.timezone);
    printf("\n Current Weather:\n");
    printf("   Temperature: %.1f%s\n", weather_data.current_weather.temperature, weather_data.current_weather_units.temperature);
    printf("   Wind Speed: %.1f %s\n", weather_data.current_weather.windspeed, weather_data.current_weather_units.windspeed);
    printf("   Wind Direction: %d%s\n", weather_data.current_weather.winddirection, weather_data.current_weather_units.winddirection);
    printf("   Day/Night: %s\n", weather_data.current_weather.is_day ? "Day" : "Night");
    printf("   Weather Code: %d\n", weather_data.current_weather.weathercode);

    printf("\n Printing first 20 hourly data forecasts:\n");
    int nums = HOURLY_DATA_POINTS < 20 ? HOURLY_DATA_POINTS : 20;

    for (int i = 0; i < nums; i++) {
        printf("   %s: %.1f%s, %d%s humidity, %.1f %s wind\n",
            weather_data.hourly.time[i],
            weather_data.hourly.temperature_2m[i], weather_data.hourly_units.temperature_2m,
            weather_data.hourly.relativehumidity_2m[i], weather_data.hourly_units.relativehumidity_2m,
            weather_data.hourly.windspeed_10m[i], weather_data.hourly_units.windspeed_10m);
    }

    free(json_response);
    printf("Test completed successfully.\n");
    printf("Exiting the program ... \n");
    return (0);
}
