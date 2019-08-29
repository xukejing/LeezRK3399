 opencvudp.cpp在x86 windows10上用visualstudio2015编译，需要opencv4.1.0
 
 
 clouddetec.cpp在嵌入式设备上跑，用apt安装opencv就可以
 
  mjpg_streamer -o "output_http.so -w ./www" -i "input_uvc.so -d /dev/video8 -r 640x480 -f 30"

 
 g++ -ggdb clouddetec.cpp -o aaaaa `pkg-config --cflags --libs opencv` g++ -ggdb clouddetec.cpp -o aaaaa `pkg-config --cflags --libs opencv`
 ./aaaaa
