#define XMODEM_DEBUG
#define XMODEM_ALLOW_NONSEQUENTIAL
#include "xmodem.c"
#include <string.h>
#include <stdio.h>

//TODO: Maybe revisit this and create a seperate clone of the arduino Serial stream?
bool setup_serial(int fd, int baudrate, int flags);
int open_serial(char* device_path, int baudrate);

int main(int argc, char** argv) {
  printf("opening device\n");
  //int fd = open_serial("/dev/ttyUSB0", B9600);
  int fd = open_serial("/dev/ttyUSB0", B4800);
  if(fd < 0) {
    printf("Error %i from open_serial: %s\n", errno, strerror(errno));
    exit(1);
  }
  struct xmodem_config config = {};
  xmodem_init_config(&config);

  //you can modify the library configuration here
  /*
  config.id_bytes = 2;
  config.data_bytes = 64;
  config.chksm_bytes = 1;
  */

  unsigned char *data = "This_is_a_64_byte_xmodem_packet.Normally_they_are_128_bytes_tho.tHIS_IS_A_64_BYTE_XMODEM_PACKET.nORMALLY_THEY_ARE_128_BYTES_THO.This_is_a_64_byte_xmodem_packet.Normally_they_are_128_bytes_tho.";
  while(!xmodem_send(fd, &config, data, strlen(data))) {}
  printf("\nDone");

  printf("\nclosing device\n");
  close(fd);

  return 0;
}

bool setup_serial(int fd, int baudrate, int flags) {
  if(flags == 0) {}//TODO: Set default flags?

  struct termios tty;
  memset((char *)&tty, 0, sizeof(struct termios));

  if(tcgetattr(fd, &tty) != 0) return false;

  //PARENB seems to not be getting set properly.... TODO: Is this valid?
  tty.c_cflag |= ( 0
      | PARENB    //Enable Parity
      | CS8       //8 bits per byte
      | CREAD     //Enable receiver
      | CLOCAL);  //Ignore model control lines

  tty.c_cflag &= ~( 0
      | CSTOPB    //single stop bit
      | CRTSCTS); //disable hardware flow control

  tty.c_lflag &= ~( 0
      | ICANON   //non-canonical input mode
      | ECHO     //disable echo
      | ECHOE    //disable erasure
      | ECHONL   //disable new-line echo
      | ISIG);   //disable INTR, QUIT, SUSP control characters

  tty.c_iflag &= ~( 0
      | IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF);  //disable special byte sequences

  tty.c_oflag &= ~( 0
      | OPOST    //disable special interpretations of output eg newlines
      | ONLCR);  //disable conversion of newline to CRLF

  tty.c_cc[VTIME] = 30; //reads wait for up to 3s
  tty.c_cc[VMIN] = 0; //reads return as soon as data is available
  //set baud rate
  cfsetispeed(&tty, baudrate);
  cfsetospeed(&tty, baudrate);

  //commit tty settings
  if(tcsetattr(fd, TCSANOW, &tty) != 0) return false;
  return true;
}

int open_serial(char* device_path, int baudrate) {
  int fd = open(device_path, O_RDWR);
  if(fd < 0) {
    return fd;
  }
  if(!setup_serial(fd, baudrate, 0)) {
    //error close device
    close(fd);
    return -1;
  }
  return fd;
}
