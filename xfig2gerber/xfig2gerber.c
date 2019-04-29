/* xfig2gerber.c: Generates a gerber format and excellon drill files for
   pcb generation from a xfig file. 


 Copyright (C) 1998-2009 Christian Kurtsiefer, <christian.kurtsiefer@gmail.com>

 This source code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Public License as published
 by the Free Software Foundation; either version 3 of the License,
 or (at your option) any later version.

 This source code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 Please refer to the GNU Public License for more details.

 You should have received a copy of the GNU Public License along with
 this source code; if not, write to:
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

--

   BASIC IDEA:

   Specific layers in xfig are used to end up in various
   gerber file layers to allow to implement most useful functions for PCBs
   to be implemented by graphical libraries. The basic conversion scaling
   factor is 25.4% to allow the decimal rastering in xfig to be used for
   snapping to standard distances, so 1cm in the xfig files corresponds to
   100 mil in the gerber file.

   Not all xfig primitives are implemented, partly out of convenience and
   partly out of lack of necessity. The implemented xfig primitives include:
   - filled polygons and lines
   - rectangular objects (filled and not filled)
   - circles (filled and not filled)
   - arcs (not filled)
   Text entries are specifically not implemented, but may be later if there
   is an obvious vectorization of fonts possible. Fillings are supposed to
   take place without fancy filling fractions or patterns.

   To allow for a proper assignment between various manufacturing files 
   (metalized layers, solder stop layers, silk screen layers), the following 
   xfig layer assignment is used:

   layer 00: contains hole information as white circles
   layer 02: contains comment info. not transferred to silk screen
   layer 03: as layer 02 
   layer 04: as layer 02
   layer 06: silk screen layer, component side
   layer 07: silk screen layer, solder side
   layer 08: additional solder mask, component side
   layer 09: additional solder mask, solder side
   layer 10: contains patterns which go on both external planes,
             but no internal planes. Color code is black (recommended)
             insulated from inner layers.
   layer 11: as layer 10, but will not go to top solder mask. Idea is to
             create covered vias. 
	     Color code is  dark grey (recommended). insulated from inners.
   layer 12: contains patterns which go on both external planes, and comp
             inner;insulated from bott inner layer.
	     Color code is magenta4 (recommended)
   layer 13: contains patterns which go on both external planes, and bott
             inner;insulated from comp inner layer.
	     Color code is cyan4 (recommended)

   layer 15: contains patterns which are transferred to all metalized
             planes; pads go to top solder mask. Color code is brown2.
   layer 16: same as layer 15, but pads are not transferred to solder
             side mask. good for covered vias.
	     Color code is brown4.

   layer 20-39: component layer. Pads on layer 21-34 will be 
             transferred to the top solder mask, layers 35-39 will not.
	     color code is white for 20, blue for 21-34 and blue3 for 35-39.
	     layer 20 is the knockout layer for removing copper.
   layer 40-59: inner layer close to component layer.
             Color code is white for 40 and magenta for 41-59.
   layer 60-79: inner layer close to solder layer.
             Color code is white for 60 and cyan for 61-79
   layer 80-99: solder layer. Pads on 81-94 will be transferred to
             solder side solder mask, pads on layer 95-99 will not.
	     Color code is white for 80, green for 81-94, and green3 for 95-99.
	     layer 80 is knockout layer for removing copper.

   layers 100- : may contain arbitrary more inner layers. Suggested
             layer spacing is 20 xfig layers per physical layer. The first
	     layer is always the knockout layer and should be colored white.


   There is a number of selected graphical objects (circles, rectangular
   pads) which get identified as pads, and will be implemented as apertures
   in the gerber file. However, if a particular circular diameter is not found
   in a list of common apertures, it gets converted into the generic gerber draw
   primitive. This may lead to problems with automatic aperture identification.

   For identification of holes, white filled circles in layer 0 are used.

   TARGET FILES:
   target files generated are Gerber files, either in original version or as
   RS274X version, Excellon Drill files, and a tool list. These were the files
   required we used for our initial manufacturer, www.apcircuits.com, but seem
   to be understood by a larger number of manufacturers, like pcmfr.com


 
   INVOCATION:
   xfig2gerber [options] sourcefile
 
   options:
   -h        print help function
   -1        create drill & tool file
   -2        create component copper layer file
   -3        create solder copper layer file
   -4        create inner file close to component
   -5        create inner file close to solder
   -6        create component solder mask
   -7        create solder solder mask
   -8        create silk screen
   -9        create bottom silk screen
   -n num    create files from layers num to num+range-1
   -r range  set range for selective print; defaults to 20. Must appear
             before -n option
   -t        switch on transfer mode from layer 15 for metal planes (default)
   -T        switch off transfer mode from layer 15
   -d        select standard double side file set, as in
             option -123
   -D        same as -d, but with extra solder and silk screen masks. If
             option -j is set, the bottom solder mask is joined with the
	     top solder mask. same as  -123678
   -j        joining option for solder masks
   -J        switch off join mode for solder masks (default)
   -f        select simple four-layer board without solder masks and
             silk screen. corresponds to -12345
   -F        same as -f, but with solder masks and top silk screen
   -s        create top silk screen (same as -8)
   -S        create bottom silk screen (same as -9)
   -a        create a separate aperture definition file
   -o fname  create outfile name not from infilename; use stdout if fname
             is "-"
   -l fname  get layers from file named fname containing the layers
   -X        allow for a knockout layer in the RS274X file format. This is
             necessary for correct isolation pads in inner layers
   -i        special consideration for inner insulation pads. If this option
             is chosen, the inner layers get a different pad for insulation
	     purposes than the connecting layers. This is can be used to
	     fulfill different insulation annulus requirements in inner layers.
	     currently, the minimum insulation spacing in inner layers is
	     set to 16mil. This is a dirty option, but necessary for a local
	     supplier to have high density of pads. 

   MISCELLANEOUS:

   Tool selection is optimized for the toolset of apcircuits, 
   www.apcircuits.com 

   TODO:
   better documentation, implement text rendering, outside contour routing

   HISTORY:

   first version, christian Kurtsiefer 02/09/98 
   added different drill sizes   chk 9/10/98  
   corrected T-code bug in circle interpolation   chk 5.11.99
   more square pads inerted; added D2 CODES AT
    END of filled polygons  chk 14.11.99
   more square pads: 113/114 for electrolytic caps  chk 27.11.02
   more square pads for mini circuits mixer 115/116 chk 01.05.03
   another square pads for minicircuits ROS  117
   more square pads for smd crystals 118/119 chk 10.07.03 
   fifi3 version running (multilayer)                    chk 18.01.04
   corrected tooling file option, inserted pads         chk 21.02.04 
   added LFCSP_VQ pads for analog devices DDS chips     chk 6.5.07
   added larger inner insulation pads for local manuf   chk 11.8.07
   added basic arc functionality - needs filled arcs still chk27.5.08
   removed doubling of hole data, fixed buggy layer 16 assoc 17.10.09chk
*/

