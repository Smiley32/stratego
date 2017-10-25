#ifndef SGRID_H
#define SGRID_H

#include "s_piece.h"

class s_grid
{

  public:

    bool checkMovement(s_piece target, int x, int y);

  private:

    /*
     *  Ici les équipes sont gérées par négatif.
     *  Il y a une équipe en positif, et l'autre en
     *  négatif
     */
    s_piece *grid[10][10];
};

#endif
