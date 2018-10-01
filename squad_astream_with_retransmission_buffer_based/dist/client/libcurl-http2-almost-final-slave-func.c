#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* ipc */

#include <iostream>
#include <future>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#define SHORT_SLEEP Sleep(100)
#else
#define SHORT_SLEEP usleep(100000)
#endif

/* curl stuff */
#include <curl/curl.h>

#define MAX_CHUNK_SIZE 15000
#define ORIG_EASY 0
#define RETX_EASY 1
#define NO_HANDLE 2
/* IPC API's */

using std::string;
//using std::cout;
// Read Msg

string ReadMsg(char *myfifor,char *myfifow, bool stream, key_t key){
  //std::cout << "EnteredRead\n";
  int msgid;
  struct mesg_buffer {
    long mesg_type;
    char mesg_text[200];
  };
  mesg_buffer msg;
  string cmd="";
  msgid = msgget(key, 0666 | IPC_CREAT);
  ssize_t numbytes=0;
  while (numbytes==0){
    //    numbytes=msgrcv(msgid, &msg, sizeof(msg.mesg_text),1,1);
    numbytes=msgrcv(msgid, &msg, sizeof(msg.mesg_text),1,IPC_NOWAIT);
    //    std::cout<<"numb:"<<numbytes<<std::endl;
  }
  if (numbytes==-1) {
    //    std::cout<<"IPC_NOWAIT";
    cmd = "-1";
    return cmd;
    }
  string message(msg.mesg_text, numbytes);
  //  std::cout<<"Received:"<< message<<"\t"<<std::endl;
  if (message.compare("QUIT")==0){
    std::cout << "QUIT" << std::endl;
    exit(1);
  }
  else {
    //    std::cout<<"Received:"<< message<<"\t"<<std::endl;
    return message;
  }

}

// Write Msg
int WriteMsg(string myfifow, int cmd, key_t key){
  int msgid;
  struct mesg_buffer {
    long mesg_type;
    char mesg_text[128];
  } message;
  msgid = msgget(key, 0666 | IPC_CREAT);
  message.mesg_type = 1;

  if (cmd==1){
    string msg="CONN_CREATED:";
    msg+=myfifow+":";
    strcpy(message.mesg_text,msg.c_str());
    //strcpy(message.mesg_text,msg.c_str());
  }
  else if (cmd==2) {
     string msg="end:";
     msg+=myfifow+":";
     strcpy(message.mesg_text,msg.c_str());
  }
  else {
    string msg="FAIL";
    strcpy(message.mesg_text,msg.c_str());
  }

  msgsnd(msgid, &message, sizeof(message), 0);
  return 0;
}

/* Curl API's */
struct MemoryStruct {
  char *memory;
  size_t size;
};

struct HandleChange {
  int prev_run,still_running,change,seg_num;
  double content_len;
  size_t chunk_size,retx_chunk_size;
} handleChange;

static size_t header_callback(char *buffer, size_t size,
                              size_t nitems, void *userdata)
{
  /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
  /* 'userdata' is set with CURLOPT_HEADERDATA */
  return nitems * size;
}

static size_t
WriteMemoryCallback2(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
   
  if(handleChange.change) {
    printf("\nCallback2:Write Still_running:%d,Segment num:%d,Size:%zu",handleChange.still_running,++handleChange.seg_num,handleChange.retx_chunk_size);//,handleChange.content_len);,content_length:%.0f
    //    printf("\n ChunkSize:%d",realsize);//handleChange.chunk_size);
    handleChange.change=0;
    //    handleChange.retx_chunk_size=0;
  }
  //  printf("\n C2:ChunkSize:%d",realsize);//handleChange.chunk_size);
  handleChange.retx_chunk_size+= realsize;

  // To get chunks of approx 15000 and send them via ipc c_w_orig 
  if(handleChange.retx_chunk_size>=MAX_CHUNK_SIZE) {
    string retx_chunk_size = std::to_string(handleChange.retx_chunk_size);
    key_t key_c_retx_w=462146;
    int read_exec = 1;
    auto future = std::async(WriteMsg, retx_chunk_size, read_exec, key_c_retx_w);
    auto write_ret = future.get();
    handleChange.retx_chunk_size=0;
  }


  return realsize;
}