#include<stdio.h>
#include<string.h>
#include<time.h>
#include <fcntl.h>
#include <unistd.h>


#define maxaperture 35

/* for consistency with the old program, several graphical diameters can 
   translate in the same tool. Therefore, the supported table has two indices:
   a drill number or virtual diameters, and a tool index (starting at 1) for
   the physical drill. */
#define drill_number 13
#define tool_number 12
typedef struct {float diameter; int graph_units, tool_index; } drill_table;
drill_table drilltab[]={
  {0.028,65,1},                              /* #70, .028" = 0.711mm */
  {0.035,66,2},     /* standard pin size */  /* #65, .035" = 0.889mm */
  {0.042,83,3},                              /* consistency? .042" */
  {0.042,99,3},                              /* #58, .042" = 1.067mm */
  {0.052,123,4},                             /* #55, .052" = 1.321mm */
  {0.0595,142,5},                            /* #53, .0595" = 1.511mm */  
  {0.086,203,6},                             /* #44, .086" = 2.184mm */
  {0.104,246,7},     /* extra cost */        /* #37, .104" = 2.642mm */
  {0.125,295,8},    /* for 4-40 screws */    /* 1/8" = 3.175mm */
  {0.152,359,9},   /* for 6-32 screws */     /* #24, .152" = 3.861mm */
  {0.0145, 31,10},  /* for tiniest vias */   /* #79, .0145" = 0.368mm */
  {0.021, 45,11},   /* for small vias */     /* #75, .021" = 0.533mm */
  {0.177, 392,12},  /* for M4/8-32 screws */ /* #16, 0.177" = 4.496mm */
};

int tool_counts[tool_number+1];    /* counts number of tool usages */


/* here, some special aperture definitions are given */
#define num_round_apert 17 /* round patches identified as apertures */
/* the round apertures have a corresponding aperture index for the
   insulation pattern to be used for non-touched inner layers. This assumes
   a minimum distance of 16 mil between the hole and the next copper. This
   assumes an association of hole diameters and aperture sizes. This is 
   no very clean, but should keep the local supplier happy. */
typedef struct {
    int xfig_rad, aperture_idx; double real_dia;  /* in inches */
    char * description; int knockout_idx; } roundap_table;
roundap_table  rnd_apt_tab[] = 
{{135, 102, 0.060, "standard round pad", 151},
 {125, 103, 0.055, "small round pad", 102},
 {101, 108, 0.045, "small round pad, 45 mil dia", 102},
 {99, 109, 0.044, "small round pad, 44 mil dia", 102},
 {95, 110, 0.042, "small round pad, 42 mil dia", 102},
 {83, 128, 0.037, "via pad, 37 mil dia", 150},
 {70, 129, 0.031, "via pad, 31 mil dia", 149},
 {180, 133, 0.080, "medium round pad, 80 mil dia", 133},
 {225, 134, 0.100, "medium round pad, 100 mil dia", 134},
 {360, 135, 0.160, "large round pad, 160 mil dia", 135},
 {405, 136, 0.180, "large round pad, 180 mil dia", 136},
 {270, 141, 0.120, "medium round pad, 120 mil dia", 141},
 {142, 142, 0.063, "small round pad, 63 mil dia", 142},
 {106, 149, 0.047, "inner insulation pad, 47mil dia", 149},
 {119, 150, 0.053, "inner insulation pad, 53 mil dia", 150},
 {151, 151, 0.067, "inner insulation pad, 67 mil dia", 151},
 {315, 170, 0.140, "BNC plug solder pad, 140 mil dia", 170} 
};

/* same with rectangular apertures. 450 xfig units translate into 100 mil */
#define num_rect_apert 70
typedef struct rectap_table {
    int xfig_x, xfig_y, aperture_idx; double real_x, real_y;
    char* description;} rectap_table;
