import hyper
import urllib
import sys

DOWNLOAD_CHUNK = 15000

def download_vid():
    ssl_context = hyper.tls.init_context()
    ssl_context.load_cert_chain(certfile='/dev/SQUAD/cert.crt', keyfile='/dev/SQUAD/cert.key')
    ssl_context.load_verify_locations(cafile='/dev/SQUAD/cert.pem')
    parse_url = urllib.parse.urlparse("https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets")
    connection = hyper.HTTP11Connection(parse_url.netloc, ssl_context=ssl_context, secure=True, port
=9000)
    connection.network_buffer_size= DOWNLOAD_CHUNK
    
    for i in range(1,150):
        segment_url="https://10.10.3.2/www-itec.uni-klu.ac.at/ftp/datasets/DASHDataset2014/BigBuckBunny/2sec/bunny_4219897bps/BigBuckBunny_2s{}.m4s".format(i)
        print("Downloading: {}\n".format(segment_url))
        try:
            seg_resp_conn = connection.request('GET',segment_url)
            seg_conn = connection.get_response()
            with open("/users/rdeshm0/hyper_test/vid.m4s",'wb') as segment_file_handle:
                for segment_data in seg_conn.read_chunked_give_size(DOWNLOAD_CHUNK):
                    if segment_data is -1:
                       break
                    segment_file_handle.write(segment_data)
                    segment_data = seg_conn.read(DOWNLOAD_CHUNK)
            seg_conn.close()
        except:
            print("Unable to download DASH Segment {} HTTP Error:{} ".format(segment_url, sys.exc_info()))
            return None
    
download_vid()