static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->memory = (char *) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  
  if(handleChange.change) {
    printf("\nCallback1:Write Still_running:%d,Segment num:%d, Size:%zu",handleChange.still_running,++handleChange.seg_num,handleChange.chunk_size);
    //    printf("\n ChunkSize:%d",realsize);//handleChange.chunk_size);
    handleChange.change=0;
  }
  //printf("\n C1:ChunkSize:%d",realsize);//handleChange.chunk_size);
  handleChange.chunk_size+= realsize;
  
  // To get chunks of approx 15000 and send them via ipc c_w_orig 
  if(handleChange.chunk_size>=MAX_CHUNK_SIZE) {
    string orig_chunk_size = std::to_string(handleChange.chunk_size);
    key_t key_c_orig_w = 262145;
    int read_exec = 1;
    auto future = std::async(WriteMsg, orig_chunk_size, read_exec, key_c_orig_w);
    auto write_ret = future.get();
    handleChange.chunk_size=0;
  }

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

  CURLcode res_orig, res_retx;
  CURL *eh=NULL;
  CURLMsg *msg=NULL;
  int i=0, msgs_left=0;
  int http_status_code;
  const char *szUrl;

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

  //curl_multi_timeout(multi_handle, &curl_timeo);
  if(curl_timeo < 0)
    curl_timeo = 1000;

  timeout.tv_sec = curl_timeo / 1000;
  timeout.tv_usec = (curl_timeo % 1000) * 1000;

  FD_ZERO(&fdread);
  FD_ZERO(&fdwrite);
  FD_ZERO(&fdexcep);
  
  struct MemoryStruct chunk;
  struct MemoryStruct retx_chunk;	

  /* get file descriptors from the transfers */
  mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
  int NUM_HANDLES = 2;
  CURL *easy[NUM_HANDLES];

  handleChange.seg_num=0; // count total number of segments
  handleChange.chunk_size=0; // get chunks of almost fixed size 
  
  int num_current_orig_urls = 0;
  int num_current_retx_urls = 0;
  int current_handle = NO_HANDLE;
  /* we start some action by calling perform right away */
  handleChange.still_running=0;
  handleChange.retx_chunk_size=0;
  handleChange.chunk_size=0;
  handleChange.change=1;

  //    curl_multi_perform(multi_handle, &handleChange.still_running);
  handleChange.prev_run=1;//handleChange.still_running;

  int cycle = 0;
  double cl_orig = -1; //header len, struct didn't work as it changes in write callback
  double cl_retx = -1;

  // write ipc
  key_t key_c_orig_w = 262145;
  key_t key_c_retx_w=462146;

  // read orig_url from ipc
  char *myfifor_orig = (char*)"./fifopipe_orig";
  char *myfifow_orig = (char*)"/tmp/fifowpipe_orig";
  key_t key_c_orig_r=262144;
  bool stream_send=false;

  // read retx_url from ipc
  char *myfifor_retx = (char*)"./fifopipe_retx";
  char *myfifow_retx = (char*)"/tmp/fifowpipe_retx";
  key_t key_c_retx_r=362146;
     
  //malloc before
  chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */
  retx_chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
  retx_chunk.size = 0;    /* no data at this point */

  // Get url from py master and download files from server in non blocking manner
    do {
      // std::cout<<"\nIn the loop"<<num_reads--<<"\n";
      auto future_orig_r = std::async(ReadMsg, myfifor_orig,myfifow_orig, stream_send, key_c_orig_r);
      auto read_ret_orig = future_orig_r.get();
      if (read_ret_orig.compare("-1")!=0) {
	//std::cout<<"\nurl:"<<read_ret_orig<<std::endl;
	//add url to easy handle and multi handle
	char url[1024];

	snprintf(url, 1024, "%s", read_ret_orig.c_str());
        easy[ORIG_EASY] = curl_easy_init(); //easy_handle is sticky
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_VERBOSE, 1L);
	printf("\nurl:%s",url);
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_URL, url);
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_SSLCERTTYPE, "PEM");
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_CAINFO, "cert.pem");
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(easy[ORIG_EASY], CURLOPT_WRITEDATA, (void *)&chunk);
	curl_multi_add_handle(multi_handle, easy[ORIG_EASY]);
	num_current_orig_urls+=1;
      }
      auto future_retx_r = std::async(ReadMsg, myfifor_retx,myfifow_retx, stream_send, key_c_retx_r);
      auto read_ret_retx = future_retx_r.get();
      if (read_ret_retx.compare("-1")!=0) {
	//std::cout<<"\nRetx_url:"<<read_ret_retx<<std::endl;
	//add url to easy handle and multi handle
	char retx_url[1024];

	snprintf(retx_url, 1024, "%s", read_ret_retx.c_str());
	easy[RETX_EASY] = curl_easy_init(); //easy_handle is sticky

	curl_easy_setopt(easy[RETX_EASY], CURLOPT_VERBOSE, 1L);
	printf("\nRetx_url:%s",retx_url);
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_URL, retx_url);
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_SSLCERTTYPE, "PEM");
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_CAINFO, "cert.pem");
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

	/* send all data to this function  */
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_WRITEFUNCTION, WriteMemoryCallback2);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(easy[RETX_EASY], CURLOPT_WRITEDATA, (void *)&retx_chunk);
	curl_multi_add_handle(multi_handle, easy[RETX_EASY]);
	num_current_retx_urls+=1;
      }
      //IPC read complete
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
	// TODO: Could cause a problem (wait for download and not execute ipc read)
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

	// Get Content-length from header
	if(num_current_orig_urls>0 ) {
	  res_orig = curl_easy_getinfo(easy[ORIG_EASY], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl_orig);
	  current_handle = ORIG_EASY;
	}
	if(num_current_retx_urls>0 ) {
	  res_retx = curl_easy_getinfo(easy[RETX_EASY], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl_retx);
	  current_handle = RETX_EASY;
	}
        // individual response end                                                                 
        if((handleChange.prev_run!=handleChange.still_running)&&(current_handle!=NO_HANDLE)) {
          printf("\nStill_running:%d",handleChange.still_running);
          handleChange.change=1; //count segments and get streamID

	  // Get Content-length from header
	  if(!res_orig) {
	    handleChange.content_len = cl_orig;
	    printf("\n--------------Orig_Size: %.0f---------------\n", handleChange.content_len);

	    if(cl_orig==-1){
	      handleChange.prev_run=handleChange.still_running;
	      break;
	    }
	  }
	  if(!res_retx) {
	    handleChange.content_len = cl_retx;
	    printf("\n--------------Retx_Size: %.0f---------------\n", handleChange.content_len);

	    if(cl_retx==-1){
	      handleChange.prev_run=handleChange.still_running;
	      break;
	    }
	  }

	  // send content-length at the end of segment download
	  if(current_handle == ORIG_EASY) {
	    printf("\nMain1:Write Still_running:%d,Segment num:%d,Size:%zu",handleChange.still_running,handleChange.seg_num,handleChange.chunk_size);
	    
	    string last_chunk_size = std::to_string(handleChange.chunk_size);
	    int read_exec = 1; //last chunk
	    auto future = std::async(WriteMsg, last_chunk_size, read_exec, key_c_orig_w);
	    auto write_ret = future.get();
	    //
	    string orig_chunk_size = std::to_string(cl_orig);
	    read_exec = 2; //end of segment
	    future = std::async(WriteMsg, orig_chunk_size, read_exec, key_c_orig_w);
	    write_ret = future.get();
	    handleChange.chunk_size=0;
	    std::cout<<"\nclearing orig_mem\n";	            
	    free(chunk.memory); // and cleared when we move to next set of parallel stream downloads 
	    // memory assigned before it is actually needed        
	    chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
	    chunk.size = 0;    /* no data at this point */
	    current_handle=NO_HANDLE;
	    if(num_current_orig_urls>0) {
	      num_current_orig_urls-=1;
	    }
	  }
	  else {
	    printf("\nMain2:Write Still_running:%d,Segment num:%d,Size:%zu",handleChange.still_running,handleChange.seg_num,handleChange.retx_chunk_size);
	    //
	    string last_chunk_size = std::to_string(handleChange.retx_chunk_size);
	    int read_exec = 1; //last chunk
	    auto future = std::async(WriteMsg, last_chunk_size, read_exec, key_c_retx_w);
	    auto write_ret = future.get();
	    
	    //
	    string retx_chunk_size = std::to_string(cl_retx);
	    read_exec = 2; //end of segment
	    future = std::async(WriteMsg, retx_chunk_size, read_exec, key_c_retx_w);
	    write_ret = future.get();
	    handleChange.retx_chunk_size=0;
	    std::cout<<"\nclearing retx_mem\n";
	    free(retx_chunk.memory); // and cleared when we move to next set of parallel stream downloads
	    // memory assigned before it is actually needed  
	    retx_chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
	    retx_chunk.size = 0;    /* no data at this point */
	    current_handle = NO_HANDLE;
	    if(num_current_retx_urls>0) {
	      num_current_retx_urls-=1;
	    }
	  }
          
        }
        handleChange.prev_run=handleChange.still_running;
        break;
      //      printf("\nin cycle:%d\n",cycle++);                                                  
      }
    } while(1);// run this forever//handleChange.still_running);
        	
  //cleanups                                                                                      
  for(int i = 0; i < NUM_HANDLES; i++)
    curl_easy_cleanup(easy[i]);
  curl_multi_cleanup(multi_handle);

  exit(0);
}
