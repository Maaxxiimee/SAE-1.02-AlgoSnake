#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <math.h>


// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80	
#define HAUTEUR_PLATEAU 40	//  
// position initiale de la tête du serpent
#define X_INITIAL 40
#define Y_INITIAL 20
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 50000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE 'O'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères et paramètres pour les éléments du plateau et les pavés
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'
#define NB_PAVE 6
#define TAILLE_PAVE 5

// définition d'un type pour le plateau : tPlateau
// Attention, pour que les indices du tableau 2D (qui commencent à 0) coincident
// avec les coordonées à l'écran (qui commencent à 1), on ajoute 1 aux dimensions
// et on neutralise la ligne 0 et la colonne 0 du tableau 2D (elles ne sont jamais
// utilisées)
typedef char tPlateau[LARGEUR_PLATEAU+1][HAUTEUR_PLATEAU+1];

int lesPommesX[NB_POMMES] = {75, 75, 78, 2, 8, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = { 8, 39, 2, 2, 5, 39, 33, 38, 35, 2};

int lesPavesX[NB_PAVE] = { 3, 74, 3, 74, 38, 38};
int lesPavesY[NB_PAVE] = { 3, 3, 34, 34, 21, 15};

void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau, int nbPommes);
void afficher(int, int, char);
void effacer(int x, int y);
void dessinerSerpent(int lesX[], int lesY[]);
char* checkMeilleurChemin(int lesX[], int lesY[], int nbPommes);
void choisirDirection(tPlateau plateau,int lesX[], int lesY[], char * direction, int nbPommes, char chemin[20], int destX, int destY);
void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme, int nbPommes, char chemin[20]);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();

int main(){

    int lesX[TAILLE];
	int lesY[TAILLE];
	int nbProgression = 0;

	// représente la touche frappée par l'utilisateur : touche de direction ou pour l'arrêt
	char touche;

	//direction courante du serpent (HAUT, BAS, GAUCHE ou DROITE)
	char direction;

	// le plateau de jeu
	tPlateau lePlateau;

	bool collision = false;
	bool gagne = false;
	bool pommeMangee = false;

	// compteur de pommes mangées
	int nbPommes = 0;

	char chemin[20];
   
	// initialisation de la position du serpent : positionnement de la
	// tête en (X_INITIAL, Y_INITIAL), puis des anneaux à sa gauche
	for(int i=0 ; i<TAILLE ; i++){
		lesX[i] = X_INITIAL-i;
		lesY[i] = Y_INITIAL;
	}

	// mise en place du plateau
	initPlateau(lePlateau);
	system("clear");
	dessinerPlateau(lePlateau);


	srand(time(NULL));
	ajouterPomme(lePlateau, nbPommes);

	// initialisation : le serpent se dirige vers la DROITE
	dessinerSerpent(lesX, lesY);
	direction = DROITE;
	touche = DROITE;

	// boucle de jeu. Arret si touche STOP, si collision avec une bordure ou
	// si toutes les pommes sont mangées
	clock_t debut = clock();
	strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
	do {
		// switch (touche){
		// 	case GAUCHE : direction=GAUCHE; break;
		// 	case HAUT 	: direction=HAUT;	break;
		// 	case BAS 	: direction=BAS;	break;
		// 	case DROITE : direction=DROITE; break;
		// }
		if (pommeMangee){
            nbPommes++;
			gagne = (nbPommes==NB_POMMES);
			if (!gagne){
				ajouterPomme(lePlateau, nbPommes);
				pommeMangee = false;
			}
			strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
		}
		if (!gagne){
			nbProgression++;
			if (!collision){
				usleep(ATTENTE);
				if (kbhit()==1){
					touche = getchar();
				}
			}
			progresser(lesX, lesY, direction, lePlateau, &collision, &pommeMangee, nbPommes, chemin);
		}

		
		
	} while (touche != STOP && !collision && !gagne);
    enable_echo();
	gotoxy(1, HAUTEUR_PLATEAU+1);
	
	clock_t fin = clock();
    double nbTick = ((fin - debut)*1.0) / CLOCKS_PER_SEC;
    
    printf("Temps CPU : %.3f secondes\n", nbTick);
	printf("Nombre de progressions effectué : %d\n ", nbProgression);

	return EXIT_SUCCESS;
}


