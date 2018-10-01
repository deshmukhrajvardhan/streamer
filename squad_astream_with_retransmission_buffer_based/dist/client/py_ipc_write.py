import sysv_ipc
import threading
import sys
import errno
import timeit
from string import ascii_letters, digits
import read_mpd
import urllib
import random
import os
import time


def write_msg(list_cmd, mq):
    for i in list_cmd:
        print(i)
        mq.send(i, True)
    return


def get_mpd(url1,url2):
    """ Module to download the MPD from the URL and save it to file"""
    key_c_orig_r = 262144
    key_c_orig_w = 262145
    key_c_retx_r = 362146
    key_c_retx_w = 462146
    keyr1 = 562146
    keyr2 = 662146

    ############################################## send url ######################################

#    mpd_file = url.split('/')[-1]
#    mpd_file = os.path.abspath(mpd_file)
    try:
        mq_orig_w = sysv_ipc.MessageQueue(key_c_orig_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")
    try:
        mq_retx_w = sysv_ipc.MessageQueue(key_c_retx_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")

    # parse_url = urllib.parse.urlparse(url)
    # cmd1=["CREATE_STREAM",parse_url.path,mpd_file]
    cmd1 = [url1]
    cmd2 = [url2]
    print(url1,url2)
    # print (mpd_file)
    # if "CONN_CREATED" in (str(message[0],'utf-8', errors='ignore').split('\x00')[0]):
    if True:
        print("CONNECTION ESTABLISHED")
        thread_orig = threading.Thread(target=write_msg, args=(cmd1, mq_orig_w))
        thread_retx = threading.Thread(target=write_msg, args=(cmd2, mq_retx_w))

        thread_orig.start()
#        time.sleep(1)
        thread_retx.start()
        thread_retx.join()
    #        return mpd_file
    else:
        return ("MPD error")

get_mpd("https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s10.m4s","https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s11.m4s")
