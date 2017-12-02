//
//  csv.c
//  Raycasting
//

//
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include<errno.h>
#include "csv.h"
/*******************************************This is Dr.Palmer's code*******************************/
// next_c wraps the getc function that provides error checking and line #
// Problem: if we do ungetc, it could screw us up on the line #


int line = 1;  // global variable, it will tells us which line is not correct

OBJECT objects[MAX_OBJECTS]; // Allocate an array for All Objects in csv File
LIGHT lights[MAX_OBJECTS];      // allocate space for lights

int num_lights;
int num_objects;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* csv) {
    int c = fgetc(csv);

    if (c == '\n') {
        line++;;
    }
    return c;
}

// skipWS() skips white space in the file.
void skipWS(FILE* csv) {
    int c = next_c(csv);
    while (isspace(c)) {
        c = next_c(csv);
    }
    ungetc(c, csv);
}


// lookfor() checks that the next character is d.
// It is not d, give us an error.
void lookfor(FILE* csv, int d) {
    int c = next_c(csv);
    if (c == d) return;
    fprintf(stderr, "Error: Expected '%c': %d\n", d, line);
    exit(1);
}

double next_number(FILE* csv) {
    double value;
    int res = fscanf(csv, "%lf", &value);
    if (res == EOF) {
        fprintf(stderr, "Error: Expected a number but found EOF: %d\n", line);
        exit(1);
    }
    //printf("next_number: %lf\n", value);
    return value;
}

/* since we could use 0-255 or 0-1 or whatever, this function checks bounds */
int check_color_val(double v) {
    if (v < 0.0 || v > MAX_COLOR_VAL)
        return 0;
    return 1;
}


/* check bounds for colors in csv light objects. These can be anything >= 0 */
int check_light_color_val(double v) {
    if (v < 0.0)
        return 0;
    return 1;
}


double* next_vector(FILE* csv) {
    double* v = malloc(3*sizeof(double));
    lookfor(csv, '[');
    skipWS(csv);
    v[0] = next_number(csv);
	printf("vector: %lf, ",v[0]);
    skipWS(csv);
    lookfor(csv, ',');
    skipWS(csv);
    v[1] = next_number(csv);
	printf("%lf, ",v[1]);
    skipWS(csv);
    lookfor(csv, ',');
    skipWS(csv);
    v[2] = next_number(csv);
	printf("%lf\n",v[2]);
    skipWS(csv);
    lookfor(csv, ']');
    return v;
}




/* Checks that the next 3 values in the FILE are valid rgb numbers */
double* next_color(FILE* csv, boolean is_rgb) {
    double* v = malloc(sizeof(double)*3);
    skipWS(csv);
    lookfor(csv, '[');
    skipWS(csv);
    v[0] = next_number(csv);
	printf("vector %lf, ",v[0]);
    skipWS(csv);
    lookfor(csv, ',');
    skipWS(csv);
    v[1] = next_number(csv);
	printf("%lf, ",v[1]);
    skipWS(csv);
    lookfor(csv, ',');
    skipWS(csv);
    v[2] = next_number(csv);
	printf("%lf\n",v[2]);
    skipWS(csv);
    lookfor(csv, ']');
    // check that all values are valid
    if (is_rgb) {
        if (!check_color_val(v[0]) ||
            !check_color_val(v[1]) ||
            !check_color_val(v[2])) {
            fprintf(stderr, "Error: next_color: rgb value out of range: %d\n", line);
            exit(1);
        }
    }
    else {
        if (!check_light_color_val(v[0]) ||
            !check_light_color_val(v[1]) ||
            !check_light_color_val(v[2])) {
            fprintf(stderr, "Error: next_color: light value out of range: %d\n", line);
            exit(1);
        }
        
    }
    return v;
}

// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* csv) {
    char buffer[129];
    char c = next_c(csv);
    int i = 0;
    while ((c != ',' )&& (c!=':')) {   // as long as c is not EOF or comma, the string is not ended.
		if (c ==EOF){
			break;
		}
        if (i >= 128) {
            fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
            exit(1);
        }
        if (c == '\\') {
            fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
            exit(1);
        }
        buffer[i] = c;
        i =i+ 1;
        c = next_c(csv);
		buffer[i]='\0';
    }
	ungetc(c,csv);  
	//printf("%s \n" , strdup(buffer));	// unget the one more character. 
	
    return strdup(buffer);
}