/************************************************/
/*		FONCTIONS ET PROCEDURES DU JEU 			*/
/************************************************/
void initPlateau(tPlateau plateau){
	// initialisation du plateau avec des espaces
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
			plateau[i][j] = VIDE;
		}
	}
	// Mise en place la bordure autour du plateau
	// première ligne
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		plateau[i][1] = BORDURE;
	}
	// lignes intermédiaires
	for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
			plateau[1][j] = BORDURE;
			plateau[LARGEUR_PLATEAU][j] = BORDURE;
		}
	// dernière ligne
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		plateau[i][HAUTEUR_PLATEAU] = BORDURE;
	}
	plateau[1][HAUTEUR_PLATEAU/2] = VIDE; // Trou gauche
	plateau[LARGEUR_PLATEAU/2][1] = VIDE; // Trou haut
	plateau[LARGEUR_PLATEAU/2][HAUTEUR_PLATEAU] = VIDE; // Trou bas
	plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU/2] = VIDE; // Trou droit

    for(int n = 0 ; n<NB_PAVE ; n++) {
        for (int x = 0; x < TAILLE_PAVE; x++) {
            for (int j = 0; j < TAILLE_PAVE; j++) {
                plateau[lesPavesX[n]+j][lesPavesY[n]] = BORDURE;
                if (lesPavesX[n]+j < LARGEUR_PLATEAU && lesPavesY[n] < HAUTEUR_PLATEAU) {
                    plateau[lesPavesX[n]+j][lesPavesY[n]] = BORDURE;
                }
            }
            lesPavesY[n]++;
        }
    }
}

void dessinerPlateau(tPlateau plateau){
	// affiche eà l'écran le contenu du tableau 2D représentant le plateau
	for (int i=1 ; i<=LARGEUR_PLATEAU ; i++){
		for (int j=1 ; j<=HAUTEUR_PLATEAU ; j++){
			afficher(i, j, plateau[i][j]);
		}
	}
}

void ajouterPomme(tPlateau plateau, int nbPommes){
	// génère aléatoirement la position d'une pomme,
	// vérifie que ça correspond à une case vide
	// du plateau puis l'ajoute au plateau et l'affiche
	int xPomme, yPomme;
	xPomme = lesPommesX[nbPommes];
	yPomme = lesPommesY[nbPommes];
	plateau[xPomme][yPomme]=POMME;
	afficher(xPomme, yPomme, POMME);
}

void afficher(int x, int y, char car){
	gotoxy(x, y);
	printf("%c", car);
	gotoxy(1,1);
}

void effacer(int x, int y){
	gotoxy(x, y);
	printf(" ");
	gotoxy(1,1);
}

void dessinerSerpent(int lesX[], int lesY[]){
	// affiche les anneaux puis la tête
	int str[] = {'1','2','3','4','5','6','7','8','9'};
	for(int i=1 ; i<TAILLE ; i++){
		afficher(lesX[i], lesY[i], CORPS);
	}
	afficher(lesX[0], lesY[0],TETE);
}

char* checkMeilleurChemin(int lesX[], int lesY[], int nbPommes){
    static char chemin[30];
    float xDepart = lesX[0], yDepart = lesY[0];
    float xPomme = lesPommesX[nbPommes], yPomme = lesPommesY[nbPommes];
    
    // Distances directes
    float plusCourtNormal = sqrt(pow((xDepart - xPomme), 2) + pow((yDepart - yPomme), 2));
    
    // Coordonnées des portails
    float portailHautX = LARGEUR_PLATEAU / 2, portailHautY = 1;
    float portailGaucheX = 1, portailGaucheY = HAUTEUR_PLATEAU / 2;
    float portailDroitX = LARGEUR_PLATEAU, portailDroitY = HAUTEUR_PLATEAU / 2;
    float portailBasX = LARGEUR_PLATEAU / 2, portailBasY = HAUTEUR_PLATEAU;
    
    // Distances via portails
    float distanceHaut = sqrt(pow((xDepart - portailHautX), 2) + pow((yDepart - portailHautY), 2)) +
                         sqrt(pow((portailBasX - xPomme), 2) + pow((portailBasY - yPomme), 2));
    float distanceGauche = sqrt(pow((xDepart - portailGaucheX), 2) + pow((yDepart - portailGaucheY), 2)) +
                           sqrt(pow((portailDroitX - xPomme), 2) + pow((portailDroitY - yPomme), 2));
    float distanceDroite = sqrt(pow((xDepart - portailDroitX), 2) + pow((yDepart - portailDroitY), 2)) +
                           sqrt(pow((portailGaucheX - xPomme), 2) + pow((portailGaucheY - yPomme), 2));
    float distanceBas = sqrt(pow((xDepart - portailBasX), 2) + pow((yDepart - portailBasY), 2)) +
                        sqrt(pow((portailHautX - xPomme), 2) + pow((portailHautY - yPomme), 2));
    
    // Comparaison des distances
    float plusCourtPortails = fminf(fminf(distanceHaut, distanceGauche), fminf(distanceDroite, distanceBas));
    float meilleureDistance = fminf(plusCourtNormal, plusCourtPortails);
    
    // Identification du chemin
    if (meilleureDistance == plusCourtNormal) {
        strcpy(chemin, "normal");
    } else if (meilleureDistance == distanceHaut) {
        strcpy(chemin, "portailHaut");
    } else if (meilleureDistance == distanceGauche) {
        strcpy(chemin, "portailGauche");
    } else if (meilleureDistance == distanceDroite) {
        strcpy(chemin, "portailDroite");
    } else {
        strcpy(chemin, "portailBas");
    }
    // printf("Distance normale: %.2f\n", plusCourtNormal);
	// printf("Distance portail Haut: %.2f\n", distanceHaut);
	// printf("Distance portail Gauche: %.2f\n", distanceGauche);
	// printf("Distance portail Droite: %.2f\n", distanceDroite);
	// printf("Distance portail Bas: %.2f\n", distanceBas);
	// printf("Plus court via portails: %.2f\n", plusCourtPortails);
	// printf("Meilleure distance: %.2f\n", meilleureDistance);
    return chemin;
}