rectap_table rectap_tab[]=
{{216, 324, 100, 0.072, 0.048, "DIL standard rect pad 72x48 mil"}, /* 1 */
 {324, 216, 101, 0.048, 0.072, "DIL standard rect pad 48x72 mil"},
 {90, 360, 104, 0.080, 0.020, "SOIC pad (50 mil sep) 80x20 mil"}, 
 {360, 90, 105, 0.020, 0.080, "SOIC pad (50 mil sep) 20x80 mil"},
 {120, 225, 106, 0.050, 0.026, "SMD (0805) resistor pads, 50x26 mil"}, /* 5 */
 {225, 120, 107, 0.026, 0.050, "SMD (0805) resistor pads, 26x50 mil"},
 {72, 180, 111, 0.040, 0.016, "TQFP-32 pads, 40x16mil"},
 {180, 72, 112, 0.016, 0.040, "TQFP-32 pads, 16x40mil"},
 {315, 495, 113, 0.110, 0.070, "SMD tantal cap (size D) pad 110x70mil"},
 {495, 315, 114, 0.070, 0.110, "SMD tantal cap (size D) pad 70x110mil"},/*10*/
 {288, 450, 115, 0.100, 0.064, "Minicircuits mixer pad 100x64mil"},
 {450, 288, 116, 0.064, 0.100, "Minicircuits mixer pad 64x100mil"},
 {270, 270, 117, 0.060, 0.060, "Mni Circuits ROS package pad 60x60mil"},
 {270, 360, 118, 0.080, 0.060, "Saronix crystal pad 80x60mil"},
 {360, 270, 119, 0.064, 0.080, "Saronix crystal pad 60x80mil"}, /*15*/
 {135, 270, 120, 0.060, 0.030, "SMD (1210) pads, 60x30mil"},
 {270, 135, 121, 0.030, 0.060, "SMD (1210) pads, 30x60mil"},
 {90, 135, 122, 0.030, 0.020, "SMD (0603) pads, 30x20mil"},
 {135, 90, 123, 0.020, 0.030, "SMD (0603) pads, 20x30mil"},
 {120, 180, 124, 0.040, 0.026, "SOT23-5 pico gate pads 40x26mil"}, /*20*/
 {180, 120, 125, 0.026, 0.040, "SOT23-5 pico gate pads 26x40mil"},
 {315, 54, 126, 0.012, 0.070, "SSOP pad (25 mil sep) 12x70mil"},
 {54, 315, 127, 0.070, 0.012, "SSOP pad (25 mil sep) 70x12mil"},
 {315, 315, 130, 0.070, 0.070, "SMD crystal pad 70x70mil"},
 {90, 150, 131, 0.033, 0.020, "alt SMD (0603) pads, 33x20mil"},  /* 25 */
 {150, 90, 132, 0.020, 0.033, "alt SMD (0603) pads, 20x33mil"},
 {162, 380, 137, 0.084, 0.036, "TO-263 pad, 84x36mil"},
 {380, 162, 138, 0.036, 0.084, "TO-263 pad, 36x84mil"},
 {200, 385, 139, 0.085, 0.044, "alt TO-263 pad, 85x44mil"},
 {385, 200, 140, 0.044, 0.085, "alt TO-263 pad, 44x85mil"}, /* 30 */
 {45, 180, 143, 0.040, 0.010, "TSSOP pad 40x10mil"},
 {180, 45, 144, 0.010, 0.040, "TSSOP pad 10x40mil"},
 {54, 162, 145, 0.036, 0.012, "LFCSP_VQ pad 36x12mil"},
 {162, 54, 146, 0.012, 0.036, "LFCSP_VQ pad 12x36mil"},
 {324, 162, 147, 0.036, 0.072, "minicirc MAR pad 72x36mil"}, /* 35 */
 {162, 324, 148, 0.072, 0.036, "minicirc MAR pad 36x72mil"}, 
 {315, 45, 152, 0.010, 0.070, "LQFP128 pad (19.7 mil sep) 10x70mil"},
 {45, 315, 153, 0.070, 0.010, "LQFP128 pad (19.7 mil sep) 70x10mil"},
 {64, 223, 154, 0.050, 0.014, "TSSOP pad 50x14mil"},
 {223, 64, 155, 0.014, 0.050, "TSSOP pad 14x50mil"}, /* 40 */
 {450, 720, 156, 0.160, 0.100, "SOT-223 ground pad 160x100mil"},
 {720, 450, 157, 0.100, 0.160, "SOT-223 ground pad 100x160mil"},
 {270, 225, 158, 0.050, 0.060, "SMD Varicap pad 50x60mil"},
 {225, 270, 159, 0.060, 0.050, "SMD Varicap pad 60x50mil"},
 {216, 162, 160, 0.036, 0.048, "SOD-123 pad, 36x48mil"}, /* 45 */
 {162, 216, 161, 0.048, 0.036, "SOD-123 pad, 48x36mil"},
 {315, 540, 162, 0.120, 0.070, "D2pak pad, 120x70mil"},
 {540, 315, 163, 0.070, 0.120, "D2pak pad, 70x120mil"},
 {765, 900, 164, 0.200, 0.170, "D2pak back, 200x170mil"},
 {900, 765, 165, 0.170, 0.200, "D2pak back, 170x200mil"}, /* 50 */
 {450, 1350, 166, 0.300, 0.100, "Inductor S size, 300x100mil"},
 {1350, 450, 167, 0.100, 0.300, "Inductor S size, 100x300mil"},
 {495, 2250, 168, 0.500, 0.110, "Inductor XL size, 500x110mil"},
 {2250, 495, 169, 0.110, 0.500, "Inductor XL size, 110x500mil"},
 {72, 360, 171, 0.080, 0.016, "MFQP44 pad, 80x16mil"}, /* 55 */
 {360, 72, 172, 0.016, 0.080, "MFQP44 pad, 16x80mil"},
 {129, 211, 173, 0.047, 0.029, "SOT23-5a pad, 47x29mil"},
 {211, 129, 174, 0.029, 0.047, "SOT23-5a pad, 29x47mil"},
 {180, 450, 175, 0.100, 0.040, "1008 coilcraft pad 100x40mil"},
 {450, 180, 176, 0.040, 0.100, "1008 coilcraft pad 40x100mil"}, /* 60 */
 {248, 196, 177, 0.044, 0.055, "SMD crystal pad 43.5x55.1mil"},
 {196, 248, 178, 0.055, 0.044, "SMD crystal pad 55.1x43.5mil"},
 {64, 225, 179, 0.050, 0.014, "TSSOP pad2 50x14mil"},
 {225, 64, 180, 0.014, 0.050, "TSSOP pad2 14x50mil"},
 {72, 225, 181, 0.050, 0.016, "uusb pad 50x16mil"},
 {225, 72, 182, 0.016, 0.050, "uusb pad 16x50mil"},
 {315, 360, 183, 0.080, 0.070, "uusb pad2 80x70mil"},
 {360, 315, 184, 0.070, 0.080, "uusb pad2 70x80mil"},
 {135, 225, 185, 0.050, 0.030, "AT1532 pad 50x30mil"},
 {225, 135, 186, 0.030, 0.050, "AT1532 pad 30x50mil"},
};

/* predefined layer lists */
static int * readlayerlist[]={
    (int []){-1}, /* empty entry 0 (reserved: manual layer list) */
    (int []){-1}, /* empty entry 1 (reserved: drill file ) */
    (int []){10,11,12,13,15,16,21,22,23,24,25,26,27,28,29,
     30,31,32,33,34,35,36,37,38,39,-1}, /* component copper (2) */
    (int []){10,11,12,13,15,16,81,82,83,84,85,86,87,88,89,
     90,91,92,93,94,95,96,97,98,99,-1}, /* solder copper (3) */
    (int []){12,15,16,41,42,43,44,45,46,47,48,49,
     50,51,52,53,54,55,56,57,58,59,-1}, /* innercomp (4) */
    (int []){13,15,16,61,62,63,64,65,66,67,68,69,
     70,71,72,73,74,75,76,77,78,79,-1}, /* innersold (5) */
    (int []){8,10,12,13,15,21,22,23,24,25,26,27,28,29,
     30,31,32,33,34,-1}, /* comp soldmask */
    (int []){9,10,12,13,15,81,82,83,84,85,86,87,88,89,
     90,91,92,93,94,-1}, /* bottom sold mask */
    (int []){6,-1}, /* top silk */
    (int []){7,-1}, /* bott silk (9) */
    (int []){-1}, /* empty_list (10) (reserved: tool cnt)*/
    (int []) {8,9,10,15,16,21,22,23,24,25,26,27,28,29,
	      30,31,32,33,34,81,82,83,84,85,86,87,88,89,
	      90,91,92,93,94,-1} /* bott&top soldermask (11) */
};
static int * punchlayerlist [] ={
    (int []){-1},(int []){-1}, /* 0, 1 */
    (int []) {20,-1},  /* cutout comp */
    (int []) {80,-1},  /* cutout sold */
    (int []) {10,11,13,40,-1},  /* cutout innercomp */
    (int []) {10,11,12,60,-1},  /* cutout innersold */
    (int []){-1},(int []){-1},   /* 6, 7 solder masks */
    (int []){-1},(int []){-1},   /* 8, 9 silk layers - layer 10? */
    (int []){-1},(int []){-1}   /* 10, 11 */ 
};
/* predefined name lists */
char * suffixlist[]= {
    ".arb.lgx", /* 0 */
    ".holes.drl", /* 1 */
    ".comp.lgx", /* 2 */
    ".bott.lgx",
    ".cmpinner.lgx",
    ".bottinner.lgx", /* 5 */
    ".compsldmask.lgx",
    ".bottsldmask.lgx",
    ".compsilk.lgx",
    ".bottsilk.lgx",
    ".tools.mfg",  /* 10 */
    ".jointsldmask.lgx", /* 11 */
};
char * file_interpretation[]= {
    "SOMELAYER",
    "",
    "COMPONENTSIDE", /* 2 */
    "BOTTOMSIDE", /* 3 */
    "COMP_INNER", /* 4 */
    "BOTTOM_INNER" /* 5 */
    "COMP_SOLDERMASK",
    "BOTT_SOLDERMASK",
    "COMP_LEGEND", /* 8 */
    "BOTT_LEGEND",
    "", /* 10 */
    "JOINT_SOLDERMASK", /* 11 */
};
/* which type is a specific job: 1:drill, 2:gerber, 4:toolcnt, 
                                 5:RS274X firstlayer, 6:RS274X lastlayer */
