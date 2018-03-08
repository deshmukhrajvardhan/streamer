# streamer
Intended to be used with modified (http1) version of [hyper](https://github.com/deshmukhrajvardhan/hyper.git)

client files used:
1. Requests library [http1](../master/astream_dash/dist/client/dash_client.py)
2. Hyper library https [http1](../master/squad_astream_with_retransmission_buffer_based/dist/client/http1_dash_client_read_chunked.py)
3. Hyper library https [http2](../master/squad_astream_with_retransmission_buffer_based/dist/client/dash_client_http2_nw_buff.py)

## Command to run Astreamer with http2: (Note: check your directory path and ip)
```
cd /mnt/QUIClientServer0/src/; time sudo python3 out/Debug/squad_astream_with_retransmission_buffer\
_based/dist/client/dash_client_http2_nw_buff.py -m https://10.10.4.2/www-itec.uni-klu.ac.at/ftp/dat\
asets/DASHDataset2014/BigBuckBunny/2sec/BigBuckBunny_2s_300s.mpd -p empirical -r;
```
