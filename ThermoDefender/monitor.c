
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
//static const int MAX_CYCLES = 9000; //5 mins
static const int MAX_CYCLES = 1800; //1 mins

static const int CYCLES_TO_RESET_BASE = 240;

int water_tc_differential = -35;
int body_tc_differential = 120;
int fire_tc_differential = 500;
int water_min_detected_pc = 300;
int body_min_detected_pc = 1000;
int fire_min_detected_pc = 100;
bool water_tc_diff_is_negative = false;
static int lepton_reference_array[80][80];
static int current_lepton_array[80][80];

typedef struct {
	bool water_detected;
	bool body_detected;
	bool fire_detected;
} DetectionResults;


static const char *device = "/dev/spidev0.1";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 16000000;
static uint16_t delay;
static uint16_t resetCount = 0;

#define VOSPI_FRAME_SIZE (164)
#define PACKETS_PER_FRAME (60)
uint8_t lepton_frame_packet[VOSPI_FRAME_SIZE];


bool _monitorActive = false;




gboolean set_status_monitor_ended(gpointer data){
	
	update_monitor_status_labels(NULL);
	return FALSE;
}

gboolean set_status_monitor_status(gpointer data){
	
	update_monitor_status_labels((char*)data);
	return FALSE;
}

gboolean set_capture_image(gpointer data){
	
	set_capture_image_from_current_array((GtkImage*)data);
	return FALSE;
}


