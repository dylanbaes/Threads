
#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void * compute_image( void * arg  );
int threads = 1;

struct parameters{
	struct bitmap *bm;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int id;
};

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-n <threads> Number of threads. (default=1)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	const char *outfile = "mandel.bmp";
	double xcenter = 0;
	double ycenter = 0;
	double scale = 4;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 1000;
	threads = 1;
	

	struct timeval begin;
	struct timeval end;

	// For each command line argument given,
	// override the appropriate configuration value.
	gettimeofday(&begin, NULL);
	while((c = getopt(argc,argv,"x:y:s:n:W:H:m:o:h"))!=-1) {
		switch(c) {
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'n':
				threads = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}
	// divide image height by the amount of threads there are (?)

	// Display the configuration of the image.
	struct parameters params[threads];

	

	printf("mandel: x=%lf y=%lf scale=%lf threads=%d max=%d outfile=%s\n",xcenter,ycenter,scale,threads,max,outfile);
	
	//printf("xmin= %lf xmax=%lf ymin=%lf ymax=%lf max=%d\n", image.xmin, image.xmax, image.ymin, image.ymax, image.max);

	pthread_t tids[threads]; // declare a pthread array that will house the thread addresses
	// iterate through a for loop and pthread create and join the created threads

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	for (int i = 0; i < threads; i++) {
		params[i].bm = bm;
		params[i].xmin = xcenter-scale;
		params[i].xmax = xcenter+scale;
		params[i].ymin = ycenter-scale;
		params[i].ymax = ycenter+scale;
		params[i].max = max;
		params[i].id = i;
	}

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

	// Compute the Mandelbrot image
	for (int i = 0; i < threads; i++) {
		pthread_create(&tids[i], NULL, compute_image, (void *) &params[i]);
	}

	for (int i = 0; i < threads; i++) {
		pthread_join(tids[i], NULL);
	}
	//compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);


	gettimeofday(&end, NULL);

	long execution = ( end.tv_sec * 1000000 + end.tv_usec ) - ( begin.tv_sec * 1000000 + begin.tv_usec );
	printf("This code took %ld microseconds to execute\n", execution);


	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}

	
	 
	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void * compute_image( void * arg )
{
	int i,j;

	struct parameters * params = (struct parameters *) arg;

	int width = bitmap_width(params->bm);
	int height = bitmap_height(params->bm);

	int begin = params->id * height / threads;
	int end = (begin + height / threads); // (?) There is a discrepancy in rendering the image line by line
	// there are a specific number of blue lines in the picture that correspond with n
	// if there are 5 threads there are 5 blue lines seen. But this is not the case for n = 10
	// is it because of the inaccuracy when dividing integers?

	// For every pixel in the image...

	for(j=begin;j<end;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = params->xmin + i*((params->xmax)-(params->xmin))/width;
			double y = params->ymin + j*((params->ymax)-(params->ymin))/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,params->max);

			// Set the pixel in the bitmap.
			bitmap_set(params->bm,i,j,iters);
		}
	}

	return NULL;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}