int filetypetable[]={2,1,2,2,2,2,2,2,2,2,4,2};

#define TARGETNAMELEN 200

/* variable definitions */
FILE *infile;
char fnam[200];   /* file name */
char inbuffer[10000];  /* input read buffer */
char constring[1000];
char *ibb;
char *varp;
char ifn[200]="stdin";
char ofn[200]="stdout";

typedef struct {int class, type, type2, width, pencolor, fillcolor, depth,
	utype1, fillmode, int11, int12, int13, int14, int15,
	int16, cx1, cx2, r1, r2, ax1, ax2, ex1, ex2, mx1, mx2; 
    float float1, float2, fx1, fx2; } obstruct;
obstruct ob;   /* actual object structure */

int whattodo(obstruct *ob, int *layerlist, int filetype);

int mode;   /* Filter mode */
int i;      /* index variable */
int aperture;
int lastaction;   /* action flag: 0 discard, !0: keep */

/* forward declarations */
int ermsg(int ern);
int makedecision(obstruct *ob);
void getpair(int *x, int *y);
void rs_plot(int *x, int *y);
void rs_drill(int *x, int *y);
void drill_header(FILE *f);
void tool_trailer(FILE *f);
void drill_trailer(FILE *f);
void gerber_header(FILE *f);
void gerber_trailer(FILE *f);
void RS274X_header_1(FILE *f, char *imagename);
void RS274X_trailer_1(FILE *f);
void RS274X_header_2(FILE *f, char *imagename);
void RS274X_trailer_2(FILE *f);

void rs_single(int *x);
int get_tool_number(int radius);
/* produce one file collecting layers in layerlist (a list of ints, 
   terminated with -1) of filetype (1: drill file, 2: gerber file, 
   4: tool file) into the target stream. Has now an index for jobtype
   to indicate punch layer corrections if necessary by punchflag !=0
   */
int do_parsing(int * layerlist, int filetype, FILE * target, int punchflag); 


extern char *optarg;
extern int optind, opterr, optopt;

#define MAXOUTFILES 200 /* number of output files */
#define MAXFILNAMLEN 200 /* max file name length */
#define MAXPRINTLAYERS 100 /* max number of layers include in one file */
#define DEFAULTRANGE 20 /* number of layers to collect per default */