static void pabort(const char *s)
{
	perror(s);
	
	_active = false;
	_monitorActive = false;
	
	// Call main thread to update the GUI
	g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
	
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
		
			
		
		if(frame_number < PACKETS_PER_FRAME )
		{
			for(i=0;i<80;i++)
			{
				current_lepton_array[frame_number][i] = (lepton_frame_packet[2*i+4] << 8 | lepton_frame_packet[2*i+5]);
			}
			resetCount = 0;
		} else {
			
			printf("Misaligned packet detected. Pausing.\n");
			usleep(200000);
			return -1;			
		
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




DetectionResults read_lepton_array(int currentIteration, int referenceArray[80][80], int compareArray[80][80])
{
	// Initialize the result struct
	DetectionResults dResults;
	dResults.water_detected = false;
	dResults.body_detected = false;
	dResults.fire_detected = false;
	
	// Set base reference array is applicable
	if(currentIteration == 1)
	{
		int i,j;
		
		for(i=0;i<PACKETS_PER_FRAME;i++)
		{
			for(j=0;j<80;j++){
				//lepton_reference_array[i][j] = current_lepton_array[i][j];
				referenceArray[i][j] = compareArray[i][j];
			}
		}	
		
		return dResults;
	}
	
	// Compare current thermal counts against the reference array
	
	int water_detected_pixel_count = 0;
	int body_detected_pixel_count = 0;
	int fire_detected_pixel_count = 0;
	
	int water_detected_total_tc = 0;
	int body_detected_total_tc = 0;
	int fire_detected_total_tc = 0;

	
	int i,j;
	int tc_diff;
	
	
	for(i=0;i<PACKETS_PER_FRAME;i++)
	{
		for(j=0;j<80;j++){
			
			tc_diff = abs(compareArray[i][j] - referenceArray[i][j]);
						
			if(water_tc_diff_is_negative && compareArray[i][j] - referenceArray[i][j] < 0 && tc_diff * -1 <= water_tc_differential) // Water diff is negative
			{
				water_detected_pixel_count++;
				water_detected_total_tc += compareArray[i][j] - referenceArray[i][j];
			}
			else if(!water_tc_diff_is_negative && tc_diff >= water_tc_differential && tc_diff < body_tc_differential)  // Water diff not negative
			{
				water_detected_pixel_count++;
				water_detected_total_tc += tc_diff;
			}
			else if(tc_diff >= body_tc_differential && tc_diff < fire_tc_differential){
				body_detected_pixel_count++;
				body_detected_total_tc += tc_diff;
			}
			else if(tc_diff >= fire_tc_differential){
				fire_detected_pixel_count++;
				fire_detected_total_tc += tc_diff;
			}
			
			
		}
	}
	
	if(water_tc_diff_is_negative)
		dResults.water_detected = water_detected_total_tc < 0 
				&& water_detected_pixel_count >= water_min_detected_pc
				&& (int)(water_detected_total_tc  / water_detected_pixel_count) <= water_tc_differential;
	else
		dResults.water_detected = water_detected_total_tc > 0 
				&& water_detected_pixel_count >= water_min_detected_pc
				&& (int)(water_detected_total_tc  / water_detected_pixel_count) >= water_tc_differential;
	
	dResults.body_detected = body_detected_total_tc > 0 
			&& body_detected_pixel_count >= body_min_detected_pc
			&& (int)(body_detected_total_tc  / body_detected_pixel_count) >= body_tc_differential;
	dResults.fire_detected = fire_detected_total_tc > 0 
			&& fire_detected_pixel_count >= fire_min_detected_pc
			&& (int)(fire_detected_total_tc  / fire_detected_pixel_count) > fire_tc_differential;

	
/*
	//if(dResults.water_detected)
	if(water_detected_total_tc < 0)
		printf("Water pixel count: %d\nWater TC Total: %d\nWater TC Avg: %d\n", water_detected_pixel_count, water_detected_total_tc, (water_detected_total_tc  / water_detected_pixel_count));
	if(dResults.body_detected)
		printf("Body TC Avg: %d\n", (body_detected_total_tc  / body_detected_pixel_count));
	if(dResults.fire_detected)
		printf("Fire TC Avg: %d\n", (fire_detected_total_tc  / fire_detected_pixel_count));
*/
		
	
	//return water_detected_pixel_count >= min_detected_pixel_count;
	return dResults;
}
void* f_monitor(void *arg)
{
	_monitorActive = true;
	water_tc_diff_is_negative = water_tc_differential < 0;
	
	int currentIteration = 0;
	int fd;
	
	while(_active){
	
		currentIteration ++;
		if(currentIteration > MAX_CYCLES){
			_active = !_active;
		} else {
			
			fd = connect_to_lepton();
		
			while(transfer(fd)!=59){}
			
			int status_value = -1;
			status_value = close(fd);
			if(status_value < 0)
				printf("Error - Could not close SPI device");

			//printf("Lepton Polled: %d\n", currentIteration);
			
		
			DetectionResults results = read_lepton_array(currentIteration, lepton_reference_array, current_lepton_array);
			
			if(results.water_detected){
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
				pabort("Water Detected");
				//char *s = "Water Detected!";
				//g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
			}
			else if(results.body_detected){
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
				pabort("Body heat Detected");
				//char *s = "Body heat Detected!";
				//g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
			}
			else if(results.fire_detected){
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
				pabort("Fire Detected");
				//char *s = "Fire Detected!";
				//g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
			}
			
			// Update the image if first iteration
			if(currentIteration == 1)
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
			
			usleep(24000); // 30 FPS
			//usleep(50000); // 20 FPS
			//usleep(62500); // 16 FPS   ##### Many more misaligned packets
			
			
			if(currentIteration % 30 == 0)
				printf("Second %d\n", currentIteration / 30);
			
		}
	}
		
	
	_monitorActive = false;
	
	// Call main thread to update the GUI
	g_main_context_invoke(mainc, set_status_monitor_ended, NULL);
	
	// Exit the thread
	pthread_exit(NULL);		
}




int get_tc_diff_from_array(int min_diff, int min_pixel_count, int referenceArray[80][80], int compareArray[80][80])
{

	int detected_pixel_count = 0;
	int total_tc_above_min = 0;
	
	// Compare array values
	int i,j;
			
	for(i=0;i<PACKETS_PER_FRAME;i++)
	{
		for(j=0;j<80;j++){
			if(abs(compareArray[i][j] - referenceArray[i][j]) > min_diff)
			{
				detected_pixel_count++;
				total_tc_above_min += compareArray[i][j] - referenceArray[i][j];
				//printf("Diff: %d\n", compareArray[i][j] - referenceArray[i][j]);
			}
		}
	}	

	//printf("Pixel Count: %d\n", detected_pixel_count);
	
	if(detected_pixel_count >= min_pixel_count)
		return (int) total_tc_above_min / detected_pixel_count;
	
	return 0;
}


bool set_reference_frame()
{
	int fd;
	fd = connect_to_lepton();
	while(transfer(fd)!=59){}
	int status_value = -1;
	status_value = close(fd);

	// read array into the reference array
	read_lepton_array(1,lepton_reference_array, current_lepton_array);
	
	return status_value > -1;
}

int get_tc_difference(int min_diff, int min_pixel_count)
{
	int fd;
	fd = connect_to_lepton();
	while(transfer(fd)!=59){}
	int status_value = -1;
	status_value = close(fd);
	
	return get_tc_diff_from_array(min_diff, min_pixel_count, lepton_reference_array, current_lepton_array);
}

