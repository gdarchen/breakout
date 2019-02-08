#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <SD.h>

#define TFT_DC 9
#define TFT_CS 10

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define JOYST_X A1
#define JOYST_Y A0
#define JOYST_MINX 206
#define JOYST_MAXX 798
#define JOYST_MINY 203
#define JOYST_MAXY 797
#define BP_EVENT (xJoyst==1023)
#define JOYST_XMILIEU (JOYST_MAXX+JOYST_MINX)/2
#define JOYST_PLAGEX (JOYST_MAXX-JOYST_XMILIEU)/2


#define couleurFond ILI9341_BLACK

#define lignes 10
#define colonnes 8
#define hauteurBrique 12
#define BANDE_AFFICHAGE (2 * hauteurBrique)
#define bandeNoire (2*hauteurBrique)
#define largeur (tft.width())
#define hauteur (tft.height()-bandeNoire - BANDE_AFFICHAGE)
#define BALL_RADIUS 6
#define ADD_CONTACT_ZONE 1

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Variables globales
int paddleWidth;
int paddleHeight;
int largeurBrique;

int xBall;
int yBall;
int xPaddle;
int yPaddle;
int xJoyst;
int yJoyst;

float vxBall;
float vyBall;

int briques[lignes][colonnes];

boolean fin;

#define NB_VIE_INIT 3
int vieRestante;

int score;

int choixMenu;
#define MENU 0
#define JOUER 1
#define NIVEAU 2
#define BONUS 3
int choixNiveau;

void setup() {
  Serial.begin(9600);
  
  tft.begin();
  tft.fillScreen(couleurFond);
  initialisationDimension();
  choixMenu = MENU;
  choixNiveau = 2;
}

// Fonction de bouclage
void loop(){
  if (choixMenu==MENU || choixMenu==NIVEAU) {
    choisirMenu();
  }
  else if (choixMenu==JOUER) {
    tft.fillScreen(couleurFond);
    tft.fillRect(0, tft.height() - BANDE_AFFICHAGE, largeur, BANDE_AFFICHAGE, ILI9341_BLUE);
    afficherBriques();
    casseBrique();
  }
  else if (choixMenu==BONUS) {
     bonus(); 
  }
}


void afficherMenu(int choix_temp) {
  // on met le texte à l'endroit
  tft.setRotation(2);
  tft.setCursor(0,0);

  // en-tete de menu  
  uint16_t c = ILI9341_RED;
  uint16_t surbrille = ILI9341_WHITE;
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(5);
  tft.println();
  tft.println("  MENU");
  tft.setTextWrap(false);
  tft.fillRect(0,hauteur/4 + 18,largeur,5,ILI9341_CYAN);
  
  tft.setTextSize(3);
  // choix JOUER
  tft.print("\n    ");
  setSurbrillance(choix_temp,JOUER,c,surbrille);
  tft.println("JOUER");
    
  // choix NIVEAU
  tft.setTextColor(c,couleurFond);
  tft.print("\n  ");
  setSurbrillance(choix_temp,NIVEAU,c,surbrille);
  tft.print("NIVEAU: "); tft.println(choixNiveau);

  // choix BONUS
  tft.setTextColor(c,couleurFond);
  tft.print("\n    ");
  setSurbrillance(choix_temp,BONUS,c,surbrille);
  tft.println("BONUS");

  // on remet l'affichage à l'endroit
  tft.setRotation(0);
}

void setSurbrillance(int choix_temp, int txt_menu, uint16_t c, uint16_t surbrille) {   
  if (choix_temp==txt_menu)
    tft.setTextColor(c,surbrille);
  else
    tft.setTextColor(c,couleurFond);
}

void choisirMenu() {
  tft.fillScreen(couleurFond);
  int choix_temp = 0;
  do {
    afficherMenu(choix_temp);
    xJoyst = analogRead(JOYST_X);
    yJoyst = analogRead(JOYST_Y);    

    if (yJoyst<300 && choix_temp<BONUS)
      choix_temp++;
    else if (yJoyst>700 && choix_temp>JOUER)
      choix_temp--;
      
    if (choix_temp==NIVEAU)
      choisirNiveau();
//    delay(10);
  }
  while (!BP_EVENT);
  choixMenu = choix_temp;
}


