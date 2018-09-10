#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define SHORT_SLEEP Sleep(100)
#else
#define SHORT_SLEEP usleep(100000)
#endif

/* curl stuff */
#include <curl/curl.h>

#define MAX_CHUNK_SIZE 15000

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct HandleChange {
  int prev_run,still_running,change,seg_num;
  double content_len;
  size_t chunk_size;
} handleChange;

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
  
  
  //  handleChange.chunk_size+= realsize;

  if(handleChange.change) {
    printf("\nWrite Still_running:%d,Segment num:%d",handleChange.still_running,++handleChange.seg_num);//,handleChange.content_len);,content_length:%.0f
    //    printf("\n ChunkSize:%d",realsize);//handleChange.chunk_size);
    handleChange.change=0;
    //    handleChange.chunk_size=0;
  }
    printf("\n ChunkSize:%d",realsize);//handleChange.chunk_size);

  //  if(handleChange.chunk_size>=MAX_CHUNK_SIZE) {
  //    printf("\n ChunkSize:%d",handleChange.chunk_size);
  //    handleChange.chunk_size=0;
  //  }

  //  handleChange.chunk_size+= realsize;


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
  CURL *eh=NULL;
  CURLMsg *msg=NULL;
  CURLcode return_code=0;
  int i=0, msgs_left=0;
  int http_status_code;
  const char *szUrl;

  struct MemoryStruct chunk;

  // Create multi handle with multiplex over a single connection                                   
  CURLM *multi_handle = curl_multi_init();
  curl_multi_setopt(multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS, (long) 1L);
  curl_multi_setopt(multi_handle, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);

  //fd                                                                                             
  struct timeval timeout;
  int rc;
  fd_set fdread;
  CURLMcode mc;
  fd_set fdwrite;
  fd_set fdexcep;
  int maxfd = -1;

  long curl_timeo;

  curl_multi_timeout(multi_handle, &curl_timeo);
  if(curl_timeo < 0)
    curl_timeo = 1000;

  timeout.tv_sec = curl_timeo / 1000;
  timeout.tv_usec = (curl_timeo % 1000) * 1000;

  FD_ZERO(&fdread);
  FD_ZERO(&fdwrite);
  FD_ZERO(&fdexcep);

  /* get file descriptors from the transfers */
  mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
  int NUM_HANDLES = 2;
  CURL *easy[NUM_HANDLES];
  handleChange.seg_num=0; // count total number of segments
  handleChange.chunk_size=0; // get chunks of almost fixed size 
  for (int j = 0; j < 2; j++) {

    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    for (int i = 0; i < NUM_HANDLES; i++) {
      char url[1024];

      snprintf(url, 1024, "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s%d.m4s",((j*2)+(i+1)));
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
    /* we start some action by calling perform right away */
    handleChange.still_running=0;
    handleChange.change=1;

    curl_multi_perform(multi_handle, &handleChange.still_running);
    handleChange.prev_run=handleChange.still_running;

    int cycle=0;
    double cl; //header len, struct didn't work as it changes in write callback                    
 
    do {
      struct timeval timeout;
      int rc; /* select() return code */
      CURLMcode mc; /* curl_multi_fdset() return code */

      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd = -1;

      long curl_timeo = -1;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);

      /* set a suitable timeout to play around with */
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      curl_multi_timeout(multi_handle, &curl_timeo);
      if(curl_timeo >= 0) {
        timeout.tv_sec = curl_timeo / 1000;
        if(timeout.tv_sec > 1)
          timeout.tv_sec = 1;
        else
          timeout.tv_usec = (curl_timeo % 1000) * 1000;
      }

      /* get file descriptors from the transfers */
      mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

      if(mc != CURLM_OK) {
        fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
        break;
      }
      //fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);                               

      /* On success the value of maxfd is guaranteed to be >= -1. We call                           
       select(maxfd + 1, ...); specially in case of (maxfd == -1) there are                         
       no fds ready yet so we call select(0, ...) --or Sleep() on Windows--                         
       to sleep 100ms, which is the minimum suggested value in the                                  
       curl_multi_fdset() doc. */

      if(maxfd == -1) {
#ifdef _WIN32
        Sleep(100);
        rc = 0;
#else
        /* Portable sleep for platforms other than Windows. */
        struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
        rc = select(0, NULL, NULL, NULL, &wait);
#endif
      }
      else {
        /* Note that on some platforms 'timeout' may be modified by select().                      
           If you need access to the original value save a copy beforehand. */
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
      }

      switch(rc) {
      case -1:
        /* select error */
        break;
      case 0:
      default:
        /* timeout or readable/writable sockets */
        curl_multi_perform(multi_handle, &handleChange.still_running);

        res = curl_easy_getinfo(easy[(NUM_HANDLES-1)-handleChange.still_running], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);

        // individual response end                                                                 
        if(handleChange.prev_run!=handleChange.still_running) {
          printf("\nStill_running:%d",handleChange.still_running);
          handleChange.change=1; //count segments and get streamID                          
          if(!res) {
	    handleChange.content_len = cl;
            printf("--------------Size: %.0f---------------\n", handleChange.content_len);
          }

        }
        handleChange.prev_run=handleChange.still_running;
        break;
      }
      //      printf("\nin cycle:%d\n",cycle++);                                                  
 
    } while(handleChange.still_running);
        
    free(chunk.memory); // essentially data from parallel streams is stored in memory              
                        // and cleared when we move to next set of parallel stream downloads       
  }
  //cleanups                                                                                      
  for(int i = 0; i < NUM_HANDLES; i++)
    curl_easy_cleanup(easy[i]);
  curl_multi_cleanup(multi_handle);

  exit(0);
}
