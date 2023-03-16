#include <stdio.h>
#include <stdbool>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


//filedirectory
int filedescriptor;
// ints for the lights, either on or off (1 = green, 0 = red)
int LightNorth;
int LightSouth;

// amount of cars in the queues, and on the bridge at the start
int Northqueue, Southqueue; Bridgecount = 0;

// treads
pthread_t pr;
pthread_t ui;
pthread_t sim;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;



int Open_port(){
	int speed = 9600; //serial port speed of 9600bps
	int filedescriptor = open("/dev/ttyS0", O_RDWR);
	if (filedescriptor < 0){
		printf("yikes \n");
	}
	struct termios termios_p; //Get and set terminal attributes
	
	//termios structs contains following:
	//tcflag_t   c_iflag;        /* input modes */
	//tcflag_t   c_oflag;        /* output modes */
	//tcflag_t   c_cflag;        /* control modes */
	//tcflag_t   c_lflag;        /* local modes */
	//cc_t       c_cc[NCCS];     /* control chars */
	
	if (tcgetattr(filedescriptor, &termios_p)){
		printf("Terminos attributes couldn't load")
		return -1;
	}
	
	cfsetospeed(&termios_p, (speed_t)speed);
	
	cfsetispeed(&termios_p, (speed_t)speed);

	tty.c_cflag |= CLOCAL | CREAD;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_lflag &= ~ICANON;
	tty.c_lflag &= ~ISIG;
	tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);
	tty.c_iflag &= ~IGNCR;
	tty.c_iflag &= ~INPCK;
	tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_oflag &= ~OPOST;
	tty.c_cc[VEOL] = 0;
	tty.c_cc[VEOL2] = 0;
	tty.c_cc[VEOF] = 0x04;
	
	if(tcsetattr(filedescriptor, TCSANOW, &termios_p)){
		printf(sterror(errno));
		return -1;
	}
	
	return filedescriptor;
	
}

void *portreader(void *arg){
	uint8_t buf;
	while(true){
		int bytereader = read(filedescriptor, &buf, sizeof(buf));
		if(bytereader != 0){
			LightNorth = (buf & 1) //om grönt ljus om 1, rött om 0 på least signifigant bit
			LightSouth = (c >> 2) & 1 //bitshift med 2, om 3e biten är 1 så grönt, rött om 0 på least signifigant bit
			pthread_create(&ui, NULL, gui, NULL);
		}
	}

}

void *GUI(void *arg){
	pthread_mutex_lock(&m);
	printf("\033c\n"); //clearar screenen i cygwin
	if(LightNorth){
		printf(
			"North: ", "\U0001F7E2", "\n"
		)
	}
	else(LightNorth){
		printf(
			"North: ", "\U0001F534", "\n"
		)
	}
	//northqueue
	if(LightSouth){
		printf(
			"North: ", "\U0001F7E2", "\n"
		)
	}
	else(LightSouth){
		printf(
			"North: ", "\U0001F534", "\n"
		)
	}
	//southqueue
	//bridgecount
	pthread_mutex_unlock(&m);
	pthread_exit(0);

}

Input(void *arg){
	char keycharacter;
	while(true){
		keycharacter = getchar();
		switch (keycharacter)
		{
		case 'n':
			Northqueue = Northqueue + 1;
			//write to port
			break;
		case 's':
			Southqueue = Southqueue + 1;
			// write to port
			break;
		case 'e':
			break;
		default:
			printf("input a valid key you donkey")
		}
	}
}

void portwriter(uint8_t input){
	int writtenbytes = write(filedescriptor, &input, 1);
	pthread_create(&ui, NULL, GUI, NULL);
	if (filedescriptor < 0){
		printf("error writing to port in portwriter \n");
	}
}

int main(void){
	filedescriptor = Open_port();

	if(pthread_create(&pr, NULL, portreader, NULL)){
		printf("Threadfail on: Portreader")
	}
	if(pthread_create(&ui, NULL, GUI, NULL)){
		printf("Threadfail on: GUI / Input")
	}
	

	return Input(NULL);

}
