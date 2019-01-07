import sysv_ipc
import time
import threading
from multiprocessing import Process, Queue
import sys
import errno
import timeit
from string import ascii_letters, digits
import urllib
import random
import os


def write_msg(list_cmd, mq):
    for i in list_cmd:
        print(i)
        mq.send(i, True)
    return

def read_ipc(segment_url):

    key_c_orig_w = 262145

    try:
        mq_orig_r = sysv_ipc.MessageQueue(key_c_orig_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    chunk_number = 0
    total_data_dl_time = 0
    segment_chunk_rates=[]
    segment_size=0
    chunk_start_time = timeit.default_timer()


    while True:
        (message, prior) = mq_orig_r.receive()
        m = message.split(b':')
        # print("Reply:{}, content-length:{}\n".format(message,m[1]))
        # chunk_sizes.append(float(m[1]))
        if m[0] != b'end':
            segment_size += float(m[1])
            timenow = timeit.default_timer()
            chunk_dl_time = timenow - chunk_start_time
            chunk_start_time = timenow
            chunk_number += 1
            total_data_dl_time += chunk_dl_time
            current_chunk_dl_rate = segment_size * 8 / total_data_dl_time
            segment_chunk_rates.append(current_chunk_dl_rate)
        else:
            break

    # content_length = chunk_sizes.pop()
    # DONE part
    with open('/dev/SQUAD/3_segs_chunk_rate_squad_libcurl_HTTP2.txt', 'a') as chk:
        chk.write("{}".format(segment_url))
        for item in segment_chunk_rates:
            chk.write(",{}".format(item))
        chk.write("\n")
    # print("{} sum:{}, content_length:{}".format(p_no,sum(chunk_sizes), content_length))

def write_ipc(segment_url):
    key_c_orig_r = 262144
    cmd1 = [segment_url]

    try:
        mq_orig_w = sysv_ipc.MessageQueue(key_c_orig_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")
    
    process3 = Process(target=write_msg, args=(cmd1, mq_orig_w))
    process3.start()
    process3.join()
    #time.sleep(0.25)



def get_segment(segment_url):
    """ Module to download the MPD from the URL and save it to file"""
    key_c_orig_r = 262144
    key_c_orig_w = 262145
#    key_c_retx_r = 362146
#    key_c_retx_w = 462146
    #print(segment_url)
    cmd1 = [segment_url]

    try:
        mq_orig_w = sysv_ipc.MessageQueue(key_c_orig_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")
    try:
        mq_orig_r = sysv_ipc.MessageQueue(key_c_orig_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    if segment_url =="-2":
        process3 = Process(target=write_msg, args=(cmd1, mq_orig_w))
        process3.start()
        process3.join()
        exit()

    chunk_number = 0
    total_data_dl_time = 0
    segment_chunk_rates=[]
    segment_size=0
    process3 = Process(target=write_msg, args=(cmd1, mq_orig_w))
    # thread3=threading.Thread(target=write_msg,args=(cmd1,mqw1))
    # thread3.start()
    process3.start()
    chunk_start_time = timeit.default_timer()
    # ipc read

    # print("Py master reading chunks")
    while True:
        (message, prior) = mq_orig_r.receive()
        m = message.split(b':')
        # print("Reply:{}, content-length:{}\n".format(message,m[1]))
        # chunk_sizes.append(float(m[1]))
        if m[0] != b'end':
            segment_size += float(m[1])
            timenow = timeit.default_timer()
            chunk_dl_time = timenow - chunk_start_time
            chunk_start_time = timenow
            chunk_number += 1
            total_data_dl_time += chunk_dl_time
            current_chunk_dl_rate = segment_size * 8 / total_data_dl_time
            segment_chunk_rates.append(current_chunk_dl_rate)
        else:
            break

    # content_length = chunk_sizes.pop()
    # DONE part
    with open('/dev/SQUAD/6_segs_chunk_rate_squad_libcurl_full_HTTP2.txt', 'a') as chk:
        chk.write("{}".format(segment_url))
        for item in segment_chunk_rates:
            chk.write(",{}".format(item))
        chk.write("\n")
    # print("{} sum:{}, content_length:{}".format(p_no,sum(chunk_sizes), content_length))

    process3.join()


def get_retx_segment(segment_url):
    """ Module to download the MPD from the URL and save it to file"""
#    key_c_orig_r = 262144
#    key_c_orig_w = 262145
    key_c_retx_r = 362146
    key_c_retx_w = 462146
    #print(segment_url)
    cmd1 = [segment_url]

    try:
        mq_retx_w = sysv_ipc.MessageQueue(key_c_retx_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")
    try:
        mq_retx_r = sysv_ipc.MessageQueue(key_c_retx_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    if segment_url =="-2":
        process3 = Process(target=write_msg, args=(cmd1, mq_retx_w))
        process3.start()
        process3.join()
        exit()

    chunk_number = 0
    total_data_dl_time = 0
    segment_chunk_rates=[]
    segment_size=0
    process3 = Process(target=write_msg, args=(cmd1, mq_retx_w))
    # thread3=threading.Thread(target=write_msg,args=(cmd1,mqw1))
    # thread3.start()
    process3.start()
    chunk_start_time = timeit.default_timer()
    # ipc read

    # print("Py master reading chunks")
    while True:
        (message, prior) = mq_retx_r.receive()
        m = message.split(b':')
        # print("Reply:{}, content-length:{}\n".format(message,m[1]))
        # chunk_sizes.append(float(m[1]))
        if m[0] != b'end':
            segment_size += float(m[1])
            timenow = timeit.default_timer()
            chunk_dl_time = timenow - chunk_start_time
            chunk_start_time = timenow
            chunk_number += 1
            total_data_dl_time += chunk_dl_time
            current_chunk_dl_rate = segment_size * 8 / total_data_dl_time
            segment_chunk_rates.append(current_chunk_dl_rate)
        else:
            break

    # content_length = chunk_sizes.pop()
    # DONE part
    with open('/dev/SQUAD/6_retx_segs_chunk_rate_squad_libcurl_full_HTTP2.txt', 'a') as chk:
        chk.write("{}".format(segment_url))
        for item in segment_chunk_rates:
            chk.write(",{}".format(item))
        chk.write("\n")
    # print("{} sum:{}, content_length:{}".format(p_no,sum(chunk_sizes), content_length))

    process3.join()

def get_http1_segment(segment_url):
    """ Module to download the MPD from the URL and save it to file"""
    key_c_orig_r = 662144
    key_c_orig_w = 662145
#    key_c_retx_r = 362146
#    key_c_retx_w = 462146
    #print(segment_url)
    cmd1 = [segment_url]

    try:
        mq_orig_w = sysv_ipc.MessageQueue(key_c_orig_r, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: Queue not created")
    try:
        mq_orig_r = sysv_ipc.MessageQueue(key_c_orig_w, sysv_ipc.IPC_CREAT, max_message_size=15000)
    except:
        print("ERROR: mq_orig_r Queue not created")

    if segment_url =="-2":
        process3 = Process(target=write_msg, args=(cmd1, mq_orig_w))
        process3.start()
        process3.join()
        exit()

    chunk_number = 0
    total_data_dl_time = 0
    segment_chunk_rates=[]
    segment_size=0
    process3 = Process(target=write_msg, args=(cmd1, mq_orig_w))
    # thread3=threading.Thread(target=write_msg,args=(cmd1,mqw1))
    # thread3.start()
    process3.start()
    chunk_start_time = timeit.default_timer()
    # ipc read

    # print("Py master reading chunks")
    while True:
        (message, prior) = mq_orig_r.receive()
        m = message.split(b':')
        # print("Reply:{}, content-length:{}\n".format(message,m[1]))
        # chunk_sizes.append(float(m[1]))
        if m[0] != b'end':
            segment_size += float(m[1])
            timenow = timeit.default_timer()
            chunk_dl_time = timenow - chunk_start_time
            chunk_start_time = timenow
            chunk_number += 1
            total_data_dl_time += chunk_dl_time
            current_chunk_dl_rate = segment_size * 8 / total_data_dl_time
            segment_chunk_rates.append(current_chunk_dl_rate)
        else:
            break

    # content_length = chunk_sizes.pop()
    # DONE part
    with open('/dev/SQUAD/6_segs_chunk_rate_squad_libcurl_HTTP1_1.txt', 'a') as chk:
        chk.write("{}".format(segment_url))
        for item in segment_chunk_rates:
            chk.write(",{}".format(item))
        chk.write("\n")
    # print("{} sum:{}, content_length:{}".format(p_no,sum(chunk_sizes), content_length))

    process3.join()


def main(argv):
    urls = ["https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s13.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s14.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s15.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s16.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s17.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s18.m4s"]
    low_urls = ["https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s13.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s14.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s15.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s16.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s17.m4s",
            "https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_3526922bps/BigBuckBunny_2s18.m4s"]

    if argv[1] == 'r':
        for i in urls:
            read_ipc(i)
    elif argv[1] == 'w':
        for i in urls:
            write_ipc(i)
    elif argv[1] == 'retx':
        for j in range(2):
        #if True:                                                                                                                                                   
            for i,j in zip(urls,low_urls):
            #if True:                                                                                                                                               
#                print("I:{},type:{}".format(i,type(i)))                                                                                                            
                process1 = Process(target=get_segment, args=(i,))
                process2 = Process(target=get_retx_segment, args=(j,))
                process1.start()
                process2.start()        
                process1.join()
    
    elif argv[1]=='http2':
        for i in urls:
            get_segment(i)
    elif argv[1]=='http1':
        for i in urls:
            get_http1_segment(i)
    else:
        print("Testing HTTP/1.1 by default!")
        for i in urls:
            get_http1_segment(i)

if __name__ == '__main__':
#get_mpd("https:
    main(sys.argv)
