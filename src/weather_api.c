/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 13:54:25
*/

#include "../include/weather.h"
#include <curl/curl.h>
#include <errno.h>
#include <cjson/cJSON.h>

size_t write_data(void *buffer, size_t size, size_t nmemb, void *usrptr) {

    s_response_data *data = (s_response_data *)usrptr;
    size_t total_size = size * nmemb;

    char *new_data = realloc(data->data, data->size + total_size + 1);
    if (new_data == NULL) {
        fprintf(stderr, "Failed to reallocate memory for response data: %s\n", strerror(errno));
        return (0);
    }
    
    data->data = new_data;
    memcpy(data->data + data->size, buffer, total_size);
    data->size += total_size;
    data->data[data->size] = '\0'; 
    return total_size;
}

e_weather_error fetch_weather_data(const char *url, char **response) {
    if (url == NULL || response == NULL) {
        fprintf(stderr, "Invalid parameters: url of response cannot be NULL.\n");
        return E_WEATHER_INVALID_PARAMETER;
    }   

    s_response_data *response_struct = malloc(sizeof(s_response_data));
    if (response_struct == NULL) {
        fprintf(stderr, "Failed to allocate memory for response struct: %s\n", strerror(errno));
        return E_WEATHER_NOT_INITIALIZED;
    } 
    response_struct->data = NULL;
    response_struct->size = 0;

    CURL *handle = curl_easy_init();
    if (!handle) {
        fprintf(stderr, "Failed to initialize CURL handle\n");
        free(response_struct);
        return E_WEATHER_NOT_INITIALIZED;
    }   

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)response_struct);

    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10L); // Set a timeout of 10 seconds

    CURLcode request = curl_easy_perform(handle);

    if (request != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(request));
        free(response_struct->data);
        free(response_struct);
        curl_easy_cleanup(handle);
        return E_WEATHER_ERROR;
    }

    *response = response_struct->data;
    if (*response == NULL) {
        free(response_struct);
        curl_easy_cleanup(handle);
        return E_WEATHER_ERROR;
    }
    long response_code;

    curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        free(response_struct->data);
        free(response_struct);
        curl_easy_cleanup(handle);
        return E_WEATHER_INVALID_RESPONSE;
    }

    curl_easy_cleanup(handle);
    free(response_struct);
    return E_WEATHER_SUCCESS;
}