void choisirNiveau() {
  if (xJoyst<300 && choixNiveau>1)
    choixNiveau--;
  else if (xJoyst>700 && choixNiveau<3)
    choixNiveau++;
}

// pour afficher des photos du projet !
void bonus() {
  // lire la carte SD et faire defiler des tofs du projet toutes les 3 sec
  do {
/*    PImage logo;
  // initialize the SD card
  SD.begin();

  // load the image into the named instance of PImage
  logo = tft.loadImage("purple.bmp");


  // draw the image on the screen starting at the top left corner
  tft..image(logo, 0, 0);
*/
  }
  while (!BP_EVENT);
  choixMenu = MENU;
}


void initialisationDimension() {
  paddleWidth = tft.width() / 4;
  paddleHeight = tft.height() / 25;
  largeurBrique =  largeur/colonnes;
}

void initialisationPosition() {
  xBall = tft.width() / 2;
  yBall = tft.height() / 3;
  xPaddle = tft.width() / 2;
  yPaddle = tft.height() / 25;

  vxBall = 0;
  vyBall = -2 * choixNiveau;
}

void initVie(int nb) {
  for (int i = 0; i < nb; i++) {
    dessinerVie(10 + i * 20, tft.height() - 10, false);
  }
}

void casseBrique() {
  initVie(NB_VIE_INIT);
  vieRestante = NB_VIE_INIT;
  initialisationPosition();
  score = 0;
  majScore();
//  int difficulte = 50*(3-choixNiveau+1); // mieux vaut changer le vecteur vitesse balle
  fin = false;
  while (!fin) {
    fin = finPartie();
    affichage(true);
    mouvementBalle();
    xJoyst = analogRead(JOYST_X);
    mouvementPaddle();
    affichage(false);
    delay(20);
  }
  if (fin){
    afficherFinPartie();
    delay(5000);
  }
  choixMenu = MENU;
}


void mouvementPaddle() {
  // en dessous d'un certain seuil, on considère que le joystick ne bouge pas
  if (abs(xJoyst-JOYST_XMILIEU)>JOYST_PLAGEX/5) {
    // vitesse du paddle proportionnelle à l'amplitude du mvmnt du joystick  
    xPaddle = xPaddle-5*(xJoyst-JOYST_XMILIEU)/(JOYST_PLAGEX);
    if (xPaddle-paddleWidth/2 < 0) {  
        xPaddle = paddleWidth/2;
      }
    else if (xPaddle+paddleWidth/2 > largeur) {
        xPaddle = largeur-paddleWidth/2;  
    } 
  }
}