void choisirDirection(tPlateau plateau,int lesX[], int lesY[], char *direction, int nbPommes, char chemin[20], int destX, int destY){
	bool va_se_toucher = false;
	bool peutAller = true;
	bool feuVertGauche = true;
	bool feuVertDroit = true;
	
	if (lesY[0] < destY) { // direction BAS
		for (int i = 1; i < TAILLE; i++){
			if ((lesY[i] == lesY[0]+i && lesX[i] == lesX[0]) || (plateau[lesX[0]][lesY[0]+1] == BORDURE)){
				va_se_toucher = true;	
			}
			if ((lesX[1]+1 == lesX[0] && lesY[i] == lesY[0]) || (plateau[lesX[0]-1][lesY[0]] == BORDURE) || (lesX[i]+1 == lesX[0] && lesY[i]+1 == lesY[0])){
				feuVertGauche = false;
			}
			if ((lesX[1]-1 == lesX[0] && lesY[i] == lesY[0]) || (plateau[lesX[0]+1][lesY[0]] == BORDURE) || (lesX[i]-1 == lesX[0] && lesY[i]+1 == lesY[0])){
				feuVertDroit = false;
			}
			if (sqrt(pow((lesX[0]-lesX[i]),2) + pow((lesY[0]-lesY[i]),2)) < i && lesY[i] > lesY[0] && lesX[i] == lesX[0]){
				peutAller = false;
			}
		}
		if (!va_se_toucher && peutAller && lesX[0] != 0){
			*direction = BAS;
		}
		else if (!peutAller){
			if ((lesX[0] > destX && feuVertGauche ) || (lesX[0] > destX && !feuVertDroit)){
				*direction = GAUCHE;
			}
			else if (feuVertDroit && (HAUTEUR_PLATEAU-lesY[0] > TAILLE || lesX[0] < destX)){
				*direction = DROITE;
			}
			else{
				*direction = HAUT;
			}
		}
		else if ((lesX[0] > destX && feuVertGauche) || (lesX[0] < destX && !feuVertDroit)){
			*direction = GAUCHE;
		}
		else if (feuVertDroit && (HAUTEUR_PLATEAU-lesY[0] > TAILLE || lesX[0] < destX)){
			*direction = DROITE;
		}
		else{
			*direction = HAUT;
		}
	}    	 
	else if (lesY[0] > destY) { // direction HAUT
		for (int i = 1; i < TAILLE; i++){
			if ((lesY[i] == lesY[0]-1 && lesX[i] == lesX[0]) || (plateau[lesX[0]][lesY[0]-1] == BORDURE)){
				va_se_toucher = true;	
			}
			if ((lesX[1]+1 == lesX[0] && lesY[1] == lesY[0]) || (plateau[lesX[0]-1][lesY[0]] == BORDURE) || (lesX[i]+1 == lesX[0] && lesY[i]-1 == lesY[0])){
				feuVertGauche = false;
			}
			if ((lesX[1]-1 == lesX[0] && lesY[i] == lesY[0]) || (plateau[lesX[0]+1][lesY[0]] == BORDURE) || (lesX[i]-1 == lesX[0] && lesY[i]-1 == lesY[0])){
				feuVertDroit = false;
			}
			if (sqrt(pow((lesX[0]-lesX[i]),2) + pow((lesY[0]-lesY[i]),2)) < i && lesY[i] < lesY[0] && lesX[i] == lesX[0]){
				peutAller = false;
			}
		}
		if (!va_se_toucher && peutAller && lesX[0] != 0){
			*direction = HAUT;
		}
		else if (!peutAller){
			if (lesX[0] > destX && feuVertGauche){
				*direction = GAUCHE;
			}
			else if (feuVertDroit && (lesY[0] > TAILLE || lesX[0] < destX)){
				*direction = DROITE;
			}
			else{
				*direction = BAS;
			}
		}
		else if (lesX[0] > destX && feuVertGauche){
			*direction = GAUCHE;
		}
		else if (feuVertDroit && (lesY[0] > TAILLE || lesX[0] < destX)){
			*direction = DROITE;
		}
		else{
			*direction = BAS;
		}
	}
	else if (lesX[0] < destX) { // direction DROITE 
		for (int i = 1; i < TAILLE; i++){
			if ((lesY[1] == lesY[0] && lesX[1]-1 == lesX[0]) || (plateau[lesX[0]+1][lesY[0]] == BORDURE)){
				va_se_toucher = true;	
			}
			if ((lesX[i]+1 == lesX[0] && lesY[1]-1 == lesY[0]) || (plateau[lesX[0]][lesY[0]-1] == BORDURE) || (lesY[i]+1 == lesY[0] && lesX[i]-1 == lesX[0])){
				feuVertGauche = false;
			}
			if ((lesX[i]+1 == lesX[0] && lesY[1]+1 == lesY[0]) || (plateau[lesX[0]][lesY[0]+1] == BORDURE) || (lesY[i]+1 == lesY[0] && lesX[i]+1 == lesX[0])){
				feuVertDroit = false;
			}
			if (sqrt(pow((lesX[0]-lesX[i]),2) + pow((lesY[0]-lesY[i]),2)) < i && lesY[i] == lesY[0] && lesX[i] > lesX[0]){
				peutAller = false;
			}
		}
		if (!va_se_toucher && peutAller && lesY[0] != 0){
			*direction = DROITE;
		}
		else if (!peutAller){
			if (lesY[0] > destY && feuVertGauche){
				*direction = HAUT;
			}
			else if (feuVertDroit && (LARGEUR_PLATEAU-lesX[0] > TAILLE || lesY[0] < destY)){
				*direction = BAS;
			}
			else{
				*direction = GAUCHE;
			}
		}
		else if (lesY[0] > destY && feuVertGauche){
			*direction = HAUT;
		}
		else if(feuVertDroit && (LARGEUR_PLATEAU-lesX[0] > TAILLE || lesY[0] < destY)){
			*direction = BAS;
		} 
	}
	else if (lesX[0] > destX) { // direction GAUCHE
		for (int i = 1; i < TAILLE; i++){
			if ((lesY[i] == lesY[0] && lesX[i]+1 == lesX[0]) || (plateau[lesX[0]-1][lesY[0]] == BORDURE)){
				va_se_toucher = true;	
			}
			if ((lesX[i]-1 == lesX[0] && lesY[1]-1 == lesY[0]) || (plateau[lesX[0]][lesY[0]-1] == BORDURE) || (lesY[i]-1 == lesY[0] && lesX[i]-1 == lesX[0])){
				feuVertGauche = false;
			}
			if ((lesX[i]-1 == lesX[0] && lesY[1]+1 == lesY[0]) || (plateau[lesX[0]][lesY[0]+1] == BORDURE) || (lesY[i]-1 == lesY[0] && lesX[i]+1 == lesX[0])){
				feuVertDroit = false;
			}
			if (sqrt(pow((lesX[0]-lesX[i]),2) + pow((lesY[0]-lesY[i]),2)) < i && lesY[i] == lesY[0] && lesX[i] < lesX[0]){
				peutAller = false;
			}
		}
		if (!va_se_toucher && peutAller && lesY[0] != 0){
			*direction = GAUCHE;
		}
		else if (!peutAller){
			if (lesY[0] > destY && feuVertGauche){
				*direction = HAUT;
			}
			else if (feuVertDroit && (lesX[0] > TAILLE || lesY[0] > destY)){
				*direction = BAS;
			}
			else{
				*direction = DROITE;
			}
		}
		else if (lesY[0] > destY && feuVertGauche){
			*direction = HAUT;
		}
		else if (feuVertDroit && (lesX[0] > TAILLE || lesY[0] > destY)){
			*direction = BAS;
		}
		else{
			*direction = DROITE;
		}
	}
	// *direction = DROITE;
}

