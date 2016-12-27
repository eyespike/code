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
#include "wand/magick-wand.h"
//#include "qdbmp.h"


// --- Constants
//static const int MAX_CYCLES = 9000; //5 mins
static const int MAX_CYCLES = 1800; //1 mins

//static const int CYCLES_TO_RESET_BASE = 240;

int water_tc_differential = -35;
int body_tc_differential = 120;
int fire_tc_differential = 500;
int water_min_detected_pc = 300;
int body_min_detected_pc = 1000;
int fire_min_detected_pc = 100;
bool water_tc_diff_is_negative = false;
static int lepton_reference_array[60][80];
static int current_lepton_array[60][80];

typedef struct {
	bool water_detected;
	bool body_detected;
	bool fire_detected;
	int minValue;
	int maxValue;
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


static const uint8_t WATER_DETECTED_SECS = 4;
static const uint8_t WATER_DETECTED_CONFIRMED_SECS_TO_SHUTDOWN = 4;

static bool water_detected = false;
static bool water_detected_confirmed = false;

static float water_detected_cycle_count;



static const int colormap[] = {1, 3, 74, 0, 3, 74, 0, 3, 75, 0, 3, 75, 0, 3, 76, 0, 3, 76, 0, 3, 77, 0, 3, 79, 0, 3, 82, 0, 5, 85, 0, 7, 88, 0, 10, 91, 0, 14, 94, 0, 19, 98, 0, 22, 100, 0, 25, 103, 0, 28, 106, 0, 32, 109, 0, 35, 112, 0, 38, 116, 0, 40, 119, 0, 42, 123, 0, 45, 128, 0, 49, 133, 0, 50, 134, 0, 51, 136, 0, 52, 137, 0, 53, 139, 0, 54, 142, 0, 55, 144, 0, 56, 145, 0, 58, 149, 0, 61, 154, 0, 63, 156, 0, 65, 159, 0, 66, 161, 0, 68, 164, 0, 69, 167, 0, 71, 170, 0, 73, 174, 0, 75, 179, 0, 76, 181, 0, 78, 184, 0, 79, 187, 0, 80, 188, 0, 81, 190, 0, 84, 194, 0, 87, 198, 0, 88, 200, 0, 90, 203, 0, 92, 205, 0, 94, 207, 0, 94, 208, 0, 95, 209, 0, 96, 210, 0, 97, 211, 0, 99, 214, 0, 102, 217, 0, 103, 218, 0, 104, 219, 0, 105, 220, 0, 107, 221, 0, 109, 223, 0, 111, 223, 0, 113, 223, 0, 115, 222, 0, 117, 221, 0, 118, 220, 1, 120, 219, 1, 122, 217, 2, 124, 216, 2, 126, 214, 3, 129, 212, 3, 131, 207, 4, 132, 205, 4, 133, 202, 4, 134, 197, 5, 136, 192, 6, 138, 185, 7, 141, 178, 8, 142, 172, 10, 144, 166, 10, 144, 162, 11, 145, 158, 12, 146, 153, 13, 147, 149, 15, 149, 140, 17, 151, 132, 22, 153, 120, 25, 154, 115, 28, 156, 109, 34, 158, 101, 40, 160, 94, 45, 162, 86, 51, 164, 79, 59, 167, 69, 67, 171, 60, 72, 173, 54, 78, 175, 48, 83, 177, 43, 89, 179, 39, 93, 181, 35, 98, 183, 31, 105, 185, 26, 109, 187, 23, 113, 188, 21, 118, 189, 19, 123, 191, 17, 128, 193, 14, 134, 195, 12, 138, 196, 10, 142, 197, 8, 146, 198, 6, 151, 200, 5, 155, 201, 4, 160, 203, 3, 164, 204, 2, 169, 205, 2, 173, 206, 1, 175, 207, 1, 178, 207, 1, 184, 208, 0, 190, 210, 0, 193, 211, 0, 196, 212, 0, 199, 212, 0, 202, 213, 1, 207, 214, 2, 212, 215, 3, 215, 214, 3, 218, 214, 3, 220, 213, 3, 222, 213, 4, 224, 212, 4, 225, 212, 5, 226, 212, 5, 229, 211, 5, 232, 211, 6, 232, 211, 6, 233, 211, 6, 234, 210, 6, 235, 210, 7, 236, 209, 7, 237, 208, 8, 239, 206, 8, 241, 204, 9, 242, 203, 9, 244, 202, 10, 244, 201, 10, 245, 200, 10, 245, 199, 11, 246, 198, 11, 247, 197, 12, 248, 194, 13, 249, 191, 14, 250, 189, 14, 251, 187, 15, 251, 185, 16, 252, 183, 17, 252, 178, 18, 253, 174, 19, 253, 171, 19, 254, 168, 20, 254, 165, 21, 254, 164, 21, 255, 163, 22, 255, 161, 22, 255, 159, 23, 255, 157, 23, 255, 155, 24, 255, 149, 25, 255, 143, 27, 255, 139, 28, 255, 135, 30, 255, 131, 31, 255, 127, 32, 255, 118, 34, 255, 110, 36, 255, 104, 37, 255, 101, 38, 255, 99, 39, 255, 93, 40, 255, 88, 42, 254, 82, 43, 254, 77, 45, 254, 69, 47, 254, 62, 49, 253, 57, 50, 253, 53, 52, 252, 49, 53, 252, 45, 55, 251, 39, 57, 251, 33, 59, 251, 32, 60, 251, 31, 60, 251, 30, 61, 251, 29, 61, 251, 28, 62, 250, 27, 63, 250, 27, 65, 249, 26, 66, 249, 26, 68, 248, 25, 70, 248, 24, 73, 247, 24, 75, 247, 25, 77, 247, 25, 79, 247, 26, 81, 247, 32, 83, 247, 35, 85, 247, 38, 86, 247, 42, 88, 247, 46, 90, 247, 50, 92, 248, 55, 94, 248, 59, 96, 248, 64, 98, 248, 72, 101, 249, 81, 104, 249, 87, 106, 250, 93, 108, 250, 95, 109, 250, 98, 110, 250, 100, 111, 251, 101, 112, 251, 102, 113, 251, 109, 117, 252, 116, 121, 252, 121, 123, 253, 126, 126, 253, 130, 128, 254, 135, 131, 254, 139, 133, 254, 144, 136, 254, 151, 140, 255, 158, 144, 255, 163, 146, 255, 168, 149, 255, 173, 152, 255, 176, 153, 255, 178, 155, 255, 184, 160, 255, 191, 165, 255, 195, 168, 255, 199, 172, 255, 203, 175, 255, 207, 179, 255, 211, 182, 255, 216, 185, 255, 218, 190, 255, 220, 196, 255, 222, 200, 255, 225, 202, 255, 227, 204, 255, 230, 206, 255, 233, 208};



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

static int currentIndexShift = 0;
static bool creatingImage = false;

unsigned char* convert_array_to_image_data (int tcArray[60][80], int minValue, int maxValue)
{
	creatingImage = true;
	
	uint16_t pixValue;
	
	float diff = maxValue - minValue;
	float scale = 255/diff;
	
	printf("Max: %d\n", maxValue);
	printf("Min: %d\n", minValue);
	printf("Diff: %3.6f\n", diff);
	printf("Scale: %3.6f\n", scale);
	
	MagickWand *m_wand = NULL;
	PixelWand *p_wand = NULL;
	PixelIterator *iterator = NULL;
	PixelWand **pixels = NULL;
	int x,y;
	char hex[128];

	MagickWandGenesis();

	p_wand = NewPixelWand();
	PixelSetColor(p_wand,"white");
	m_wand = NewMagickWand();
	// Create a 100x100 image with a default of white
	MagickNewImage(m_wand,80,60,p_wand);
	// Get a new pixel iterator 
	iterator=NewPixelIterator(m_wand);
	for(y=0;y<60;y++) {
		// Get the next row of the image as an array of PixelWands
		pixels=PixelGetNextIteratorRow(iterator,&x);
		// Set the row of wands to a simple gray scale gradient
		for(x=0;x<80;x++) {
			pixValue = (tcArray[y][x] - minValue) * scale;
			if(pixValue > 0)
				pixValue = pixValue - (pixValue % 3);
			
			sprintf(hex,"#%02x%02x%02x",colormap[3*pixValue], colormap[3*pixValue+1], colormap[3*pixValue+2]);
			PixelSetColor(pixels[x],hex);
		}
		// Sync writes the pixels back to the m_wand
		PixelSyncIterator(iterator);
	}
	
	MagickResizeImage(m_wand,480,360,LanczosFilter,1);
	
	unsigned char *block = (unsigned char*)malloc(480*360*3);
	MagickExportImagePixels(m_wand,0,0,480,360, "RGB", CharPixel, block);
	
	// Clean up
	iterator=DestroyPixelIterator(iterator);
	DestroyMagickWand(m_wand);
	MagickWandTerminus();

		
	creatingImage = false;	
	
	return block;
	//printf("Palette size: %d\n", sizeof(colormap_rainbow) /sizeof(*colormap_rainbow));
}


gboolean set_video_frame(gpointer data){
	//video_area_expose ((GtkWidget*)videoArea, NULL, data);
	video_area_expose ((GtkWidget*)videoArea, (unsigned char*)data);
	return FALSE;
}

static void que_video_frame(int tcArray[60][80], int minValue, int maxValue)
{
	if(!creatingImage){
		//BMP *bmp = convert_array_to_bmp(tcArray, minValue, maxValue);
		
		//BMP *bmp = BMP_ReadFile( "capture.bmp" );
		
		unsigned char * block = convert_array_to_image_data(tcArray, minValue, maxValue);
		
		g_main_context_invoke(mainc, set_video_frame, (gpointer)block);
	}
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




DetectionResults read_lepton_array(int currentIteration, int referenceArray[60][80], int compareArray[60][80])
{
	// Initialize the result struct
	DetectionResults dResults;
	dResults.water_detected = false;
	dResults.body_detected = false;
	dResults.fire_detected = false;
	dResults.minValue = INT_MAX;
	dResults.maxValue = 0;
	
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
			
			if(compareArray[i][j] < dResults.minValue)
				dResults.minValue = compareArray[i][j];
			if(compareArray[i][j] > dResults.maxValue)
				dResults.maxValue = compareArray[i][j];
			
			
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

	
	return dResults;
}


void* f_monitor(void *arg)
{
	_monitorActive = true;
	water_tc_diff_is_negative = water_tc_differential < 0;
	water_detected = false;
	water_detected_confirmed = false;
	water_detected_cycle_count = 0;
	
	int currentIteration = 0;
	int fd;
	char *msg;
	
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

		
			DetectionResults results = read_lepton_array(currentIteration, lepton_reference_array, current_lepton_array);
			
			if(results.water_detected)
			{
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				water_detected_cycle_count++;
				
				// First detection?
				if(!water_detected)
				{
					water_detected = true;
					//g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
					msg = "Water Detected! Looking for flooding.";
					g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)msg);
				}
				else if(water_detected && !water_detected_confirmed && (water_detected_cycle_count/30) > WATER_DETECTED_SECS)
				{
					water_detected_cycle_count = 0;
					water_detected_confirmed = true;
					msg = "Flooding confirmed! Commencing shutdown!";
					g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)msg);
				}
				else if(water_detected && water_detected_confirmed && (water_detected_cycle_count/30) > WATER_DETECTED_CONFIRMED_SECS_TO_SHUTDOWN)
				{
					pabort("Water Shutdown Complete!");
				}
				
			}
			else if(results.fire_detected)
			{
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				//g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
				pabort("Fire Detected");
				//char *s = "Fire Detected!";
				//g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
			}
			else if(!results.water_detected)
			{
				water_detected = false;
				water_detected_confirmed = false;
				water_detected_cycle_count = 0;
				msg = "Monitoring";
				g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)msg);
			}
/*
			else if(results.body_detected){
				
				printf("Water detected: %d\n", results.water_detected);
				printf("Body detected: %d\n", results.body_detected);
				printf("Fire detected: %d\n", results.fire_detected);
				
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
				pabort("Body heat Detected");
				//char *s = "Body heat Detected!";
				//g_main_context_invoke(mainc, set_status_monitor_status, (gpointer)s);
			}
*/
			
			
			
			
			// Update the image if first iteration
			if(currentIteration == 1){
				g_main_context_invoke(mainc, set_capture_image, (gpointer)captureImage);
			}
			
			if(currentIteration % 2 ==0){
			//if(currentIteration == 2){
				que_video_frame(current_lepton_array, results.minValue, results.maxValue);
			}

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


/*********************************** Calibration Methods **********************************/
// <editor-fold>

int get_tc_diff_from_array(int min_diff, int min_pixel_count, int referenceArray[60][80], int compareArray[60][80])
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

// </editor-fold>