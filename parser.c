#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG


typedef struct {
  int type;//1 - sphere, 2 - plane.
  double *color;
  double radius;
  double *position;
  double *normal;
}Shape;

typedef struct{
  double height;
  double width;
  Shape *shapes;
}Scene;

int line = 1;

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
  fscanf(json, "%f", &value);
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
  Scene scene;
  scene.shapes = malloc(sizeof(Scene*)*2);//Malloc!
  int oCount = 0;
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
      printf("Were done, lets see if you suck!\n");
      printf("The camera has a height %d, and a width %d\n",
	     scene.width, scene.height);
      for(int i = 0; i <= oCount; i++){
	if(scene.shapes[i].type == 1){
	  printf("The  object is a sphere with color %d, position %d, and radius %d\n",scene.shapes[i].color, scene.shapes[i].position, scene.shapes[i].radius);
	}else if(scene.shapes[i].type==2){
	   printf("The  object is a plane with color %d, position %d, and radiusnormal %d\n",scene.shapes[i].color, scene.shapes[i].position, scene.shapes[i].normal);
	}else{
	  printf("Yep, you suck!\n");
	}
      }
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
      } else if (strcmp(value, "sphere") == 0) {
	scene.shapes[oCount].type = 1;
      } else if (strcmp(value, "plane") == 0) {
	scene.shapes[oCount].type = 2;
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
	    double value = next_number(json);
	    scene.width = value;
	  }else if (strcmp(key, "height") == 0){
	    double value = next_number(json);
	    scene.height = value;
	  }else if (strcmp(key, "radius") == 0){
	    double value = next_number(json);
	    scene.shapes[oCount].radius = value;
	  }else if (strcmp(key, "color") == 0){
	    double* value = next_vector(json);
	    scene.shapes[oCount].color = value;
	  }else if (strcmp(key, "position") == 0){
	    double* value = next_vector(json);
	    scene.shapes[oCount].position = value;
	  }else if (strcmp(key, "normal") == 0){
	    double* value = next_vector(json);
	    scene.shapes[oCount].normal = value;
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
  read_scene(argv[1]);
  return 0;
}
