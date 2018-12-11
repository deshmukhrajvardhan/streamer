//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <curl/curl.h>

#include <iostream>
#include <future>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

#include <fstream>

#include "time_snippet.h"

#ifdef _WIN32
#define SHORT_SLEEP Sleep(100)
#else
#define SHORT_SLEEP usleep(100000)
#endif

#define MAX_CHUNK_SIZE 15000
#define ORIG_EASY 0
#define NO_HANDLE 2

  /* curl stuff */

using std::string;

struct HandleChange {
  int prev_run,still_running,change,seg_num;
  double content_len;
  string url;
  size_t chunk_size;
} handleChange;

int ReadMsg(char *myfifor,char *myfifow, bool stream, key_t key){
  //std::cout << "EnteredRead\n";
  int msgid;
  struct mesg_buffer {
    long mesg_type;
    char mesg_text[200];
  };
  mesg_buffer msg;
  int cmd;
  msgid = msgget(key, 0777 | IPC_CREAT);
  ssize_t numbytes=0;
  //std::cout<<"Waiting to read url\n";
  while (numbytes==0){
    //    numbytes=msgrcv(msgid, &msg, sizeof(msg.mesg_text),1,1);
    numbytes=msgrcv(msgid, &msg, sizeof(msg.mesg_text),1,1);
    //    std::cout<<"numb:"<<numbytes<<std::endl;
  }
  string message(msg.mesg_text, numbytes);
  //std::cout<<"Received:"<< message<<"\t"<<std::endl;
  if (numbytes==-1) {
    //    std::cout<<"IPC_NOWAIT";
    cmd = -1;
    return cmd;
  }
  else {
    string message(msg.mesg_text, numbytes);
    //  std::cout<<"Received:"<< message<<"\t"<<std::endl;
    if (message.compare("QUIT")==0){
        std::cout << "QUIT" << std::endl;
        cmd = -2;
        return cmd;
    }
    else {
    //    std::cout<<"Received:"<< message<<"\t"<<std::endl;
    cmd = 1;
    handleChange.url.assign(message);
    return cmd;
    }
  }
}

// Write Msg
int WriteMsg(string myfifow, int cmd, key_t key){
  int msgid;
  struct mesg_buffer {
    long mesg_type;
    char mesg_text[128];
  } message;
  msgid = msgget(key, 0777 | IPC_CREAT);
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

  mem->memory = (char*) realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
  /* out of memory! */
  printf("not enough memory (realloc returned NULL)\n");
  return 0;
    }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  
  handleChange.chunk_size+= realsize;
  //  printf("\nWriting: %zu Bytes in memory",handleChange.chunk_size);                                            

  if(handleChange.chunk_size>=MAX_CHUNK_SIZE) {
        string orig_chunk_size = std::to_string(handleChange.chunk_size);
        key_t key_c_orig_w = 662145;
        int read_exec = 1;
        auto future = std::async(WriteMsg, orig_chunk_size, read_exec, key_c_orig_w);
        auto write_ret = future.get();
        handleChange.chunk_size=0;
    }

return realsize;
}

