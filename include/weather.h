#ifndef WEATHER_H
#define WEATHER_H 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_TIMEZONE_LEN         14
#define MAX_TIME_LEN             32
#define HOURLY_DATA_POINTS       168          // 7 days times 24 hours to get it
#define OPEN_METEO_URL           "https://api.open-meteo.com/v1/forecast?latitude=45.8150&longitude=15.9819&current_weather=true&hourly=temperature_2m,relativehumidity_2m,windspeed_10m"


typedef struct {
    char time[32];                            // FORMAT: "2025-06-26T13:00"
    double temperature;                       // 37.2  
    double windspeed;                         // 7.4
    int winddirection;                        // 219
    int is_day;                               // 1 if day and 0 if not     
    int weathercode;                          // 0 = clear, 1 = cloudy, 2 = rain and etc.
} s_current_weather;

typedef struct {
    char time[16];
    char temperature[8];
    char windspeed[8];
    char winddirection[8];
} s_current_weather_units;

typedef enum {
    E_WEATHER_SUCCESS = 0,
    E_WEATHER_ERROR = -1,
    E_WEATHER_INVALID_RESPONSE = -2,
    E_WEATHER_INVALID_URL = -3,
    E_WEATHER_INVALID_JSON = -4,
    E_WEATHER_INVALID_DATA = -5,
    E_WEATHER_TIMEOUT = -6,
    E_WEATHER_NOT_INITIALIZED = -7,
    E_WEATHER_UNSUPPORTED = -8,
    E_WEATHER_INVALID_PARAMETER = -9,
    E_WEATHER_NOT_FOUND = -10,
    E_WEATHER_UNAUTHORIZED = -11,
    E_WEATHER_FORBIDDEN = -12,
    E_WEATHER_RATE_LIMITED = -13,
    E_WEATHER_SERVICE_UNAVAILABLE = -14,
    E_WEATHER_UNKNOWN_ERROR = -99
} e_weather_error;

typedef struct {
    char time[16];                            // "iso8601"
    char temperature_2m[8];                   // "°C"
    char relativehumidity_2m[8];              // "%"
    char windspeed_10m[8];                    // "km/h"
} s_hourly_units;

typedef struct {
    char time[HOURLY_DATA_POINTS][32];
    double temperature_2m[HOURLY_DATA_POINTS];
    int relativehumidity_2m[HOURLY_DATA_POINTS];
    double windspeed_10m[HOURLY_DATA_POINTS];
} s_hourly;

typedef struct {
    double latitude;
    double longitude;
    double elevation;
    char timezone[64];

    s_current_weather current_weather;
    s_current_weather_units current_weather_units;

} s_weather;

typedef struct {
    char *data;
    size_t size;
} s_response_data;


/* Function prototypes */
e_weather_error fetch_weather_data(const char *url, char **response);
e_weather_error parse_weather_json(const char *json_data, s_weather *weather_data);

#endif