void mouvementBalle() {
  yBall = yBall+vyBall;
  xBall = xBall+vxBall;

  // si la balle arrive sur le paddle
  if (yBall-BALL_RADIUS <= yPaddle+paddleHeight
      && xBall-ADD_CONTACT_ZONE <= xPaddle+paddleWidth/2
      && xBall+ADD_CONTACT_ZONE >= xPaddle-paddleWidth/2) {
    paddleCollision();
    yBall = yPaddle+paddleHeight+BALL_RADIUS;
  }
  
  // si la balle touche le haut de l'ecran
  else if (yBall+BALL_RADIUS >= tft.height()-BANDE_AFFICHAGE) {
    verticalCollision();
    yBall = tft.height() - BALL_RADIUS - BANDE_AFFICHAGE-1;
  }
  
  // si la balle touche le bas de l'ecran
  else if (yBall - BALL_RADIUS <= 0) {  // si la balle touche un cote de l'ecran

    vieRestante--;
    dessinerVie(10 + vieRestante * 20, tft.height() - 10, true);
    if (vieRestante == 0 || finPartie()) {
      fin = true;
      afficherFinPartie();
      delay(5000);
    }
    else {
      initialisationPosition();
    }
  }
  
  // si la balle touche un cote de l'ecran
  if (xBall - BALL_RADIUS <= 0){
    horizontalCollision();
    xBall = BALL_RADIUS;
  }
  else if (xBall + BALL_RADIUS >= tft.width()) {
    horizontalCollision();
    xBall = tft.width() - BALL_RADIUS;
  }
  
  // si la balle arrive dans un angle en bas (entre le paddle et les bords de l'ecran)
  if (xBall - BALL_RADIUS <= 0 && (yBall-BALL_RADIUS <= yPaddle+paddleHeight
                                    && xBall-ADD_CONTACT_ZONE <= xPaddle+paddleWidth/2
                                    && xBall+ADD_CONTACT_ZONE >= xPaddle-paddleWidth/2)){
    coinsEnBasCollision();
    yBall = yPaddle+paddleHeight+BALL_RADIUS;
  }
  else if (xBall + BALL_RADIUS >= tft.width() && (yBall-BALL_RADIUS <= yPaddle+paddleHeight
                                    && xBall-ADD_CONTACT_ZONE <= xPaddle+paddleWidth/2
                                    && xBall+ADD_CONTACT_ZONE >= xPaddle-paddleWidth/2)){
    coinsEnBasCollision();
  }
  
  // collision briques
  int l, l1, l2, c, c1, c2;
  int deltaYRadius, deltaXRadius;
  if (vyBall > 0 ) {
    deltaYRadius = BALL_RADIUS;
  }
  else {
    deltaYRadius = -BALL_RADIUS;
  }
  if (vxBall > 0 ) {
    deltaXRadius = BALL_RADIUS;
  }
  else {
    deltaXRadius = -BALL_RADIUS;  

  }
  if (yBall + deltaYRadius > hauteur - lignes * hauteurBrique && yBall + deltaYRadius < hauteur) {
    // collision verticale
    l = (hauteur - (yBall + deltaYRadius) ) / hauteurBrique;
    c1 = (xBall - ADD_CONTACT_ZONE) / largeurBrique;
    c2 = (xBall + ADD_CONTACT_ZONE) / largeurBrique;
    if (briques[l][c1] == 1 || briques[l][c2] == 1) {
       if (c1 != c2 && briques[l][c1] == 1 && briques[l][c2] == 1) {
          score += 20 * choixNiveau;
       }
       else {
          score += 10 * choixNiveau;
       }
       briques[l][c1] = 0;
       briques[l][c2] = 0;
       if (vyBall > 0 ) {
         yBall = hauteur - (l+1) * hauteurBrique - BALL_RADIUS;
       }
       else {
         yBall = hauteur - l * hauteurBrique + BALL_RADIUS;
       }
       verticalCollision();
       tft.fillRect(c1 * largeurBrique, hauteur - (l+1) * hauteurBrique, largeurBrique, hauteurBrique, couleurFond);
       tft.fillRect(c2 * largeurBrique, hauteur - (l+1) * hauteurBrique, largeurBrique, hauteurBrique, couleurFond);
       majScore();
    }
    
    // collision horizontale
    l1 = (hauteur - yBall - ADD_CONTACT_ZONE) / hauteurBrique;
    l2 = (hauteur - yBall + ADD_CONTACT_ZONE) / hauteurBrique;
    c = (xBall + deltaXRadius) / largeurBrique;
    if (briques[l1][c] == 1 || briques[l2][c] == 1) {
       if (l1 != l2 && briques[l1][c] == 1 && briques[l2][c] == 1) {
          score += 20 * choixNiveau;
       }
       else {
          score += 10 * choixNiveau;
       }
       briques[l1][c] = 0;
       briques[l2][c] = 0;
       if (vxBall > 0 ) {
         xBall = c * largeurBrique - BALL_RADIUS;
       }
       else {
         xBall = (c+1) * largeurBrique + BALL_RADIUS;
       }
       horizontalCollision();
       tft.fillRect(c * largeurBrique, hauteur - (l1+1) * hauteurBrique, largeurBrique, hauteurBrique, couleurFond);
       tft.fillRect(c * largeurBrique, hauteur - (l2+1) * hauteurBrique, largeurBrique, hauteurBrique, couleurFond);
       majScore();
    }
  }
}

void coinsEnBasCollision(){
  vyBall = -vyBall*1.01;
  vxBall = -vxBall*1.01;
}

void paddleCollision() {
  vyBall = -vyBall;
  double oldAngle = atan(vxBall/vyBall);
  double radius = vyBall/cos(oldAngle);
  double newAngle = oldAngle+((xBall-xPaddle)/(paddleWidth/2.0))*(M_PI/2-M_PI/6);
  vxBall = round(sin(newAngle)*radius)*1.01;
  vyBall = round(cos(newAngle)*radius)*1.01;
}