int main(){
    //check nghttp2 supprt
    //std::fstream file_download_rate;
    //file_download_rate.open("./cAppRate.txt",std::fstream::out|std::fstream::app);
    //  std::fstream("example.txt",std::fstream::out|std::fstream::app)<<"logging enabled\n";
    //file_download_rate.close();

  //    const curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
    //    if(data->features & CURL_VERSION_HTTP_1_1){
    //fprintf(stdout, "This libcurl DOES have HTTP2 support!\n");
    //}
  std::cout<<"HTTPS1_1 Slave Online!\n";
    // CURLcode res;
    CURLcode res_orig, res_retx;
    CURLMsg *multi_msg=NULL;

    struct MemoryStruct chunk;

    // Create multi handle with multiplex over a single connection                                    
    CURLM *multi_handle = curl_multi_init();
    curl_multi_setopt(multi_handle, CURLMOPT_MAX_HOST_CONNECTIONS, (long) 1L);
    //    curl_multi_setopt(multi_handle, CURLPIPE_NOTHING, CURLPIPE_HTTP1);

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

    handleChange.chunk_size=0; // get chunks of almost fixed size
    handleChange.seg_num=0; // count total number of segments
    handleChange.still_running=0;
    handleChange.change=0;
    // Add some requests                                                                              
    //int NUM_HANDLES = 1;
    int orig_easy = 0;
    int retx_easy =0;

    double cl_orig = -1; //header len, struct didn't work as it changes in write callback

    // write ipc
    key_t key_c_orig_w = 662145;


    // read orig_url from ipc
    char *myfifor_orig = (char*)"./fifopipe_orig";
    char *myfifow_orig = (char*)"/tmp/fifowpipe_orig";
    key_t key_c_orig_r=662144;
    bool stream_send=false;

    int current_orig_handle = NO_HANDLE;
    double orig_content_len;
    int NUM_HANDLES = 1;
    CURL *easy[NUM_HANDLES];
    int i = 0;
    //  int orig_done = 1;
    //int retx_done = 1;
    int j = 0;

    //std::fstream file_download_rate;
    //  file_download_rate.open("./cAppRate.txt",std::fstream::out|std::fstream::app);
    //file_download_rate<<"logging enabled\n";
    //  for (int j = 1; j < 149; j++) {    
    chunk.memory = (char*) malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    uint64 orig_multi_perf_to_done = 0;
    i = 0;                                                           
    do {
        // diff condition
        if (orig_easy==0){
            std::future<int> future_orig_r = std::async(ReadMsg, myfifor_orig,myfifow_orig, stream_send, key_c_orig_r);
            int read_ret_orig = future_orig_r.get();
	    //	    std::cout<<read_ret_orig<<"<-ret(1)in orig_url getter\n";
            //int read_ret_orig = ReadMsg(myfifor_orig,myfifow_orig, stream_send, key_c_orig_r);
            //std::cout<<"\nurl:"<<read_ret_orig<<std::endl;
            /*    if (read_ret_orig==-2) {
              break;
              }*/
            if (read_ret_orig==1) {
                //std::cout<<"\nurl:"<<read_ret_orig<<std::endl;
                //add url to easy handle and multi handle
	      //std::cout<<handleChange.url<<"In orig easy\n";
                char url[1024];

                snprintf(url, 1024, "%s", handleChange.url.c_str());
                easy[ORIG_EASY] = curl_easy_init(); //easy_handle is sticky
                //curl_easy_setopt(easy[ORIG_EASY], CURLOPT_VERBOSE, 1L);
		//                printf("\nurl:%s",url);
		std::cout<<"\nurl:"<<url<<"\n";
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_URL, url);
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_SSLCERTTYPE, "PEM");
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_CAINFO, "cert.pem");
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                curl_easy_setopt(easy[ORIG_EASY], CURLOPT_WRITEDATA, (void *)&chunk);
                curl_multi_add_handle(multi_handle, easy[ORIG_EASY]);
                orig_easy=1;
		//std::cout<<"going into multiperform\n";
                //std::cout<<"\nTime from ipc url read until 1st multi_perform:"<<GetTimeMs64()-orig_url_to_multi_perf<<"\n";
                orig_multi_perf_to_done = GetTimeMs64();
                //      curl_multi_perform(multi_handle, &handleChange.still_running);
              
                }
        }
        
     
        /* we start some action by calling perform right away */
        //int still_running;
        if (orig_easy==1){ // multiperform only if url set 
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
		    //		    std::cout<<"Running multi-perform\n";
                    // descriptive
                    //tells if end

                    res_orig = curl_easy_getinfo(easy[ORIG_EASY], CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl_orig);

                    if(cl_orig>0) {
                        orig_content_len = cl_orig;
			//std::cout<<orig_content_len<<"<-Content_len\n";
                    }

		    int msgq = 0;
		    multi_msg = curl_multi_info_read(multi_handle, &msgq);

                    if(multi_msg && (multi_msg->msg == CURLMSG_DONE)) {
                        current_orig_handle = ORIG_EASY;
                        printf("\nOrig_Change\n");
                    }
                      // individual response end                                                                 
                    if(current_orig_handle == ORIG_EASY) {
                        // Get Content-length from header

                        printf("\n--------------Orig_Size: %.0f---------------\n", orig_content_len);
                        std::fstream("Http1CppAppDwRate.txt",std::fstream::out|std::fstream::app)<< orig_content_len/1000000<< ',' << (orig_content_len*8*1000)/((GetTimeMs64()-orig_multi_perf_to_done)*(1000000))<< "\n";

                        string last_chunk_size = std::to_string(handleChange.chunk_size);
                        int read_exec = 1; //last chunk
                        auto future = std::async(WriteMsg, last_chunk_size, read_exec, key_c_orig_w);
                        auto write_ret = future.get();
                        //    
                        string orig_segment_size = std::to_string(orig_content_len);//cl_orig);
                        read_exec = 2; //end of segment
                        future = std::async(WriteMsg, orig_segment_size, read_exec, key_c_orig_w);
                        write_ret = future.get();
                        handleChange.chunk_size=0;
                        std::cout<<"\nclearing orig_mem\n";            
                        free(chunk.memory); // and cleared when we move to next set of parallel stream downloads 
                        // memory assigned before it is actually needed        
                        chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
                        chunk.size = 0;    /* no data at this point */;
                        orig_easy=0;
                        current_orig_handle = NO_HANDLE;

                    }
            }//#descriptive

        }
    } while(1);
    //free(chunk.memory); // essentially data from parallel streams is stored in memory               
                    // and cleared when we move to next set of parallel stream downloads        
    //cleanups
    //  file_download_rate.close();                   
    for(int i = 0; i < NUM_HANDLES; i++)
        curl_easy_cleanup(easy[i]);
    curl_multi_cleanup(multi_handle);

    exit(0);
}


