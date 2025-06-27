/*
* @Author: karlosiric
* @Date:   2025-06-26 14:39:26
* @Last Modified by:   karlosiric
* @Last Modified time: 2025-06-27 09:58:47
*/

#include "../include/weather.h"
#include <curl/curl.h>
#include <errno.h>


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
