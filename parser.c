#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


typedef struct {
  int type;//1 - sphere, 2 - plane.
  double* color;
  double radius;
  double* position;
  double* normal;
}Shape;

Shape shapes[180];

int height;
int width;
int oCount=0;
double cameraheight;
double camerawidth;
double* viewplane;

int line = 1;

void fill_viewplane(){
  printf("Filling Viewplane\n");
  int index = 0;
  int i, j;
  for(i=0;i<width;i++){
    for(j=0;j<height;j++){
      viewplane[index][0] = 0.1;
      viewplane[index][1] = 0.1;
      viewplane[index][2] = 0.1;
      index++;
    }
  }
}

int caster(){
  viewplane = (double*)malloc(sizeof(double)*3*width*height);
  double x_pos, y_pos, z_pos, x_dist, y_dist, z_dist, distance, a;
  double mag,t;
  double t_min=-1.0;
  int index = 0;
  int i, j, k;
  fill_viewplane();
  for (i=0;i<width;i++){
    for(j=0;j<height;j++){
      x_pos = 0 - camerawidth/2+camerawidth/width*j+0.5;
      y_pos = 0 - cameraheight/2+cameraheight/height*i+0.5;
      mag = sqrt(pow(x_pos, 2)+pow(y_pos, 2)+1);
      x_pos = x_pos/mag;
      y_pos = y_pos/mag;
      z_pos = 1/mag;
      for(k=0; k<oCount; k++){
	// printf("width, height, objects %d %d %f\n", i, j, k);
	if(shapes[k].type = 1){
	  t=((x_pos*shapes[k].position[0])+(y_pos*shapes[k].position[1])+(z_pos*shapes[k].position[2]))/(x_pos+y_pos+z_pos);
	  x_dist = x_pos*t;
	  y_dist = x_pos*t;
	  z_dist = z_pos*t;
	  distance = sqrt(pow(x_dist - shapes[k].position[0], 2)+pow(y_dist - shapes[k].position[1], 2)+pow(z_dist - shapes[k].position[2], 2));
	  if(distance <= shapes[k].radius){
	    a = sqrt(pow(shapes[k].radius, 2) - pow(distance, 2));
	    t = t-a;
	    if(t_min == -1 || t<t_min){
	      t_min = t;
	      viewplane[index++]=shapes[k].color[0];
	      viewplane[index++]=shapes[k].color[1];
	      viewplane[index++]=shapes[k].color[2];
	    }
	  }
	}
      }
    }
    index=index+3;
  }
  for(i=0;i<width;i++){
    for(j=0;j<height;j++){
      printf("%.1f\n", *(*(viewplane+i)+j));
    }
    printf("\n");
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
	  }else if (strcmp(key, "normal") == 0){
	    shapes[oCount].normal = next_vector(json);
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
  width = 5;
  height = 5;
  read_scene(argv[1]);
  // print_scene();
  caster();
  return 0;
}
