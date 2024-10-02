
#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int download_file(const char *url) {
  CURL *curl_handle;
  CURLcode res;

  struct MemoryStruct chunk;

  char useragent[256];

  chunk.memory = malloc(1);  // will be grown as needed by the realloc above
  chunk.size = 0;    // no data at this point

  curl_global_init(CURL_GLOBAL_ALL);

  // init the curl session
  curl_handle = curl_easy_init();

  // specify URL to get
  curl_easy_setopt(curl_handle, CURLOPT_URL, url);

  // send all data to this function
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

  // we pass our 'chunk' struct to the callback function
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

  // some servers don't like requests that are made without a user-agent field, so we provide one
  snprintf(useragent, 256, "%s/%s", prog, VERSION);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, useragent);

  // get it!
  res = curl_easy_perform(curl_handle);

  // check for errors
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
    printf("Data:\n%s\n", chunk.memory);
  }

  // cleanup curl stuff
  curl_easy_cleanup(curl_handle);

  free(chunk.memory);

  // we're done with libcurl, so clean it up
  curl_global_cleanup();

  return 0;
}
