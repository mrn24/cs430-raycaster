all: parser.c
	gcc parser.c -o raycast -lm

clean:
	rm -rf raycast *~