// This function at here is for quadric
double* next_coefficient(FILE* csv){
    double* v = malloc(10*sizeof(double));
    
    skipWS(csv);
    lookfor(csv, '[');
    skipWS(csv);
    v[0] = next_number(csv);
    for(int i=1;i<10;i++){
        skipWS(csv);
        lookfor(csv, ',');
        skipWS(csv);
        v[i] = next_number(csv);
    }
    skipWS(csv);
    lookfor(csv, ']');
    return v;
}

void read_scene(const char* filename) {
    FILE* csv = fopen(filename, "r");
    
    if (csv == NULL) {
        fprintf(stderr, "Error: Could not open file\n");
        exit(1);
    }
    skipWS(csv);
    
    // find beginning of the list
    int c  = next_c(csv);
    if (c != 'c') {
        fprintf(stderr, "Error: read_csv: csv file must begin with camera \n");
        exit(1);
    }
	ungetc(c,csv);
    
    // check if file empty
    if (c == EOF) {
        fprintf(stderr, "Error: read_csv: Empty csv file\n");
        exit(1);
    }
    skipWS(csv);
    
    int object_counter = 0;
    int light_counter = 0;
    int object_type = 0;
    boolean find_object = true;
    // find the objects
	char *type = next_string(csv);
    while (find_object) {
        //c  = next_c(csv);
        if (object_counter > MAX_OBJECTS) {
            fprintf(stderr, "Error: read_csv: Number of objects is too large: %d\n", line);
            exit(1);
        }
		if (strcmp(type, "camera") == 0) {
			object_type = CAM;
			objects[object_counter].type = CAM;
		}
		else if (strcmp(type, "sphere") == 0) {
			object_type = SPH;
			objects[object_counter].type = SPH;
			//printf("type: %d \n",object_type);
		}
		else if (strcmp(type, "plane") == 0) {
			object_type = PLAN;
			objects[object_counter].type = PLAN;
			//printf("type: %d \n",object_type);
		}
		else if (strcmp(type, "quadric") == 0) {
			object_type = QUAD;
			objects[object_counter].type = QUAD;
		}
		else if (strcmp(type, "light") == 0) {
			object_type = LIG;
			//printf("type: %d \n",object_type);
		}
		else {
			break;
		}
        lookfor(csv,',');   
        skipWS(csv);
            
            while (true) {
                //  , }
                char *property = next_string(csv);
				printf("word %s \n",property);
                if (strcmp(property, "width") == 0) {
					if (object_type != CAM) {
						fprintf(stderr, "Error: read_csv: Width cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("width %lf \n",temp);
					if (temp <= 0) {
						fprintf(stderr, "Error: read_csv: width must be positive: %d\n", line);
						exit(1);
					}
					objects[object_counter].camera.width = temp;
					lookfor(csv,',');
					skipWS(csv);
                }
				else if (strcmp(property, "height") == 0) {
					if (object_type != CAM) {
						fprintf(stderr, "Error: read_csv: Width cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("height %lf \n",temp);
					if (temp <= 0) {
						fprintf(stderr, "Error: read_csv: height must be positive: %d\n", line);
						exit(1);
					}
					objects[object_counter].camera.height = temp;
					skipWS(csv);
				}
				else if (strcmp(property, "radius") == 0) {
					if (object_type != SPH) {
						fprintf(stderr, "Error: read_csv: Radius cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("radius %lf \n",temp);
					if (temp <= 0) {
						fprintf(stderr, "Error: read_csv: radius must be positive: %d\n", line);
						exit(1);
					}
					objects[object_counter].sphere.radius = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "theta") == 0) {
					if (object_type != LIG) {
						fprintf(stderr, "Error: read_csv: Theta cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("theta %lf \n",temp);
					if (temp > 0.0) {
						lights[light_counter].type = SPOTLIG;
					}
					else if (temp < 0.0) {
						fprintf(stderr, "Error: read_csv: theta must be >= 0: %d\n", line);
						exit(1);
					}
					lights[light_counter].theta_deg = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
                    
				else if (strcmp(property, "radial-a0") == 0) {
					if (object_type != LIG) {
						fprintf(stderr, "Error: read_csv: Radial-a0 cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("radius0 %lf \n",temp);
					if (temp < 0) { // TODO: find out if this should be <=
						fprintf(stderr, "Error: read_csv: radial-a0 must be positive: %d\n", line);
						exit(1);
					}
					lights[light_counter].rad_att0 = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "radial-a1") == 0) {
					if (object_type != LIG) {
						fprintf(stderr, "Error: read_csv: Radial-a0 cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("radius1 %lf \n",temp);
					if (temp < 0) { // TODO: find out if this should be <=
						fprintf(stderr, "Error: read_csv: radial-a1 must be positive: %d\n", line);
						exit(1);
					}
					lights[light_counter].rad_att1 = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "radial-a2") == 0) {
					if (object_type != LIG) {
						fprintf(stderr, "Error: read_csv: Radial-a0 cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					printf("radius2 %lf \n",temp);
					if (temp < 0) { // TODO: find out if this should be <=
						fprintf(stderr, "Error: read_csv: radial-a2 must be positive: %d\n", line);
						exit(1);
					}
					lights[light_counter].rad_att2 = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "angular-a0") == 0) {
					if (object_type != LIG) {
						fprintf(stderr, "Error: read_csv: Radial-a0 cannot be set on this type: %d\n", line);
						exit(1);
					}
					lookfor(csv,':');
					skipWS(csv);
					double temp = next_number(csv);
					if (temp < 0) { // TODO: find out if this should be <=
						fprintf(stderr, "Error: read_csv: angular-a0 must be positive: %d\n", line);
						exit(1);
					}
					lights[light_counter].ang_att0 = temp;
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "color") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type != LIG) {
						fprintf(stderr, "Error: Just plain 'color' vector can only be applied to a light object\n");
						exit(1);
					}
					lights[light_counter].color = next_color(csv, false);
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "direction") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type != LIG) {
						fprintf(stderr, "Error: Direction vector can only be applied to a light object\n");
						exit(1);
					}
					lights[light_counter].type = SPOTLIG;
					lights[light_counter].direction = next_vector(csv);
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "specular_color") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == SPH){
						objects[object_counter].sphere.spec_color = next_color(csv, true);
					}
					else if (object_type == PLAN){
						objects[object_counter].plane.spec_color = next_color(csv, true);
					}
					else if (object_type == QUAD){
						objects[object_counter].quadric.spec_color = next_color(csv,true);
					}
					else {
						fprintf(stderr, "Error: read_csv: speculaor_color vector can't be applied here: %d\n", line);
						exit(1);
					}
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "diffuse_color") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == SPH){
						objects[object_counter].sphere.diff_color = next_color(csv, true);
					}
					else if (object_type == PLAN){
						objects[object_counter].plane.diff_color = next_color(csv, true);
					}
					else if (object_type == QUAD){
						objects[object_counter].quadric.diff_color = next_color(csv,true);
					}
					else {
						fprintf(stderr, "Error: read_csv: diffuse_color vector can't be applied here: %d\n", line);
						exit(1);
					}
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "position") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == SPH){
						objects[object_counter].sphere.position = next_vector(csv);
					}
					else if (object_type == PLAN){
						objects[object_counter].plane.position = next_vector(csv);
					}
					else if (object_type == LIG){
						lights[light_counter].position = next_vector(csv);
					}
					else {
						fprintf(stderr, "Error: read_csv: Position vector can't be applied here: %d\n", line);
						exit(1);
					}
					skipWS(csv);
					
				}
				else if (strcmp(property, "reflextivity") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == SPH){
						objects[object_counter].sphere.reflect = next_number(csv);
						printf("%lf\n",objects[object_counter].sphere.reflect);
					}
					else if(object_type == PLAN){
						objects[object_counter].plane.reflect = next_number(csv);
					}
					else if (object_type == QUAD){
						objects[object_counter].quadric.reflect = next_number(csv);
					}
					else{
						fprintf(stderr, "Error: read_csv: Reflectivity can't be applied here: %d\n", line);
						exit(1);
					}
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "refractivity") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == SPH){
						objects[object_counter].sphere.refract = next_number(csv);
						printf("%lf\n",objects[object_counter].sphere.refract);
					}
					else if(object_type == PLAN){
						objects[object_counter].plane.refract = next_number(csv);
					}
					else if (object_type == QUAD){
						objects[object_counter].quadric.refract = next_number(csv);
					}
					else{
						fprintf(stderr, "Error: read_csv: Refractivity can't be applied here: %d\n", line);
						exit(1);
					}
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "ior") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type == PLAN){
						objects[object_counter].plane.ior = next_number(csv);
						printf("%lf\n",objects[object_counter].plane.ior);
					}
					else if(object_type == SPH){
						objects[object_counter].sphere.ior = next_number(csv);
						printf("%lf\n",objects[object_counter].sphere.ior);
					}
					else if (object_type == QUAD){
						objects[object_counter].quadric.ior = next_number(csv);
					}
					else{
						fprintf(stderr, "Error: read_csv: ior can't be applied here: %d\n", line);
						exit(1);
					}
					lookfor(csv,',');
					skipWS(csv);
				}
				else if (strcmp(property, "normal") == 0) {
					lookfor(csv,':');
					skipWS(csv);
					if (object_type != PLAN) {
						fprintf(stderr, "Error: read_csv: Normal vector can't be applied here: %d\n", line);
						exit(1);
					}
					else{
						objects[object_counter].plane.normal = next_vector(csv);
					}
					lookfor(csv,',');
					skipWS(csv);
					
				}
                else {
					type = property;
					break;
                }
            }

        
        if (object_type == LIG){
            light_counter++;
        }
        
        else{
            object_counter++;
        }
    }
    fclose(csv);
    num_lights = light_counter;
    num_objects = object_counter;
}