void progresser(int lesX[], int lesY[], char direction, tPlateau plateau, bool * collision, bool * pomme, int nbPommes, char chemin[20]){
	// efface le dernier élément avant d'actualiser la position de tous les 
	// élémentds du serpent avant de le  redessiner et détecte une
	// collision avec une pomme ou avec une bordure
	effacer(lesX[TAILLE-1], lesY[TAILLE-1]);
	// printf("lesX[0] :  %d\n", lesX[0]);
	// printf("lesY[0] :  %d\n", lesY[0]);
	bool va_se_toucher = false;
	bool peutAller = true;
	bool feuVertGauche = true;
	bool feuVertDroit = true;
	bool feuVertHaut = true;
	bool feuVertBas = true;
	
	if (strcmp(chemin,"normal") == 0){
		choisirDirection(plateau,lesX, lesY, &direction, nbPommes, chemin, lesPommesX[nbPommes], lesPommesY[nbPommes]);
	}
	else if (strcmp(chemin,"portailHaut") == 0){
		choisirDirection(plateau,lesX, lesY, &direction, nbPommes, chemin, LARGEUR_PLATEAU / 2, 0);
	}
	else if (strcmp(chemin,"portailGauche") == 0){
		choisirDirection(plateau,lesX, lesY, &direction, nbPommes, chemin, 0, HAUTEUR_PLATEAU / 2);
	}
	else if (strcmp(chemin,"portailDroite") == 0){
		choisirDirection(plateau,lesX, lesY, &direction, nbPommes, chemin, LARGEUR_PLATEAU+1, HAUTEUR_PLATEAU / 2);
	}
	else if (strcmp(chemin,"portailBas") == 0){
		choisirDirection(plateau,lesX, lesY, &direction, nbPommes, chemin, LARGEUR_PLATEAU / 2, HAUTEUR_PLATEAU+1);
	}

	for(int i=TAILLE-1 ; i>0 ; i--){
		lesX[i] = lesX[i-1];
		lesY[i] = lesY[i-1];
	}
	//faire progresser la tete dans la nouvelle direction
	switch(direction){
		case HAUT : 
			lesY[0] = lesY[0] - 1;
			break;
		case BAS:
			lesY[0] = lesY[0] + 1;
			break;
		case DROITE:
			lesX[0] = lesX[0] + 1;
			break;
		case GAUCHE:
			lesX[0] = lesX[0] - 1;
			break;
	}
	*pomme = false;
	// détection d'une "collision" avec une pomme
	if (plateau[lesX[0]][lesY[0]] == POMME){
		*pomme = true;
		// la pomme disparait du plateau
		plateau[lesX[0]][lesY[0]] = VIDE;
	}
	// détection d'une collision avec la bordure
	// else if (plateau[lesX[0]][lesY[0]] == BORDURE){
	// 	*collision = true;
	// }
	if (lesX[0] == LARGEUR_PLATEAU/2 && lesY[0] == 0 && direction == HAUT){
		lesY[0] = HAUTEUR_PLATEAU;  // Téléportation de la tête
		strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
	}
	else if (lesX[0] == LARGEUR_PLATEAU/2 && lesY[0] == HAUTEUR_PLATEAU+1 && direction == BAS){
		lesY[0] = 0;
		strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
		
	}
	else if (lesX[0] == LARGEUR_PLATEAU+1 && lesY[0] == HAUTEUR_PLATEAU/2 && direction == DROITE){
		lesX[0] = 0;
		strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
		
	}
	else if (lesX[0] == 0 && lesY[0] == HAUTEUR_PLATEAU/2 && direction == GAUCHE){
		lesX[0] = LARGEUR_PLATEAU;
		strcpy(chemin, checkMeilleurChemin(lesX, lesY, nbPommes));
		
	}
   	dessinerSerpent(lesX, lesY);
	
}



/************************************************/
/*				 FONCTIONS UTILITAIRES 			*/
/************************************************/
void gotoxy(int x, int y) { 
    printf("\033[%d;%df", y, x);
}

int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

// Fonction pour désactiver l'echo
void disable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Désactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Fonction pour réactiver l'echo
void enable_echo() {
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Réactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}