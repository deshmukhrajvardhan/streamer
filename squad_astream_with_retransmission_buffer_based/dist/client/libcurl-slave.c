#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <future>
/* curl stuff */
#include <curl/curl.h>

/* ipc */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>

using std::string;
//using std::cout;
// Read Msg

string ReadMsg(char *myfifor,char *myfifow, bool stream, key_t key){
  std::cout << "EnteredRead\n";
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
    numbytes=msgrcv(msgid, &msg, sizeof(msg.mesg_text),1,1);
    std::cout<<"numb:"<<numbytes<<std::endl;
  }
  string message(msg.mesg_text, numbytes);
  std::cout<<"Received:"<< message<<"\t"<<std::endl;
  if (message.compare("QUIT")==0){
    std::cout << "QUIT" << std::endl;
  }

  else if (message.compare("CREATE_CONN")==0){
    cmd="conn";
    return cmd;
  }
  else if (message.compare("ABANDON")==0){
    cmd="abandon";
    return cmd;
  }
  else if (message.compare("CREATE_STREAM")==0){
    cmd="stream";
    //unlink(myfifor);
    std::cout << "Stream created" << std::endl;
    return cmd;
  }
  else {
    std::cout <<"Stream Received" << message << std::endl;
    return message;

    //unlink(myfifor);
  }

  //unlink(myfifo);
  return "done";

}

// Write Msg
int WriteMsg(char* myfifow, int cmd, key_t key){
  int msgid;
  struct mesg_buffer {
    long mesg_type;
    char mesg_text[128];
  } message;
  msgid = msgget(key, 0666 | IPC_CREAT);
  message.mesg_type = 1;

  if (cmd==1){
    string msg="CONN_CREATED";
    strcpy(message.mesg_text,msg.c_str());
  }
  else {
    string msg="FAIL";
    strcpy(message.mesg_text,msg.c_str());
  }

  msgsnd(msgid, &message, sizeof(message), 0);
  return 0;
}


/* libcurl code */
struct MemoryStruct {
  char *memory;
  size_t size;
};

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
  
  //printf("\nWriting: %d Bytes in memory",mem->size);
  std::cout << "\nWriting:"<< mem->size <<"Bytes in memory\n";

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
  int NUM_HANDLES = 1;
  CURL *easy[NUM_HANDLES];
  //  for (int i = 0; i < NUM_HANDLES; i++) {
    //easy[i] = curl_easy_init();
  //} 
  /*  curl_easy_setopt(easy[i], CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(easy[i], CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(easy[i], CURLOPT_CAINFO, "cert.pem");
    curl_easy_setopt(easy[i], CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(easy[i], CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
  }*/

  //  for (int j = 1; j < 149; j++) {
  //for (int j = 0; j < 74; j++) {
  // get url
  char *myfifor = (char*)"./fifopipe";
  char *myfifow = (char*)"/tmp/fifowpipe";
  key_t key_py_r=262145;
  bool stream_send=false;

  auto future2 = std::async(ReadMsg, myfifor,myfifow, stream_send, key_py_r);
  auto read_ret = future2.get();
  if (read_ret=="conn"){
    future2 = std::async(ReadMsg, myfifor,myfifow, stream_send, key_py_r);
    auto host = future2.get();
    future2 = std::async(ReadMsg, myfifor,myfifow, stream_send, key_py_r);
    int port = std::stoi(future2.get());
    future2 = std::async(ReadMsg, myfifor,myfifow, stream_send, key_py_r);
    auto urls=future2.get();
    //const base::CommandLine::StringVector& urls = &ur_res;
  }

  //



  chunk.memory = (char *)malloc(1);  /* will be grown as needed by the realloc above */
  chunk.size = 0;    /* no data at this point */

    for (int i = 0; i < NUM_HANDLES; i++) {
      char url[1024];

      snprintf(url, 1024, "http://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s.mpd"); //get in the file path
//"https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s%d.m4s",((j*2)+(i+1)));
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
    while(still_running){
      curl_multi_perform(multi_handle, &still_running);
    }
    
    free(chunk.memory); // essentially data from parallel streams is stored in memory 
                        // and cleared when we move to next set of parallel stream downloads
    //}
    //cleanups
    for(int i = 0; i < NUM_HANDLES; i++)
      curl_easy_cleanup(easy[i]);
    curl_multi_cleanup(multi_handle);

  exit(0);
}

