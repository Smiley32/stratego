# stratego

Cursus : Licence 3 Cursus Master en Ingenierie Informatique. 

Intitulé du projet : Implémentation d'un statego en réseau
Librairies : GameDev Framework - BoostASIO

Principe du jeu Stratego : 
Stratego est un jeu de société de stratégie et de bluff, créé en 1947 (dérivé du jeu L'Attaque, breveté par Hermance Edan en 1909).

Le Stratego se joue à 2 joueurs (un joueur avec les pièces rouges, l'autre avec les pièces bleues) sur un plateau carré de 92 cases (10 cases de côté moins 2 lacs carrés de 4 cases chacun). Chaque joueur possède 40 pièces.

Les pièces représentent des unités militaires et ont deux faces. Une face ne peut être vue que par un seul joueur à la fois, l'autre ne voyant que la couleur de la pièce. Les pièces sont placées de telle façon que le joueur ne voit que le rang de ses propres pièces.

Au début de la partie chaque joueur dispose ses pièces comme il l'entend sur ses quatre premières rangées. Cette pré-phase du jeu est stratégique et déterminante pour la suite de la partie.

Chaque joueur déplace une pièce d'une case par tour : à gauche, à droite, en avant ou en arrière (pas en diagonale). Une attaque se produit quand le joueur déplace sa pièce sur une case déjà occupée par l'adversaire. Chaque joueur montre alors sa pièce à l'adversaire. La pièce la plus forte reste en jeu, l'autre est éliminée ; en cas d'égalité, les deux sont éliminées.


Stratego

L'édition de 2006 par Jumbo
Voici les pièces classées de la plus forte à la plus faible (la force entre parenthèses) :

le Maréchal (10), 1 par joueur
le Général (9), 1 par joueur
les Colonels (8), 2 par joueur
les Commandants (7), 3 par joueur
les Capitaines (6), 4 par joueur
les Lieutenants (5), 4 par joueur
les Sergents (4), 4 par joueur
les Démineurs (3), 5 par joueur
les Éclaireurs (2), 8 par joueur
l'Espion (1), 1 par joueur
le Drapeau (0), 1 par joueur
À ces pièces s'ajoutent les Bombes (6 par joueur). Ni les Bombes ni le Drapeau ne se déplacent.

Le but du jeu est de capturer le Drapeau de l'adversaire ou d'éliminer assez de pièces adverses afin que l'adversaire ne puisse plus faire de déplacements.

Certaines pièces obéissent à des règles spéciales :

Si l'Espion, grade le plus faible, attaque le Maréchal, grade le plus élevé, l'Espion gagne (si le Maréchal attaque en premier, le Maréchal gagne);
Toute pièce attaquant une Bombe est éliminée, sauf le Démineur qui prend alors la place de la Bombe (si une pièce autre qu'un Démineur attaque une Bombe, cette pièce est éliminée, et la Bombe reste en place jusqu'à l'éventuelle attaque d'un Démineur);
L'Éclaireur peut se déplacer d'autant de cases libres qu'il le souhaite, en ligne droite.


-- ENGLISH --

Bachelor Degree third year 

Implementation of a Stratego Game 
with : Game Dev Framework and Boost ASIO

Players alternate moving; red moves first. Each player moves one piece per turn. A player must move a piece in his turn; there is no "pass" (like that in the game of go).

Two zones in the middle of the board, each 2×2, cannot be entered by either player's pieces at any time. They are shown as lakes on the battlefield and serve as choke points to make frontal assaults less direct.

The game can be won by capturing the opponent's Flag, or all of his moveable pieces. Note that it is possible to have ranked pieces that are not moveable because they are trapped behind bombs. There are several ways in which a draw can effectively result (though the rules do not indicate that a draw is a possible outcome):

each Flag is surrounded by bombs and neither player has a miner, and there are insufficient number of moveable pieces or of insufficient rank for either player to corner and capture all of the opponent's moveable pieces;
an opponent's Flag is exposed, but the player has only one moveable piece, the opponent has only one moveable piece but of a higher rank and it guards the Flag, so that it is impossible for the player to either capture the Flag or the higher ranked piece;
there are a few "indian stand-off" situations, where one or more pairs of pieces shuffle around endlessly in order to maintain some advantage or prevent some advantage by the opponent, and no progress can be made;
It is possible for both players to have one mobile piece remaining, and they are of equal rank. If one piece strikes the other, both are removed, leaving no mobile pieces on the board. This may be defined as a draw.
The average game has 381 moves. The number of legal positions is 10115. The number of possible games is 10535.Stratego has many more moves and substantially greater complexity than other familiar games like Chess, Go and Backgammon. However unlike those games where a single bad move at any point may result in loss of the game, most moves in Stratego are inconsequential.

Rules of movement
All movable pieces, with the exception of the Scout, may move only one step to any adjacent space vertically or horizontally (but not diagonally). A piece may not move onto a space occupied by a like color piece. Bomb and Flag pieces are not moveable. The Scout may move any number of spaces in a straight line (such as the rook in chess). In the older versions of Stratego the Scout could not move and strike in the same turn; in newer versions this was allowed. Even before that, sanctioned play usually amended the original Scout movement to allow moving and striking in the same turn because it facilitates gameplay. No piece can move back and forth between the same two spaces for more than three consecutive turns (two square rule), nor can a piece endlessly chase a piece it has no hope of capturing (more square rule).

When the player wants to attack, they move their piece onto a square occupied by an opposing piece. Both players then reveal their piece's rank; the weaker piece (see exceptions below) is removed from the board. If the engaging pieces are of equal rank, both are removed. A piece may not move onto a square already occupied unless it attacks. Two pieces have special attack powers. One special piece is the Bomb which only Miners can defuse. It immediately eliminates any other piece striking it, without itself being destroyed. Each player also has one Spy, which succeeds only if it attacks the Marshal, or the Flag. If the Spy attacks any other piece, or is attacked by any piece (including the Marshal), the Spy is defeated. The original rules contained a provision that following a strike, the winning piece immediately occupies the space vacated by the losing piece. This makes sense when the winning piece belongs to the player on move, but no sense when the winning piece belongs to the player not on move. The latter part of the rule has been quietly ignored in most play.