void verticalCollision() {
  vyBall = -vyBall*1.01;
}

void horizontalCollision() {
  vxBall = -vxBall*1.01;
}

void majScore() {
  tft.fillRect(tft.width() - 2 * tft.width() / 3, tft.height()- BANDE_AFFICHAGE, 2 * tft.width() / 3, BANDE_AFFICHAGE, ILI9341_BLUE);
  
  // on met le texte à l'endroit
  tft.setRotation(2);
  tft.setCursor(0,0);

  // affichage score
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Score ");
  tft.println(score);

  // on remet l'affichage à l'endroit
  tft.setRotation(0);
}

void dessinerVie(int x, int y, boolean clean) {
  uint16_t couleur;
  if (clean) {
    couleur = ILI9341_BLUE;
  }
  else {
    couleur = ILI9341_RED;
  }
  tft.fillCircle(x+6, y, 4, couleur);
  tft.fillCircle(x-2, y, 4, couleur);
  tft.fillTriangle(x-6, y, x+10, y, x+2, y-10, couleur);
}

boolean finPartie(){
  boolean res = true;
  int i,j;
  for (int i=0; i<lignes; i++){
    for (int j=0; j<colonnes; j++){
      if (briques[i][j]==1){
        res = 0;
      }
    }
  }
  return res;
}

void afficherFinPartie(){
  tft.fillRect(0,0,tft.width(),tft.height(),ILI9341_YELLOW);
  // on met le texte à l'endroit
  tft.setRotation(2);
  tft.setCursor(0,0);
  
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(5);
  tft.println();
  tft.println(" Score :\n\n");
  tft.print("   ");
  tft.println(score);
  tft.setTextWrap(false);
}


void affichage(boolean clean) {
  tft.fillCircle(xBall, yBall, BALL_RADIUS, (clean ? couleurFond : ILI9341_RED));
  tft.fillRect(xPaddle-paddleWidth/2, yPaddle, paddleWidth, paddleHeight, (clean ? couleurFond : ILI9341_WHITE));
}

// Fonction qui cree un tableau de briques de lignes*colonnes cases
void creerBriques(){
  for (int i=0 ; i<lignes ; i++){
    for (int j=0 ; j<colonnes ; j++){
      briques[i][j]=1;
    }
  }
}

// Fonction qui affiche les briques à l'écran
void afficherBriques(){
  uint16_t couleur;
  creerBriques();
  //briques[0][0]=0;// faire une case vide
  for (int i=0; i<lignes; i++){
    for (int j=0; j<colonnes; j++){
      couleur = couleurCase();
        tft.drawRect(j*largeurBrique,hauteur-(i+1)*hauteurBrique,largeurBrique,hauteurBrique,couleurFond);
        tft.fillRect(j*largeurBrique+1,hauteur-(i+1)*hauteurBrique+1,largeurBrique-1,hauteurBrique-1,couleur);
      }
    }
    
  }


// Fonction qui retourne au hasard une des couleurs prédéfinies de l'écran
uint16_t couleurCase(){
  int i=random(0,17);
  uint16_t couleurs[18];
  couleurs[0]=ILI9341_NAVY;
  couleurs[1]=ILI9341_DARKGREEN;
  couleurs[2]=ILI9341_DARKCYAN;
  couleurs[3]=ILI9341_MAROON;
  couleurs[4]=ILI9341_PURPLE;
  couleurs[5]=ILI9341_OLIVE;
  couleurs[6]=ILI9341_LIGHTGREY;
  couleurs[7]=ILI9341_DARKGREY;
  couleurs[8]=ILI9341_BLUE;
  couleurs[9]=ILI9341_GREEN;
  couleurs[10]=ILI9341_CYAN;
  couleurs[11]=ILI9341_RED;
  couleurs[12]=ILI9341_MAGENTA;
  couleurs[13]=ILI9341_YELLOW;
  couleurs[14]=ILI9341_WHITE;
  couleurs[15]=ILI9341_ORANGE;
  couleurs[16]=ILI9341_GREENYELLOW;
  couleurs[17]=ILI9341_PINK;
  return couleurs[i];
}

