import sysv_ipc
import sys
import errno
import timeit
from string import ascii_letters, digits
import read_mpd
import urllib
import random
import os

def write_msg(list_cmd,mq):
        for i in list_cmd:
                print (i)
                mq.send(i, True)
        return


def get_mpd(url):
    """ Module to download the MPD from the URL and save it to file"""
    keyr = 262144
    keyw = 262145
    keyw1=362146
    keyw2=462146
    keyr1=562146
    keyr2=662146
    try:
        mqr=sysv_ipc.MessageQueue(keyr, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    try:
        mqw=sysv_ipc.MessageQueue(keyw, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    cmd=["CREATE_CONN","10.10.1.1","443",url]
    cmd_cpp=""
    thread1=threading.Thread(target=write_msg,args=(cmd,mqw))
    thread1.start()
    thread1.join()
    #process1= Process(target=write_msg, args=(cmd,mqw))
    #process1.start()
    #process1.join()
    #os.system("sudo /dev/SQUAD/src/out/Test/quic_persistent_client --disable-certificate-verification 2>&1 > quic_out.txt &")
    message=mqr.receive()
    mpd_file = url.split('/')[-1]
    mpd_file=os.path.abspath(mpd_file)
    try:
        mqw1=sysv_ipc.MessageQueue(keyw1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    try:
        mqr1=sysv_ipc.MessageQueue(keyr1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    try:
        mqr1=sysv_ipc.MessageQueue(keyr1, sysv_ipc.IPC_CREAT, max_message_size = 15000)
    except:
        print ("ERROR: Queue not created")
    parse_url = urllib.parse.urlparse(url)
    cmd1=["CREATE_STREAM",parse_url.path,mpd_file]
    print (parse_url.path)
    print (mpd_file)
    print (str(message[0],'utf-8', errors='ignore').split('\x00')[0])
    if "CONN_CREATED" in (str(message[0],'utf-8', errors='ignore').split('\x00')[0]):
        print ("CONNECTION ESTABLISHED")
        thread3=threading.Thread(target=write_msg,args=(cmd1,mqw1))
        thread3.start()
        while True:
          message=mqr1.receive()
          print("Received msg:{}".format(message[0]))
          if "DONE" in str(message[0]):
                   break
        try:
              mqr1.remove()
              mqw1.remove()
        except sysv_ipc.ExistentialError:
              print ("Existen.. error")
        #process3.join()
        thread3.join()
        return mpd_file
    else:
        return ("MPD error")
        
get_mpd(" http://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s.mpd")

