#include "taipei.h"

extern unsigned char grid[GRID_SX][GRID_SY][GRID_SZ];

extern int layer;
extern char *filename;

void load(int complain);
void invalidate_tile(int x, int y, int z);

#define gx2x(x,z) ((x)*GRID_DX+(z)*GRID_DZ+MARGIN)
#define gy2y(y,z) ((y)*GRID_DY+(GRID_SZ-(z))*GRID_DZ+MARGIN)
#define x2gx(x,z) (((x)-(z)*GRID_DZ-MARGIN)/GRID_DX)
#define y2gy(y,z) (((y)-(GRID_SZ-(z))*GRID_DZ-MARGIN)/GRID_DY)