//Get Objects
void get_objects(OBJECT *object) {
    int i = 0;
    while (i < MAX_OBJECTS && object[i].type > 0) {
        printf("object type: %d\n", object[i].type);
        if (object[i].type == CAM) {
            printf("height: %lf\n", object[i].camera.height);
            printf("width: %lf\n", object[i].camera.width);
        }
        else if (object[i].type == SPH) {
            printf("color: %lf %lf %lf\n", object[i].sphere.spec_color[0],
                   object[i].sphere.spec_color[1],
                   object[i].sphere.spec_color[2]);
            printf("position: %lf %lf %lf\n", object[i].sphere.position[0],
                   object[i].sphere.position[1],
                   object[i].sphere.position[2]);
            printf("radius: %lf\n", object[i].sphere.radius);
        }
        else if (object[i].type == PLAN) {
            printf("color: %lf %lf %lf\n", object[i].plane.spec_color[0],
                   object[i].plane.spec_color[1],
                   object[i].plane.spec_color[2]);
            printf("position: %lf %lf %lf\n", object[i].plane.position[0],
                   object[i].plane.position[1],
                   object[i].plane.position[2]);
            printf("normal: %lf %lf %lf\n", object[i].plane.normal[0],
                   object[i].plane.normal[1],
                   object[i].plane.normal[2]);
        }
        else if (object[i].type == QUAD){
            printf("coefficient: %lf %lf %lf %lf %lf %lf %lf  %lf %lf %lf\n", object[i].quadric.spec_color[0],
                   object[i].quadric.spec_color[1],object[i].quadric.spec_color[2],object[i].quadric.spec_color[3],object[i].quadric.spec_color[4],object[i].quadric.spec_color[5],object[i].quadric.spec_color[6],object[i].quadric.spec_color[7],object[i].quadric.spec_color[8],object[i].quadric.spec_color[9]);
        }
        else {
            printf("unsupported value\n");
        }
        i++;
    }
    printf("end at i=%d\n", i);
}
