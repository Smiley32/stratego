#ifndef SPIECE_H
#define SPIECE_H

class Personnage
{
  public:

    /*
     *  Getter pour l'attribut value
     *  @return int: value
     */
    int getValue();

    /*
     *  Getter pour l'attribut posX
     *  @return int: posX
     */
    int getPosX();

    /*
     *  Getter pour l'attribut posY
     *  @return int: posY
     */
    int getPosY();

    /*
     *  Setter pour l'attribut value
     *  @param int v: la valeur à mettre dans value
     */
    void setValue(int v);

    /*
     *  Setter pour l'attribut posX
     *  @param int x: la valeur à mettre dans posX
     */
    void setPosX(int x);

    /*
     *  Setter pour l'attribut posY
     *  @param int y: la valeur à mettre dans posY
     */
    void setPosY(int y);

    /*
     *  Méthode qui contrôle que le mouvement d'une pièce est correct
     *  @param int x: abcisse cible
     *  @param int y: ordonnée cible
     *  @return bool: mouvement correct ou non
     */
    bool checkMovement(int x, int y)

  private:

    int value; // La valeur de la pièce (de 0 à 10)
    int posX; // Son abcisse sur le plateau
    int posY; // Son ordonnée sur le plateau
};
#endif
