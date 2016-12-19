/*

#include <stdio.h>
#include <stdbool.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include <stdint.h>
#include<unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <limits.h>
#include "main.h"


// --- Constants
static const int MAX_CYCLES = 240;
static const int CYCLES_TO_RESET_BASE = 240;
static const int max_tc_differential = 50;
static const int min_detected_pixel_count = 300;
static unsigned int lepton_reference_array[80][80];


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev0.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 16000000;
static uint16_t delay;

#define VOSPI_FRAME_SIZE (164)
#define PACKETS_PER_FRAME (60)
uint8_t lepton_frame_packet[VOSPI_FRAME_SIZE];
static unsigned int current_lepton_array[80][80];

bool _monitorActive = false;




gboolean set_status_monitor_ended(gpointer data){
	
	update_monitor_status_labels(NULL);
	return FALSE;
}

gboolean set_status_monitor_error(gpointer data){
	
	update_monitor_status_labels((char*)data);
	return FALSE;
}

static void pabort(const char *s)
{
	perror(s);
	
	_active = false;
	_monitorActive = false;
	
	// Call main thread to update the GUI
	g_main_context_invoke(mainc, set_status_monitor_error, (gpointer)s);
	
	// Exit the thread
	pthread_exit(NULL);
}

char* save_pgm_file(void)
{
	int i;
	int j;
	unsigned int maxval = 0;
	unsigned int minval = UINT_MAX;
	static char image_name[32];
	int image_index = 0;

	do {
		sprintf(image_name, "IMG_%.4d.pgm", image_index);
		image_index += 1;
		if (image_index > 9999) 
		{
			image_index = 0;
			break;
		}

	} while (access(image_name, F_OK) == 0);

	FILE *f = fopen(image_name, "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	printf("Calculating min/max values for proper scaling...\n");
	for(i=0;i<60;i++)
	{
		for(j=0;j<80;j++)
		{
			if (current_lepton_array[i][j] > maxval) {
				maxval = current_lepton_array[i][j];
			}
			if (current_lepton_array[i][j] < minval) {
				minval = current_lepton_array[i][j];
			}
		}
	}
	printf("maxval = %u\n",maxval);
	printf("minval = %u\n",minval);
	
	fprintf(f,"P2\n80 60\n%u\n",maxval-minval);
	for(i=0;i<60;i++)
	{
		for(j=0;j<80;j++)
		{
			fprintf(f,"%d ", current_lepton_array[i][j] - minval);
			//fprintf(f,"%d ", current_lepton_array[i][j]);
		}
		fprintf(f,"\n");
	}
	fprintf(f,"\n\n");

	fclose(f);
	
	return image_name;
}


int transfer(int fd)
{
	int ret;
	int i;
	int frame_number;
	uint8_t tx[VOSPI_FRAME_SIZE] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)lepton_frame_packet,
		.len = VOSPI_FRAME_SIZE,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");

	if(((lepton_frame_packet[0]&0xf) != 0x0f))
	{
		frame_number = lepton_frame_packet[1];
		
		//printf("frame=%d\n", frame_number);

		if(frame_number < PACKETS_PER_FRAME )
		{
			for(i=0;i<80;i++)
			{
				current_lepton_array[frame_number][i] = (lepton_frame_packet[2*i+4] << 8 | lepton_frame_packet[2*i+5]);
			}
		} else {
			
			int j;
			for(j=0;j<80;j++)
			{
				printf("Index:%d, value:%d\n", j, lepton_frame_packet[j]);
			}
			
			pabort("Frame greater than 59");
			
		}
	}
	
	return frame_number;
}


int connect_to_lepton()
{

	int ret = 0;
	int fd;
	
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		pabort("Can't open device");
	}

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		pabort("Can't set spi mode");
	}

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	{
		pabort("Can't get spi mode");
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		pabort("Can't set bits per word");
	}

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		pabort("Can't get bits per word");
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		pabort("Can't set max speed hz");
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		pabort("Can't get max speed hz");
	}

	return fd;
}




bool read_lepton_array(int currentIteration)
{

	// Set base reference array is applicable
	if(currentIteration == 1)
	{
		int i,j;
		
		for(i=0;i<PACKETS_PER_FRAME;i++)
		{
			for(j=0;j<80;j++){
				lepton_reference_array[i][j] = current_lepton_array[i][j];
			}
		}	
		
		return FALSE;
	}
	
	int detected_pixel_count = 0;
	// Compare current thermal counts against the reference array
	int i,j;
		
	for(i=0;i<PACKETS_PER_FRAME;i++)
	{
		for(j=0;j<80;j++){
			if(abs(current_lepton_array[i][j] - lepton_reference_array[i][j]) > max_tc_differential)
			{
				//printf("C_TC:%d, R_TC:%d, Diff:%d\n", current_lepton_array[i][j], lepton_reference_array[i][j], current_lepton_array[i][j] - lepton_reference_array[i][j]);
				detected_pixel_count++;
			}
				
		}
	}	
	
	printf("Pixel Differential: %d\n", detected_pixel_count);
	
	return detected_pixel_count >= min_detected_pixel_count;
}



void* f_monitor(void *arg)
{
	_monitorActive = true;
	
	int currentIteration = 0;
	int fd;
	
	while(_active){
	
		currentIteration ++;
		if(currentIteration > MAX_CYCLES){
			_active = !_active;
		} else {
			
			fd = connect_to_lepton();
			
			printf("spi mode: %d\n", mode);
			printf("bits per word: %d\n", bits);
			printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
			
			while(transfer(fd)!=59){}
			
			int status_value = -1;
			status_value = close(fd);
			if(status_value < 0)
				printf("Error - Could not close SPI device");

			printf("Lepton Polled: %d\n", currentIteration);
			
			if(read_lepton_array(currentIteration))
				pabort("Change Detected");
			
		
			usleep(50000); // 20 FPS
			
		}
	}
		
	
	_monitorActive = false;
	
	// Call main thread to update the GUI
	g_main_context_invoke(mainc, set_status_monitor_ended, NULL);
	
	// Exit the thread
	pthread_exit(NULL);		
}

*/
