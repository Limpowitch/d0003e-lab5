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
int LightNorth = 0; //ta bort = 0 senare
int LightSouth = 0; //ta bort = 0 senare

// amount of cars in the queues, and on the bridge at the start
int Northqueue, Southqueue, Bridgecount = 0;

// treads
pthread_t pr;
pthread_t ui;
pthread_t sim;
pthread_t drive;
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
	printf("\033c\n"); //clearar screenen i cygwin
	if(LightNorth == 1){
		printf("               North: Green            ");
	}
	
	if(LightNorth == 0){
		printf("               North: Red            ");
	}
	
	if(LightSouth == 1){
		printf("          South: Green          \n");
	}
	
	if(LightSouth == 0){
		printf("         South: Red          \n");
	}
	
	printf("Northqueue: %d", Northqueue, "       ");
	printf("               Bridgequeue: %d", Bridgecount, "            ");
	printf("                 Southqueue: %d", Southqueue);
	printf("\n");
	pthread_mutex_unlock(&m);
	pthread_exit(0);

}


void *portreader(void *arg){
	uint8_t buf;
	
	while(1){
		int bytereader = read(filedirector, &buf, sizeof(buf));
		
		if(bytereader != 0){
			LightNorth = (buf & 1); //om grönt ljus om 1, rött om 0 på least signifigant bit
			LightSouth = (buf >> 2) & 1; //bitshift med 2, om 3e biten är 1 så grönt, rött om 0 på least signifigant bit
			pthread_create(&ui, NULL, GUI, NULL);
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

int Input(void *arg){
	char keycharacter;
	while(1){
		struct termios old, new;
		char c;
		
		tcgetattr(STDIN_FILENO, &old);
		
		new = old;
		
		new.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		
		c = getchar();
		
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		
		
		switch (c)
		{
		case 'n':
			Northqueue = Northqueue + 1;
			portwriter(1);
			break;
		case 's':
			Southqueue = Southqueue + 1;
			portwriter(4);
			break;
		case 'e':
			break;
		default:
			printf("input a valid key you donkey");
		}
	}
}



void *bridge(void *arg){
	Bridgecount = Bridgecount + 1;
	pthread_create(&ui, NULL, GUI, NULL);
	sleep(5);
	Bridgecount = Bridgecount - 1;
	pthread_create(&ui, NULL, GUI, NULL);
	pthread_exit(0);
}

void *simulator(void *arg){
	int oknorth = 1;
	int oksouth = 1;
	
	while(1){
		if( (Southqueue > 0) && LightSouth && oksouth){
			Southqueue = Southqueue - 1;
			pthread_create(&drive, NULL, bridge, NULL);
			portwriter(8);
			oksouth = 0;
		}
		if( (Northqueue > 0) && LightNorth && oknorth){
			Northqueue = Northqueue - 1;
			pthread_create(&drive, NULL, bridge, NULL);
			portwriter(2);
			oksouth = 0;
		}
		if(!LightNorth && !oknorth){
			oknorth = 1;
		}
		if (!LightSouth && !oksouth){
			oksouth = 1;
		}
	}
}

int main(void){
	filedirector = Open_port();

	if(pthread_create(&pr, NULL, portreader, NULL)){
		printf("threadfail on: portreader");
	}
	if(pthread_create(&ui, NULL, GUI, NULL)){
		printf("Threadfail on: GUI / Input");
	}
	if(pthread_create(&sim, NULL, simulator, NULL)){
		printf("Threadfail on: Simulator");
	}
	
	
	
	

	return Input(NULL);

	//while(1){
		//
	//}
}
