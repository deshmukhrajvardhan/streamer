#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* curl stuff */
#include <curl/curl.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_HEADERDATA */
  return nitems * size;
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  //  printf("\nWriting: %d Bytes in memory",mem->size);                                            

  return realsize;
}

int main(){
  //check nghttp2 supprt                                                                            
  const curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
  if(data->features & CURL_VERSION_HTTP2){
    fprintf(stdout, "This libcurl DOES have HTTP2 support!\n");
  } else {
    fprintf(stdout, "This libcurl does NOT have HTTP/2 support!\n");
  }
  CURLcode res;

  struct MemoryStruct chunk;

  // Create multi handle with multiplex over a single connection                                    
  CURLM *multi_handle = curl_multi_init();
  curl_multi_setopt(multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS, (long) 1L);
  curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);

  // Add some requests                                                                              
  //int NUM_HANDLES = 1;                                                                            
  int NUM_HANDLES = 2;
  CURL *easy[NUM_HANDLES];
  for (int j = 0; j < 2; j++) {

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    for (int i = 0; i < NUM_HANDLES; i++) {
      char url[1024];

      snprintf(url, 1024, "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/Bi\
gBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s%d.m4s",((j*2)+(i+1)));
      //snprintf(url, 1024, "https://http2.akamai.com/demo");                                       
      easy[i] = curl_easy_init();
      curl_easy_setopt(easy[i], CURLOPT_VERBOSE, 1L);
      printf("\nurl:%s",url);
      curl_easy_setopt(easy[i], CURLOPT_URL, url);
      curl_easy_setopt(easy[i], CURLOPT_SSLCERTTYPE, "PEM");
      curl_easy_setopt(easy[i], CURLOPT_CAINFO, "cert.pem");
      curl_easy_setopt(easy[i], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
      curl_easy_setopt(easy[i], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
      /* send all data to this function  */
      curl_easy_setopt(easy[i], CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

      /* we pass our 'chunk' struct to the callback function */
      curl_easy_setopt(easy[i], CURLOPT_WRITEDATA, (void *)&chunk);

      curl_multi_add_handle(multi_handle, easy[i]);
    }

    //perform requests                                                                              
    int still_running = 1;
    int once=1;
    int number_of_streams=0;
    double dupe=0;
    while(still_running){
      res=curl_multi_perform(multi_handle, &still_running);
            if(!res && number_of_streams<NUM_HANDLES) {
        /* check the size */
        double cl;
        res = curl_easy_getinfo(easy[number_of_streams], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);
        if(cl>0 && dupe!=cl) {
          dupe=cl;
          number_of_streams++;
          if(!res) {
            printf("--------------Size: %.0f---------------\n", cl);
          }
        }
      }
          }

    free(chunk.memory); // essentially data from parallel streams is stored in memory               
                        // and cleared when we move to next set of parallel stream downloads        

  }
    //cleanups                                                                                      
    for(int i = 0; i < NUM_HANDLES; i++)
      curl_easy_cleanup(easy[i]);
    curl_multi_cleanup(multi_handle);

  exit(0);
}
