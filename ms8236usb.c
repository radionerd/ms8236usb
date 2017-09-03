#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>


// This code was pasted together from the following sources
// Time Example: https://en.wikipedia.org/wiki/C_date_and_time_functions
// Serial Port Example: 
//    http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
// Protocol reference: https://sigrok.org/wiki/HYELEC_MS8236
// Additions by Richard Jones


int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}

void prtime(void)
{
    time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        exit(EXIT_FAILURE);
    }

    /* Print to stdout. ctime() has already added a terminating newline character. */
    (void) printf("%s", c_time_string);
    fflush(stdout);
}

void 	decode_digit(unsigned char raw_digit )
{
	unsigned char digit_pattern[] = { 0x00,0x5f,0x06,0x6b,0x2f,0x36,0x3d,0x7d,0x07,0x7f,0x3f,0x58 };
	char *digit_string[]          = {  "" , "0", "1", "2", "3", "4", "5", "6", "7", "8", "9","L" };
	if (raw_digit & 0x80 )
		printf(".");
	for (int i = 0 ; i <= 11; i++ )
	  if ( digit_pattern[i] == ( raw_digit & 0x7f ) )
		printf("%s",digit_string[i]);  
}

void decode_bits(unsigned char bits,char*icon[])
{
   for ( int i = 0 ; i < 8 ; i++)
   {
      if ( bits & 1 )
      {
          printf("%s",icon[i]);
      }
      bits = bits >> 1;
   }
}

void decode_msg(unsigned char *raw_msg)
{
        if(raw_msg[10]&0x18) printf("-");
	decode_digit(raw_msg[9]);
	decode_digit(raw_msg[8]);
	decode_digit(raw_msg[7]);
	decode_digit(raw_msg[6]);
        printf(" ");
	char * icons20[8] = {"DegC ","DegF ","?","?","m","u","n","F "};
        decode_bits(raw_msg[20],icons20);
	char * icons21[8] = {"u","m","A ","V ","M","k","Ohms ","Hz "};
        decode_bits(raw_msg[21],icons21);
	char * icons10[8] = {"Diode ","AC ","DC ","-","-","","Continuity ","LowBattery "};
        decode_bits(raw_msg[10]&0xE7,icons10);
	char * icons18[8] = {"","","","","Wait ","Auto ","Hold ","REL "};
        decode_bits(raw_msg[18],icons18);
	char * icons19[8] = {"","MAX","-","MIN","N/A","%","hFE","N/A"};
        decode_bits(raw_msg[19],icons19);
	prtime();
}


int main()
{
    char *portname = "/dev/ttyUSB0";
    int fd;
    // int wlen;

    printf("Data Logging interface for PeakMeter MS8236 USB Multimeter.\n");
    printf("If logging does not start make sure USB lead is connected,\n");
    printf("then press and hold USB button on meter for two seconds.\n");
    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(fd, B2400);
    //set_mincount(fd, 0);                /* set to pure timed read */

    /* simple output */
    //wlen = write(fd, "Hello!\n", 7);
    //if (wlen != 7) {
    //    printf("Error from write: %d, %d\n", wlen, errno);
    //}
    //tcdrain(fd);    /* delay for output */


    /* simple noncanonical input */
    int msg_index=0;
    unsigned char raw_msg[80];
    do {
        unsigned char buf[80];
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);
        if (rdlen > 0) {
#ifdef DISPLAY_STRING
            buf[rdlen] = 0;
            printf("Read %d: \"%s\"\n", rdlen, buf);
#else /* display hex */
            unsigned char   *p;
            //printf("Read %d:", rdlen);
            for (p = buf; rdlen-- > 0; p++)
            {
                if (*p == 0xAA )
                {
                    //printf("\n");
                    msg_index = 0;
                }
                //printf(" 0x%02x", *p);
                raw_msg[msg_index++] = *p ;
                if (msg_index >= 80 ) msg_index--;
                if (msg_index == 22 ) 
                {
                    decode_msg(raw_msg);
                    msg_index = 0;
                }
                //printf("msg_index=%d\n",msg_index);
            }
#endif
        } else if (rdlen < 0) {
            printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        }
        /* repeat read to get full message */
    } while (1);
}