e_weather_error parse_weather_json(const char *json_data, s_weather *weather_data) {
    
    cJSON *root = cJSON_Parse(json_data);
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON data in %s: %s\n", __func__, cJSON_GetErrorPtr());
        return E_WEATHER_INVALID_JSON;
    }

    cJSON *latitude = cJSON_GetObjectItem(root, "latitude");
    cJSON *longitude = cJSON_GetObjectItem(root, "longitude");
    cJSON *elevation = cJSON_GetObjectItem(root, "elevation");
    cJSON *timezone = cJSON_GetObjectItem(root, "timezone");

    if (!cJSON_IsNumber(latitude) || !cJSON_IsNumber(longitude) || !cJSON_IsNumber(elevation) 
        || !cJSON_IsString(timezone)) {
        fprintf(stderr, "Invalid data in JSON response.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    weather_data->latitude = latitude->valuedouble;
    weather_data->longitude = longitude->valuedouble;
    weather_data->elevation = elevation->valuedouble;
    strncpy(weather_data->timezone, timezone->valuestring, sizeof(weather_data->timezone) - 1);
    weather_data->timezone[sizeof(weather_data->timezone) - 1] = '\0'; // Ensure null-termination

    cJSON *current_weather = cJSON_GetObjectItem(root, "current_weather");
    if (!cJSON_IsObject(current_weather)) {
        fprintf(stderr, "current_weather data is not an object or is not found.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    cJSON *temp = cJSON_GetObjectItem(current_weather, "temperature");
    cJSON *windspeed = cJSON_GetObjectItem(current_weather, "windspeed");
    cJSON *winddirection = cJSON_GetObjectItem(current_weather, "winddirection");
    cJSON *is_day = cJSON_GetObjectItem(current_weather, "is_day");
    cJSON *weather_code = cJSON_GetObjectItem(current_weather, "weathercode");

    if (!cJSON_IsNumber(temp) || !cJSON_IsNumber(windspeed) || !cJSON_IsNumber(winddirection) || !cJSON_IsNumber(is_day) || !cJSON_IsNumber(weather_code)) {
        fprintf(stderr, "Invalid current_weather data in JSON response.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    weather_data->current_weather.is_day = is_day->valueint;
    weather_data->current_weather.weathercode = weather_code->valueint;
    weather_data->current_weather.temperature = temp->valueint;
    weather_data->current_weather.windspeed = windspeed->valuedouble;
    weather_data->current_weather.winddirection = winddirection->valuedouble;

    cJSON *current_weather_units = cJSON_GetObjectItem(root, "current_weather_units");
    if (!cJSON_IsObject(current_weather_units)) {
        fprintf(stderr, "current_weather_units data is not an object or is not found.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    cJSON *time = cJSON_GetObjectItem(current_weather_units, "time");
    cJSON *temperature = cJSON_GetObjectItem(current_weather_units, "temperature");
    cJSON *windspeed_units = cJSON_GetObjectItem(current_weather_units, "windspeed");
    cJSON *winddirection_units = cJSON_GetObjectItem(current_weather_units, "winddirection");
    if (!cJSON_IsString(time) || !cJSON_IsString(temperature) || !cJSON_IsString(windspeed_units) || !cJSON_IsString(winddirection_units)) {
        fprintf(stderr, "Invalid current_weather_units data in JSON response.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    strncpy(weather_data->current_weather_units.time, time->valuestring, sizeof(weather_data->current_weather_units.time) - 1);
    strncpy(weather_data->current_weather_units.temperature, temperature->valuestring, sizeof(weather_data->current_weather_units.temperature) - 1);
    strncpy(weather_data->current_weather_units.windspeed, windspeed_units->valuestring, sizeof(weather_data->current_weather_units.windspeed) -1);
    strncpy(weather_data->current_weather_units.winddirection, winddirection_units->valuestring, sizeof(weather_data->current_weather_units.winddirection) - 1);

    weather_data->current_weather_units.time[sizeof(weather_data->current_weather_units.time) - 1] = '\0';
    weather_data->current_weather_units.temperature[sizeof(weather_data->current_weather_units.temperature) - 1] = '\0';
    weather_data->current_weather_units.windspeed[sizeof(weather_data->current_weather_units.windspeed) - 1] = '\0';
    weather_data->current_weather_units.winddirection[sizeof(weather_data->current_weather_units.winddirection) - 1] = '\0';


    /* Now we need the arrays from the hourly data (hourly and hourly_units) */

    cJSON *hourly = cJSON_GetObjectItem(root, "hourly");
    if (!cJSON_IsObject(hourly)) {
        fprintf(stderr, "hourly data is not an object or cannot be found.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    cJSON *time_array = cJSON_GetObjectItem(hourly, "time");
    cJSON *temperature_2m_array = cJSON_GetObjectItem(hourly, "temperature_2m");
    cJSON *relativehumidity_2m_array = cJSON_GetObjectItem(hourly, "relativehumidity_2m");
    cJSON *windspeed_10m_array = cJSON_GetObjectItem(hourly, "windspeed_10m");

    if (!cJSON_IsArray(time_array) || !cJSON_IsArray(temperature_2m_array) || !cJSON_IsArray(relativehumidity_2m_array) || !cJSON_IsArray(windspeed_10m_array)) {
        fprintf(stderr, "Invalid hourly data in JSON response.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    int num_points = cJSON_GetArraySize(time_array);
    if (num_points > HOURLY_DATA_POINTS) {
        fprintf(stderr, "Number of hourly points exceeds the limits.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    for (int i = 0; i < num_points; i++) {
        /* First we get all of the items inside the arrays */
        cJSON *time_item = cJSON_GetArrayItem(time_array, i);
        cJSON *temperature_item = cJSON_GetArrayItem(temperature_2m_array, i);
        cJSON *relativehumidity_item = cJSON_GetArrayItem(relativehumidity_2m_array, i);
        cJSON *windspeed_item = cJSON_GetArrayItem(windspeed_10m_array, i);

        if (!cJSON_IsString(time_item) || 
            !cJSON_IsNumber(temperature_item) || 
            !cJSON_IsNumber(relativehumidity_item) || 
            !cJSON_IsNumber(windspeed_item)) {
            fprintf(stderr, "Invalid data in hourly arrays.\n");
            cJSON_Delete(root);
            return E_WEATHER_INVALID_DATA;
        }

        /* Now since it has been validated now we can copy it into the weather_data structure */
        strncpy(weather_data->hourly.time[i], time_item->valuestring, sizeof(weather_data->hourly.time[i]) - 1);
        weather_data->hourly.temperature_2m[i] = temperature_item->valueint;
        weather_data->hourly.relativehumidity_2m[i] = relativehumidity_item->valueint;
        weather_data->hourly.windspeed_10m[i] = windspeed_item->valuedouble;
        weather_data->hourly.time[i][sizeof(weather_data->hourly.time[i]) - 1] = '\0';
    }

    cJSON *hourly_units = cJSON_GetObjectItem(root, "hourly_units");

    if (!cJSON_IsObject(hourly_units)) {
        fprintf(stderr, "hourly_units data is not an object or cannot be found.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    cJSON *time_units = cJSON_GetObjectItem(hourly_units, "time");
    cJSON *temperature_units = cJSON_GetObjectItem(hourly_units, "temperature_2m");
    cJSON *relativehumidity_units = cJSON_GetObjectItem(hourly_units, "relativehumidity_2m");
    cJSON *windspeed_unit = cJSON_GetObjectItem(hourly_units, "windspeed_10m");
    if (!cJSON_IsString(time_units) ||
        !cJSON_IsString(temperature_units) ||
        !cJSON_IsString(relativehumidity_units) ||
        !cJSON_IsString(windspeed_unit)) {
        fprintf(stderr, "Invalid hourly_units data in JSON response.\n");
        cJSON_Delete(root);
        return E_WEATHER_INVALID_DATA;
    }

    strncpy(weather_data->hourly_units.time, time_units->valuestring, sizeof(weather_data->hourly_units.time) - 1);
    strncpy(weather_data->hourly_units.temperature_2m, temperature_units->valuestring, sizeof(weather_data->hourly_units.temperature_2m) - 1);
    strncpy(weather_data->hourly_units.relativehumidity_2m, relativehumidity_units->valuestring, sizeof(weather_data->hourly_units.relativehumidity_2m) - 1);
    strncpy(weather_data->hourly_units.windspeed_10m, windspeed_unit->valuestring, sizeof(weather_data->hourly_units.windspeed_10m) - 1);

    weather_data->hourly_units.time[sizeof(weather_data->hourly_units.time) - 1] = '\0';
    weather_data->hourly_units.temperature_2m[sizeof(weather_data->hourly_units.temperature_2m) - 1] = '\0';
    weather_data->hourly_units.relativehumidity_2m[sizeof(weather_data->hourly_units.relativehumidity_2m) - 1] = '\0';
    weather_data->hourly_units.windspeed_10m[sizeof(weather_data->hourly_units.windspeed_10m) - 1] = '\0';

    cJSON_Delete(root);

    return E_WEATHER_SUCCESS;
}


