#include <stdio.h>
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
int filedirector;
// ints for the lights, either on or off (1 = green, 0 = red)
int LightNorth;
int LightSouth;

// amount of cars in the queues, and on the bridge at the start
int Northqueue, Southqueue, Bridgecount = 0;

// treads
pthread_t pr;
pthread_t ui;
pthread_t sim;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;



int Open_port(){
	int speed = 9600; //serial port speed of 9600bps
	int filedirector = open("/dev/ttyS0", O_RDWR);
	if (filedirector < 0){
		printf("yikes \n");
	}
	struct termios termios_p; //Get and set terminal attributes
	
	//termios structs contains following:
	//tcflag_t   c_iflag;        /* input modes */
	//tcflag_t   c_oflag;        /* output modes */
	//tcflag_t   c_cflag;        /* control modes */
	//tcflag_t   c_lflag;        /* local modes */
	//cc_t       c_cc[NCCS];     /* control chars */
	
	if (tcgetattr(filedirector, &termios_p)){
		printf("Terminos attributes couldn't load");
		return -1;
	}
	
	cfsetospeed(&termios_p, (speed_t)speed);
	
	cfsetispeed(&termios_p, (speed_t)speed);

	termios_p.c_cflag |= CLOCAL | CREAD;
	termios_p.c_cflag &= ~CSIZE;
	termios_p.c_cflag |= CS8;
	termios_p.c_cflag &= ~PARENB;
	termios_p.c_cflag &= ~CSTOPB;
	termios_p.c_cflag &= ~CRTSCTS;
	termios_p.c_lflag &= ~ICANON;
	termios_p.c_lflag &= ~ISIG;
	termios_p.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);
	termios_p.c_iflag &= ~IGNCR;
	termios_p.c_iflag &= ~INPCK;
	termios_p.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
	termios_p.c_iflag &= ~(IXON | IXOFF | IXANY);
	termios_p.c_oflag &= ~OPOST;
	termios_p.c_cc[VEOL] = 0;
	termios_p.c_cc[VEOL2] = 0;
	termios_p.c_cc[VEOF] = 0x04;
	
	if(tcsetattr(filedirector, TCSANOW, &termios_p)){
		printf(strerror(errno));
		return -1;
	}
	
	return filedirector;
	
}



void *GUI(void *arg){
	pthread_mutex_lock(&m);
	//printf("\033c\n"); //clearar screenen i cygwin
	if(LightNorth){
		printf(
			"North: 1 \n", "\U0001F7E2", "\n"
		);
	}
	else;{
		printf(
			"North: 2 \n", "\U0001F534", "\n"
		);
	};
	//northqueue
	if(LightSouth){
		printf(
			"South: 1 \n", "\U0001F7E2", "\n"
		);
	}
	else;{
		printf(
			"South: 2 \n", "\U0001F534", "\n"
		);
	};
	//southqueue
	//bridgecount
	pthread_mutex_unlock(&m);
	pthread_exit(0);

}

void *portreader(void *arg){
	uint8_t buf;
	
	while(1){
		uint8_t inverted_buf = ~buf;
		int bytereader = read(filedirector, &inverted_buf, sizeof(inverted_buf));
		
		if(bytereader != 0){
			LightNorth = (inverted_buf & 1); //om grönt ljus om 1, rött om 0 på least signifigant bit
			LightSouth = (inverted_buf >> 2) & 1; //bitshift med 2, om 3e biten är 1 så grönt, rött om 0 på least signifigant bit
			pthread_create(&ui, NULL, GUI, NULL);
		}
	}

}

int Input(void *arg){
	char keycharacter;
	while(1){
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
			printf("input a valid key you donkey");
		}
	}
}

void portwriter(uint8_t input){
	int writtenbytes = write(filedirector, &input, 1);
	pthread_create(&ui, NULL, GUI, NULL);
	if (filedirector < 0){
		printf("error writing to port in portwriter \n");
	}
}

int main(void){
 	printf("Välkommen till mitt ascoola program \n");
	filedirector = Open_port();

	if(pthread_create(&pr, NULL, portreader, NULL)){
		printf("Threadfail on: Portreader");
	}
	if(pthread_create(&ui, NULL, GUI, NULL)){
		printf("Threadfail on: GUI / Input");
	}
	

	return Input(NULL);

}