int main(int argc, char *argv[]){
    int opt, i, i2, jobtype;
    char sourcename[MAXFILNAMLEN]= "" ;
    char targetname[MAXFILNAMLEN]="";
    char outfileroot[MAXFILNAMLEN]=""; /* if different name root is wanted */
    char layerfilename[MAXFILNAMLEN]=""; /* source layer file */
    int outfilenumber;    /* number of target files to be produced */
    int outfilejob[MAXOUTFILES];  /* type list of target files */
    int layerlist[MAXPRINTLAYERS]; /* manual layer list, terminated with -1 */
    int playerlist[2]; /* punch layer list or manual construction */
    int layerrange = DEFAULTRANGE; /* manual layer list */
    int layerstart=-1; /* manual layer start */
    int transfermode_15 = 1; /* transfer behavior for layer 15 */
    int RS274Xmode = 0;   /* if !=0, RS274X files instead if RS274D files */
    int joinmode = 0;  /* join mode for solder masks */
    int outfilemode = 0;
    int Large_inner_insulation = 0; /* for making special inner pads */

    FILE *target;
    FILE * layerfile; /* for reading in separate layers */

    outfilenumber=0; /* start with no files */

    /* try to interpret options */
    opterr=0; /* be quiet when there are no options */
    while ((opt=getopt(argc, argv, "h123456789n:r:tTdDjJfFsSo:l:Xi")) != EOF) {
	switch (opt) {
	    case 'h': /* print help text */
 		printf("print help text\n");
		return 0;
	    case '1': /* drill and tool file */
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=1;
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=10;
		break;
	    case '2':case '3':case '4':case '5': /* create defined layers */
	    case '6':case '7':case '8':case '9':
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=(opt-'0');
		break;
	    case 'r': /* set layer range to collect */
		sscanf(optarg,"%d",&layerrange);
		if ((layerrange<1) || (layerrange >(MAXPRINTLAYERS-2))) {
		    return -ermsg(12);
		}
		break;
	    case 'n': /* set start layer & setlayers */
		sscanf(optarg,"%d",&layerstart);
		if (layerstart<0) return -ermsg(13);
		for (i=0;i<layerrange;i++) layerlist[i]=layerstart+i;
		if (transfermode_15) layerlist[i++]=15;
		layerlist[i++]=-1; /* terminate list */
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=0;
		break;
	    case 't': /* switch on transfer of layer 15 */
		transfermode_15=1;
		break;
	    case 'T': /* swizch off transfer mode */
		transfermode_15=0;
		break;
	    case 'D': /* create bottom &top mask + top silk */
		if (outfilenumber+2>MAXOUTFILES) return -ermsg(11);
		if (joinmode) {
		    outfilejob[outfilenumber++]=11; /* joint mask */
		} else {
		    outfilejob[outfilenumber++]=6; /* separate masks */
		    outfilejob[outfilenumber++]=7;	
		}
		outfilejob[outfilenumber++]=8; /* silk layer */
	    case 'd': /* create hole, tool, top and bottom layer */
		if (outfilenumber+2>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=1;
		outfilejob[outfilenumber++]=10;
		outfilejob[outfilenumber++]=2;
		outfilejob[outfilenumber++]=3;
		break;
	    case 'j': /* set joinmode for soldermasks */
		joinmode=1;
		break;
	    case 'J': /* reset joinmode for soldermasks */
		joinmode=0;
		break;
	    case 'F': /* four layer board w top&bott solder mask + topsilk */
		if (outfilenumber+2>MAXOUTFILES) return -ermsg(11);
		if (joinmode) {  
		    outfilejob[outfilenumber++]=11;
		} else {
		    outfilejob[outfilenumber++]=6;
		    outfilejob[outfilenumber++]=7;
		}
		outfilejob[outfilenumber++]=8;
	    case 'f': /* simple four-layer board, like -12345 */
		if (outfilenumber+5>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=1;
		outfilejob[outfilenumber++]=10;
		outfilejob[outfilenumber++]=2;
		outfilejob[outfilenumber++]=3;
		outfilejob[outfilenumber++]=4;
		outfilejob[outfilenumber++]=5;
		break;
	    case 's':
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=8;
		break;
	    case 'S':
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=9;
		break;
	    case 'o': /* use separate outfile name */
		outfilemode=1; /* special outfilemode */
		strncpy(outfileroot,optarg,MAXFILNAMLEN-1);
		outfileroot[MAXFILNAMLEN-1]=0; /* safe term */
		break;
	    case 'l': /* read layer list from file */
		strncpy(layerfilename,optarg,MAXFILNAMLEN);
		layerfilename[MAXFILNAMLEN-1]=0;
		layerfile=fopen(layerfilename,"r");
		if (!layerfile)  return -ermsg(14);
		for (i=0;i<MAXPRINTLAYERS-1;i++) {
		    if (1!=fscanf(layerfile,"%d",&layerlist[i])) break;
		    if (layerlist[i]<0) return -ermsg(15);
		}
		layerlist[i]=-1; /* term list */
		playerlist[0]=layerlist[0];playerlist[1]=-1;
		fclose(layerfile);
		
		if (outfilenumber>MAXOUTFILES) return -ermsg(11);
		outfilejob[outfilenumber++]=0; /* arb list */
		break;
	    case 'X':
		RS274Xmode = 1; /* allow for knockout layers an X format */
		break;
	    case 'i':
		Large_inner_insulation = 1;
		break;
	    default:
		break;
	}
    }

    if (optind==argc-1) { /* try to read source name */
	strncpy(sourcename,argv[optind],MAXFILNAMLEN);
	sourcename[MAXFILNAMLEN-1]=0;
    };

    /* do the real work */
    /* printf("outfiles: %d\n",outfilenumber); */

    for (i=0;i<outfilenumber;i++) {
	jobtype=outfilejob[i];
	/* open one particular output file */
	strncpy(targetname,outfilemode?outfileroot:sourcename,MAXFILNAMLEN);
	targetname[MAXFILNAMLEN-1]=0;
	if (strncmp(targetname,"-",1)) {
	    strncat(targetname,suffixlist[jobtype],MAXFILNAMLEN);
	    targetname[MAXFILNAMLEN-1]=0;
	    if (!(target=fopen(targetname,"w"))) return -ermsg(4);
	} else {
	    target=stdout;
	}
	/* open infile */
	if strncmp(sourcename,"-",1) {
	    if (!(infile=fopen(sourcename,"r"))) return -ermsg(3);
	} else {
	    infile=stdin;
	}


	/* new headers */
  
	/* create destination header */
	switch(filetypetable[jobtype]){
	    case 1: /* drill file */
		drill_header(target);
		/* reset drill count */
	    case 4: 
		for (i2=0;i2<tool_number;i2++) tool_counts[i2]=0;
		break;
	    case 2: /* gerber file */
		if (RS274Xmode) {
		    RS274X_header_1(target, file_interpretation[jobtype]);
		} else {
		    gerber_header(target);
		}
		break;
	    default:
		return -1; /* wrong file type */
	};

	/* printf("jobtype: %d\n",jobtype); */
	if (fseek(infile,0L,SEEK_SET)) return -ermsg(16); /* rewind */	

	do_parsing((jobtype?readlayerlist[jobtype]:&layerlist[1]),
		   filetypetable[jobtype], target, 0);
	/* close text files for this round */
	if (RS274Xmode && (filetypetable[jobtype]==2)) { 
            /* for X files, go for second round */
	    RS274X_trailer_1(target); /* end layer 1*/
	    if (fseek(infile,0L,SEEK_SET)) return -ermsg(16); /* rewind */
	    RS274X_header_2(target, file_interpretation[jobtype]); /* layer2 */
	    /* go for second run */
	    do_parsing((jobtype?punchlayerlist[jobtype]:playerlist),
		       filetypetable[jobtype], target,
		       Large_inner_insulation?1:0);
	}

	/* create destination file trailers */
	switch(filetypetable[jobtype]){
	    case 4: /* drill count file */
		tool_trailer(target);
		break;
	    case 2: /* gerber file trailer */
		if (RS274Xmode) {
		    RS274X_trailer_2(target); /* end layer 2 */
		} else {
		    gerber_trailer(target);
		}
		break;
	    case 1: /* drill file trailer - add an M30 */
		drill_trailer(target);
		break;
	};
  
	if (strncmp(sourcename,"-",1)) fclose(infile);
	fclose(target);
	/* printf("bla; i: %d\n",i); */
    }
    /* all files are produced. */
    return 0;
}

/* do parsing; params: *layerlist contains layers to be considered and is
   a list terminated with -1, filetype is 1: drill, 2: gerber, 4: toolcnt;
   target is the (open) target file handle. This routine does not distinguish
   between a 274D file and the different 274X layers. punchflag to indicate
   possible special treatment for punch layer */
int do_parsing(int *layerlist, int filetype, FILE * target, int punchflag) {
  int k,x,y,xmin,xmax,ymin,ymax,padnum;
  int difx,dify;
  int actual_drill=-1; /* no tool selected */
  int apindex;
  int target_aperture; /* for dealing with special requirements in inner
			  layers for insulation */

  /* reading of header of source file */
  if (fgets(inbuffer,10000,infile)==NULL) return ermsg(6);
  /* changed strcmp to strncmp to be compatible with 3.2.5 plus comment */
  if (strncmp(inbuffer,"#FIG 3.2",8)!=0) {printf(">%s<",inbuffer);return ermsg(7);};
  
  /* read rest of header */
  for (i=0;i<8;i++){
   if (fgets(inbuffer,10000,infile)==NULL) return ermsg(6);
  };  

  /* main conversion loop */
  lastaction=0;
  while (feof(infile)==0){
    if (fgets(inbuffer,10000,infile)==NULL) {
      if (feof(infile)) break;
      return ermsg(6);
    };
    /* printf("read:%s",inbuffer); */
    if (inbuffer[0]=='\n') {fprintf(target,"\n");continue;};
    if ((inbuffer[0]==' ')||(inbuffer[0]=='\t')){  /* continuation line ? */
      if (lastaction!=0) { /* copy if interesting command */
	i=i;
      };
    } else {
      /* get object class */
      sscanf(inbuffer,"%d",&ob.class);
      ibb=inbuffer;
      switch (ob.class){
      case 1:
	/* circles */
	sscanf(strtok(ibb," "),"%d",&i);
	sscanf(strtok(NULL," "),"%d",&ob.type);
	sscanf(strtok(NULL," "),"%d",&ob.type2);
	sscanf(strtok(NULL," "),"%d",&ob.width);
	sscanf(strtok(NULL," "),"%d",&ob.pencolor);
	sscanf(strtok(NULL," "),"%d",&ob.fillcolor);
	sscanf(strtok(NULL," "),"%d",&ob.depth);
	sscanf(strtok(NULL," "),"%d",&ob.utype1);
	sscanf(strtok(NULL," "),"%d",&ob.fillmode);
	sscanf(strtok(NULL," "),"%f",&ob.float1);
	sscanf(strtok(NULL," "),"%d",&ob.int11);
	sscanf(strtok(NULL," "),"%f",&ob.float2);
	sscanf(strtok(NULL," "),"%d",&ob.cx1);
	sscanf(strtok(NULL," "),"%d",&ob.cx2);
	sscanf(strtok(NULL," "),"%d",&ob.r1);
	sscanf(strtok(NULL," "),"%d",&ob.r2);
	ibb=strtok(NULL,"\n");   /* get rest... */
	break;
	
      case 2:
	/* lines */
	sscanf(strtok(ibb," "),"%d",&i);
	sscanf(strtok(NULL," "),"%d",&ob.type);
	sscanf(strtok(NULL," "),"%d",&ob.type2);
	sscanf(strtok(NULL," "),"%d",&ob.width);
	sscanf(strtok(NULL," "),"%d",&ob.pencolor);
	sscanf(strtok(NULL," "),"%d",&ob.fillcolor);
	sscanf(strtok(NULL," "),"%d",&ob.depth);
	sscanf(strtok(NULL," "),"%d",&ob.utype1);
	sscanf(strtok(NULL," "),"%d",&ob.fillmode);
	sscanf(strtok(NULL," "),"%f",&ob.float1);
	sscanf(strtok(NULL," "),"%d",&ob.int11);
	sscanf(strtok(NULL," "),"%d",&ob.int12);
	sscanf(strtok(NULL," "),"%d",&ob.int13);
	sscanf(strtok(NULL," "),"%d",&ob.int14);
	sscanf(strtok(NULL," "),"%d",&ob.int15);
	sscanf(strtok(NULL," "),"%d",&ob.int16);
	/* ibb=strtok(NULL,"\n"); */  /* get rest... */
	break;

      case 5:
	/* arcs */
	sscanf(strtok(ibb," "),"%d",&i);
	sscanf(strtok(NULL," "),"%d",&ob.type);
	sscanf(strtok(NULL," "),"%d",&ob.type2);
	sscanf(strtok(NULL," "),"%d",&ob.width);
	sscanf(strtok(NULL," "),"%d",&ob.pencolor);
	sscanf(strtok(NULL," "),"%d",&ob.fillcolor);
	sscanf(strtok(NULL," "),"%d",&ob.depth);
	sscanf(strtok(NULL," "),"%d",&ob.utype1);
	sscanf(strtok(NULL," "),"%d",&ob.fillmode);
	sscanf(strtok(NULL," "),"%f",&ob.float1);
	sscanf(strtok(NULL," "),"%d",&ob.int11);
	sscanf(strtok(NULL," "),"%d",&ob.int12);
	sscanf(strtok(NULL," "),"%d",&ob.int13);
	sscanf(strtok(NULL," "),"%d",&ob.int14);
	sscanf(strtok(NULL," "),"%f",&ob.fx1);
	sscanf(strtok(NULL," "),"%f",&ob.fx2);
	sscanf(strtok(NULL," "),"%d",&ob.ax1);
	sscanf(strtok(NULL," "),"%d",&ob.ax2);
	sscanf(strtok(NULL," "),"%d",&ob.mx1);
	sscanf(strtok(NULL," "),"%d",&ob.mx2);
	sscanf(strtok(NULL," "),"%d",&ob.ex1);
	sscanf(strtok(NULL," "),"%d",&ob.ex2);
	/* ibb=strtok(NULL,"\n"); */  /* get rest... */
	/* convert center position into int */
	ob.cx1=(int)ob.fx1; ob.cx2=(int)ob.fx2;
	break;

      default: /* ignore unknown objects */
	lastaction=1;
      };
      
      /* do interpretation */
      i=whattodo(&ob, layerlist, filetype);
      switch(i){ /* aperture selection */
	  case 2: case 3: case 4: case 5: case 7:
	aperture=ob.width+20;
	if (aperture>maxaperture+20) aperture=maxaperture+20;
	if (aperture<20) aperture=20;
	fprintf(target,"G54D%02d*\n",aperture);
	break;
      default:
	  break;
      };
      switch(i){
      case 0: /* skip command */
	break;
      case 1: /* output drill coordinates or count tools */
	  if (filetype==1 ) { /* drill file */
	      rs_drill(&ob.cx1,&ob.cx2);
	      /* make drill selection */
	      if (actual_drill!=get_tool_number(ob.r1)) {
		  actual_drill=get_tool_number(ob.r1);
		  fprintf(target,"T%01dC%05.3f\n",
			  drilltab[actual_drill].tool_index,
			  drilltab[actual_drill].diameter);
	      };
	      fprintf(target,"X%06dY%06d\n",ob.cx1,ob.cx2);
	      break;
	  }
	  /* make tool count */
	  actual_drill=get_tool_number(ob.r1);
	  if (actual_drill>=0 && actual_drill<drill_number) 
	      tool_counts[drilltab[actual_drill].tool_index]++;
	  break;
      case 2: /* generate lines */
	varp=NULL;
	k=ob.int16; /* point count */
	getpair(&x,&y);
	rs_plot(&x,&y);
	fprintf(target,"G01X%05dY%05dD02*",x,y); /* first coordinates */
	if (k==1) {
	  fprintf(target,"D03*D02*\n");
	} else {
	  while (k>1) {
	    getpair(&x,&y);
	    rs_plot(&x,&y);
	    fprintf(target,"X%05dY%05dD01*",x,y);
	    k--;
	  };
	  fprintf(target,"\n");
	};
	break;
      case 3: /* generate polygon */
	varp=NULL;
	k=ob.int16; /* point count */
	getpair(&x,&y);
	rs_plot(&x,&y);
	fprintf(target,"G36*G01X%05dY%05dD02*",x,y); /* first coordinates */
	if (k==1) {
	  fprintf(target,"D03*D02*G37*\n");
	} else {
	  while (k>1) {
	    getpair(&x,&y);
	    rs_plot(&x,&y);
	    fprintf(target,"X%05dY%05dD01*",x,y);
	    k--;
	  };
	  fprintf(target,"D02*G37*\n");
	};      
	break;
      case 4: /* generate circle */
	rs_plot(&ob.cx1,&ob.cx2);	
	rs_single(&ob.r1);
	fprintf(target,
		"G75*G01*X%05dY%05dD02*G03X%05dY%05dI%06dJ%05dD01*G01*\n",
		ob.cx1+ob.r1,ob.cx2,ob.cx1+ob.r1,ob.cx2,-ob.r1,0);
	break;
      case 5: /* generate filled circle - and scan for pads */
	rs_plot(&ob.cx1,&ob.cx2);
        /* scan for pads */
	for (apindex=0;apindex<num_round_apert;apindex++) {
	    if (ob.r1==(rnd_apt_tab[apindex].xfig_rad)) {
		/* make special considerations for known round apertures to
		   have corrected separations in inner layers */
		target_aperture=punchflag?
		    rnd_apt_tab[apindex].knockout_idx:
		    rnd_apt_tab[apindex].aperture_idx;
		fprintf(target, "G54D%03d*G01*X%05dY%05dD02*D03*\n",
			target_aperture,ob.cx1,ob.cx2);
		break;
	    }
	}
	if (apindex<num_round_apert) break;

	/* do it manually if no pad was found */
	rs_single(&ob.r1);
	fprintf(target,
		"G36*G75*G01*X%05dY%05dD02*G03X%05dY%05dI%06dJ%05dD01*G01*D02*G37*\n",
		ob.cx1+ob.r1,ob.cx2,ob.cx1+ob.r1,ob.cx2,-ob.r1,0);
	break;

      case 7: /* generate open arcs */
	  /* store cw/ccw decision in int15 */
	  ob.int15=
	      (ob.mx1-ob.ax1)*(ob.ex2-ob.mx2)-(ob.mx2-ob.ax2)*(ob.ex1-ob.mx1);
	  /*  convert coordinates */
	  rs_plot(&ob.ax1,&ob.ax2);
	  rs_plot(&ob.ex1,&ob.ex2);
	  rs_plot(&ob.cx1,&ob.cx2);
	  
	  /* execute stroke */
	  fprintf(target,
		"G75*G01*X%05dY%05dD02*%sX%05dY%05dI%05dJ%05dD01*G01*D02*\n",
		  ob.ax1,ob.ax2, /* start coordinates */
		  (ob.int15>0)?"G02":"G03", /* which turn */
		  ob.ex1, ob.ex2, /* end coordinates */
		  ob.cx1-ob.ax1, ob.cx2-ob.ax2 /* center offset */);
	break;

      case 6: /* generate square pad */
	getpair(&xmin,&ymin);
	xmax=xmin;ymax=ymin;
	for (k=1;k<5;k++) {
	  getpair(&x,&y);
	  if (x>xmax) xmax=x;if (x<xmin) xmin=x;
	  if (y>ymax) ymax=y;if (y<ymin) ymin=y;
	};
	x=(xmax+xmin)/2;y=(ymax+ymin)/2;rs_plot(&x,&y);
	padnum=0;
	difx=xmax-xmin;dify=ymax-ymin;

	/* try to find a matching rectangular aperture in list */
	for (apindex=0;apindex<num_rect_apert;apindex++) {
	    if ((difx==rectap_tab[apindex].xfig_x) &&
		(dify==rectap_tab[apindex].xfig_y)) {
		padnum=rectap_tab[apindex].aperture_idx;
		break;
	    }
	}

	if (padnum==0) { /* do it by hand...*/
	  /* just a standard filled square */
	  difx/=2; dify/=2; rs_plot(&difx, &dify); /* rescale differences */
	  /*    create aperture selection */
	  aperture=ob.width+20;
	  if (aperture>maxaperture+20) aperture=maxaperture+20;
	  if (aperture<20) aperture=20;
	  fprintf(target,"G54D%02d*\n",aperture);
	  /* create filled polygon */
	  fprintf(target,"G36*G01X%05dY%05dD02*",x-difx,y-dify); /* start */
	  fprintf(target,"X%05dY%05dD01*",x+difx,y-dify);
	  fprintf(target,"X%05dY%05dD01*",x+difx,y+dify);
	  fprintf(target,"X%05dY%05dD01*",x-difx,y+dify);
	  fprintf(target,"X%05dY%05dD01*",x-difx,y-dify);
	  fprintf(target,"X%05dY%05dD02*G37*\n",x-difx,y-dify);
	  
	  /* fprintf(stderr, "%d, %d, %d, %d\n",xmin,xmax,ymin,ymax);
	     fprintf(stderr,"Cannot interpret black box.\n");exit(-1); */
	} else { /* ...or use the found aperture */
	    fprintf(target,"G54D%03d*G01*X%05dY%05dD02*D03*\n",padnum,x,y);
	}
	break; 
      };
  
    };
  };
  

  return 0;
}

char *emsg[]={"No error.",   /* 0 */
	      "Wrong filter mode.",
	      "Error converting filename.",
	      "Error opening input file.",
	      "Error opening output file.",
	      "Wrong number of command line args.\n Usage: filterfig [sourcefile [ destfile ]] option",   /* 5 */
	      "Error reading from input file.",
	      "Not a xfig version 3.2 file.",
	      "Error writing to output ile.",
	      "Wrong job type (internal err)",
	      "Wrong ourfile mode (internal err)", /* 10 */
	      "Too many output files",
	      "Too many or not enough layers to be collected.",
	      "Start layer is negative.",
	      "Cannot open layer file",
	      "read in layer is negative", /* 15 */
	      "Cannot create layered RS274X file because rewind failed",
};

int ermsg(int ern){
  fprintf(stderr,"%s\n",emsg[ern]);  
  return -ern;
}

/* getpair function: give back a pair of values from input file */
void getpair(int *x, int *y){
  if (varp==NULL) {
    fgets(constring,1000,infile);
    varp=strtok(constring," \t");
  } else {
    if ((varp=strtok(NULL," \t"))==NULL) {
      fgets(constring,1000,infile);
      varp=strtok(constring," \t");
    };
  };
  sscanf(varp,"%d",x);
  varp=strtok(NULL," \t");sscanf(varp,"%d",y);
}
/* rescaling function for plot coordinates; assumes 1cm(xfig)=100 mils */
void rs_plot(int *x, int *y){
  int a,b;
  a=(2 * (*x))/9;b=(2 * (*y))/9;
  *x=b;*y=a;
}
/* rescaling function for drill coordinates; assumes 1cm(xfig)=100 mils */
void rs_drill(int *x, int *y){
  int a,b;
  a=(20 * (*x))/9;b=(20 * (*y))/9;
  *x=b;*y=a;
}
void rs_single(int *x){
  int a;
  a=(2 * (*x))/9;
  *x=a;
}
int get_tool_number(int radius) {
  int i=drill_number;
  for (i=0;i<drill_number;i++) {
    if (drilltab[i].graph_units==radius) break;
  };
  if (i==drill_number) i=0;  /* default drill tool */
  return i;
}

/* what to do with a specific graphical object? possible results:
   0: skip entry; 1: output drill coordinate; 2: generate line; 
   3: generate polygon; 4: generate circle; 5: filled circle;
   6: filled square pad; 7: open arc
   */
int whattodo(obstruct *ob, int *layerlist, int filetype){
  int val=0;
  int i;

  switch (filetype) {
      case 1: case 4:/* drill/tool file */
	  if ((ob->class==1)&&(ob->type==3)&&(ob->type2==0)&&(ob->depth==0)&&
	      (ob->fillcolor==7)&&(ob->width==0)){ /* this is a hole... */
	      val=1;
	  };
	  break;
      case 2: /* gerber file */
	  /* check correct layer */
	  for (i=0;layerlist[i]>=0;i++) 
	      if (layerlist[i]==ob->depth) break;
	  if (layerlist[i]<0) break; /* not right layer */
	  /* check for arcs */
	  if ((ob->class==5)&&(ob->type==1)) {/* no check for filling yet */
	      val=7;break; /* open arc */
	  }
	  /* check for lines, polygons, circles, filled circles */
	  if (((ob->class==2)&&(ob->type>0)&&(ob->type<4))||
	      ((ob->class==1)&&(ob->type==3))) {
	      val=((ob->class==2)?2:4)+((ob->fillmode==20)?1:0);
	      /* special treatment for possible square pads */
	      if ((ob->class==2)&&(ob->type==2)&&
		  (ob->pencolor==ob->fillcolor)&&
		  (ob->fillmode==20)) val=6;
	  };
	  break;

      default: /* dont know... */
	  val=0;
  };
  return val;
    
}
/* generate header files */
void drill_header(FILE *f){
  time_t ti;
  ti=time(NULL);
  fprintf(f,"\n\n");
  fprintf(f,"/%%********************************************************\n");
  fprintf(f,"/%%\n/%%\n");
  fprintf(f,"/%%   Program: xfig2gerber, (c) 1998-2009 Christian Kurtsiefer\n");
  fprintf(f,"/%%   Date          : %s",ctime(&ti));
  fprintf(f,"/%%   Source file   : %s \n",ifn);
  fprintf(f,"/%%   Dest file     : %s \n",ofn);
  fprintf(f,"/%%   Format        : Drill file \n");
  fprintf(f,"/%%\n/%%\n");
  fprintf(f,"/%%********************************************************\n");
  fprintf(f,"\n\n");

  fprintf(f,"/DBGRID 1\n/DBUNIT 8\n"); /* is that necessary ?? */
  fprintf(f,"M72\n");

}
void tool_trailer(FILE *f){
  int i2,j;
  /* output drill file */
  /* printf("hit tool trailer prog\n"); */
  fprintf(f,"TOOL\tCOUNT\tSIZE\n");
  for (i2=1;i2<tool_number+1;i2++) {
    if (tool_counts[i2]>0) {
      for (j=0;j<drill_number;j++) if (drilltab[j].tool_index==i2) break;
      if (j==0 || j>=drill_number) j=0;
      fprintf(f,"%d\t%d\t%06.4f\n",i2,tool_counts[i2],drilltab[j].diameter);
      /* printf("i2=%d, tc: %d\n",i2,tool_counts[i2]); */
    };
  };
}
void drill_trailer(FILE *f){
      fprintf(f,"M30\n");
}
void aperture_header(FILE *f){
  int i;
  /* aperture macro definitions */
  /* Aperture size(outer diameter) = 3.333 mils/line thickness units */
  fprintf(f,"G04 Aperture definition for polygons or lines *\n");
  for (i=0;i<=maxaperture;i++)
    fprintf(f,"%%ADD%2dC,%#8.6f*%%\n",i+20,(i==0?0.001:(i==2?.008:i*0.003333)));
  /* special aperture definition for round pads */
  fprintf(f,"G04 Aperture definitions for round pads *\n");
  for (i=0;i<num_round_apert;i++) {
      fprintf(f,"%%ADD%3dC,%05.3f*%%\n", rnd_apt_tab[i].aperture_idx,
	      rnd_apt_tab[i].real_dia);
  }
  /* special aperture definition for square pads */
  fprintf(f,"G04 Aperture definitions for square pads *\n");
  for (i=0;i<num_rect_apert;i++) {
      fprintf(f,"%%ADD%03dR,%05.3fX%05.3f*%%\n",
	      rectap_tab[i].aperture_idx,
	      rectap_tab[i].real_x, rectap_tab[i].real_y);
  }

}

void gerber_header(FILE *f){
  fprintf(f,"%%FSLAX23Y23*%%\n"); /* format definition */
  fprintf(f,"%%MOIN*%%\n"); /* inch as base unit */
  aperture_header(f);  /* define all the apertures */
}
void gerber_trailer(FILE *f){
  fprintf(f,"D02*M02*\n");
}

void RS274X_header_1(FILE *f, char *imagename){ /* layer 1 of 274X file */
  fprintf(f,"%%FSLAX23Y23*%%\n"); /* format definition */
  fprintf(f,"%%MOIN*%%\n"); /* inch as base unit */
  fprintf(f,"%%IN%s*%%\n",imagename); /* name of file */
  aperture_header(f);  /* define all the apertures */
  fprintf(f,"%%LN%s1*%%\n%%LPD*%%\n",imagename); /* first (dark) layer */
}
void RS274X_trailer_1(FILE *f){
  fprintf(f,"D02*\n");
}

void RS274X_header_2(FILE *f, char *imagename){ /* layer 2 of 274X file */
  fprintf(f,"%%LN%s2*%%\n%%LPC*%%\n",imagename); /* first (dark) layer */
}
void RS274X_trailer_2(FILE *f){
  fprintf(f,"D02*M02*\n");
}


