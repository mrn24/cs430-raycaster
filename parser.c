#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct RGBpixel {
  unsigned char r, g, b;
} RGBpixel;

typedef struct Image{
  int width, height, format, range, count;
  RGBpixel *buffer;
}Image;

typedef struct {
  int type;//1 - sphere, 2 - plane.
  double* color;
  double radius;
  double* position;
  double* normal;
}Shape;

Shape shapes[180];
Image image;

int height;
int width;
int oCount=0;
double cameraheight;
double camerawidth;
double* viewplane;

int line = 1;

int write_p6(char* input){
  FILE* fp = fopen(input, "wb");
  fprintf(fp, "P6\n");
  fprintf(fp, "#This document was converted from json to P6 by my converter\n", image.format);
  fprintf(fp, "%d %d\n%d\n", image.width, image.height, image.range);
  fwrite(image.buffer, sizeof(RGBpixel), image.height * image.width, fp);
  /*for(int i = 0; i<image.count; i++){
    fwrite(fp, "%d%d%d", image.buffer[i].r, image.buffer[i].g, image.buffer[i].b);
    }*/
  fclose(fp);
  return 0;
}

int write_p3(char* input){
  FILE* fp = fopen(input, "w");
  fprintf(fp, "P3\n");
  fprintf(fp, "#This document was converted from json to P3 by my converter\n", image.format);
  fprintf(fp, "%d %d\n%d\n", image.width, image.height, image.range);
  for(int i = 0; i<height*width; i++){
    fprintf(fp, "%d\n%d\n%d\n", image.buffer[i].r, image.buffer[i].g, image.buffer[i].b);
  }
  fclose(fp);
  return 0;
}

double quadratic(double a, double b, double c){
   double t0 = (-b-sqrt((pow(b, 2))-4*a*c))/2*a;
   double t1 = (-b+sqrt((pow(b, 2))-4*a*c))/2*a;

   if(t0<0) return t1;
   if(t0<t1) return t0;
   return -1;
}

void fill_viewplane(){
  printf("Filling Viewplane\n");
  int i;
  for(i=0;i<width*height;i++){
      image.buffer[i].r = 0.1;
      image.buffer[i].g = 0.1;
      image.buffer[i].b = 0.1;
  }
}

int caster(){
  image.buffer = malloc(sizeof(RGBpixel)*width*height);
  image.height = height;
  image.width = width;
  image.range = 255;
  double x_pos, y_pos, z_pos, x_dist, y_dist, z_dist, distance, a, b, c, t0, t1;
  double mag,t=0;
  double t_min=-1.0;
  image.count = 0;
  int index = 0;
  int i, j, k;
  fill_viewplane();
  for (i=0;i<height;i++){
    for(j=0;j<width;j++){
      t_min = -1;
      x_pos = 0 - camerawidth/2+camerawidth/width*(j+0.5);
      y_pos = 0 - cameraheight/2+cameraheight/height*(i+0.5);
      mag = sqrt(pow(x_pos, 2)+pow(y_pos, 2)+1);
      x_pos = x_pos/mag;
      y_pos = y_pos/mag;
      z_pos = 1/mag;
      for(k=0; k<oCount; k++){
	// printf("width, height, objects %d %d %f\n", i, j, k);
	if(shapes[k].type == 1){	  
	  a=pow(x_pos, 2)+pow(y_pos, 2)+pow(z_pos, 2);
	  b=2 * (x_pos * (0-shapes[k].position[0]) + y_pos * (0-shapes[k].position[1]) + z_pos * (0-shapes[k].position[2]));
	  c=(pow(0-shapes[k].position[0], 2) + pow(0-shapes[k].position[1], 2) + pow(0-shapes[k].position[2], 2)-pow(shapes[k].radius, 2));
	  
	  t=quadratic(a, b, c);
	  
	  if(t>0){
	    t_min = t;
	    //Take the double value. Mult by max color value(255)
	    //Cast to unsigned character. (u_char) number
	    image.buffer[i*width+j].r=(int)(shapes[k].color[0]*image.range);
	    image.buffer[i*width+j].g=(int)(shapes[k].color[1]*image.range);
	    image.buffer[i*width+j].b=(int)(shapes[k].color[2]*image.range);
	  }
	}
      }
    }
  }
  return 0;
}


void print_scene(){
  printf("\nThe camera's width is %f, and the heigth is %f\n", camerawidth, cameraheight);
  for(int i=0;i<oCount;i++){
    printf("Object: type %d\n", shapes[i].type);
    if(shapes[i].type == 1){
      printf("Radius: %f\n", shapes[i].radius);
    }
    for(int j=0;j<3;j++){
      printf("Color: %f\n", shapes[i].color[j]);
      printf("Position: %f\n", shapes[i].position[j]);
      if(shapes[i].type == 2){
	printf("Normal: %f\n", shapes[i].normal[j]);
      }
    }
  }
}

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);    
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }  
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);      
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);      
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

double next_number(FILE* json) {
  double value;
  fscanf(json, "%lf", &value);
  // Error check this..
  return value;
}

double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}


void read_scene(char* filename) {
  int c;
  FILE* json = fopen(filename, "r");
  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }
  
  skip_ws(json);
  
  // Find the beginning of the list
  expect_c(json, '[');
  
  skip_ws(json);
  
  // Find the objects
  while (1) {
    c = fgetc(json);
    if (c == ']') {
      fclose(json);
      return;
    }
    if (c == '{') {
      skip_ws(json);
      
      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
	fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
	exit(1);
      }
      
      skip_ws(json);
      
      expect_c(json, ':');
      
      skip_ws(json);
      
      char* value = next_string(json);
      
      if (strcmp(value, "camera") == 0) {
	oCount = oCount - 1;
      } else if (strcmp(value, "sphere") == 0) {
	shapes[oCount].type = 1;
      } else if (strcmp(value, "plane") == 0) {
	shapes[oCount].type = 2;
      } else {
	fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
	exit(1);
      }
      
      skip_ws(json);
      
      while (1) {
	// , }
	c = next_c(json);
	if (c == '}') {
	  // stop parsing this object
	  oCount = oCount + 1;
	  break;
	} else if (c == ',') {
	  // read another field
	  skip_ws(json);
	  char* key = next_string(json);
	  skip_ws(json);
	  expect_c(json, ':');
	  skip_ws(json);
	  if (strcmp(key, "width") == 0){
	    camerawidth = next_number(json);
	  }else if (strcmp(key, "height") == 0){
	    cameraheight = next_number(json);
	  }else if (strcmp(key, "radius") == 0){
	    shapes[oCount].radius = next_number(json);
	  }else if (strcmp(key, "color") == 0){
            shapes[oCount].color = next_vector(json);
	  }else if (strcmp(key, "position") == 0){
	    shapes[oCount].position = next_vector(json);
	    shapes[oCount].position[1] = -1*shapes[oCount].position[1];
	  }else if (strcmp(key, "normal") == 0){
	    shapes[oCount].normal = next_vector(json);
	    shapes[oCount].normal[1] = -1*shapes[oCount].normal[1];
	  }else{
	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d\n",
		    key, line);
	  }
	  skip_ws(json);
	} else {
	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
	  exit(1);
	}
      }
      skip_ws(json);
      c = next_c(json);
      if (c == ',') {
	// noop
	skip_ws(json);
      } else if (c == ']') {
	fclose(json);
	return;
      } else {
	fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
	exit(1);
      }
    }
  }
}


int main(int c, char** argv) {
  width = 1000;
  height = 1000;
  read_scene(argv[1]);
  print_scene();
  caster();
  write_p3("output.ppm");
  return 0;
}
